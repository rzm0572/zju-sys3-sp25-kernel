#include "private_kdefs.h"
#include <mm.h>
#include <stdint.h>
#include <printk.h>
#include <string.h>

#ifdef ONBOARD

extern uint8_t _ekernel[];

static struct kfreelist {
  struct kfreelist *next;
} *kfreelist;

static uint32_t* ref_cnt;

void *alloc_page(void) {
  struct kfreelist *r = kfreelist;
  kfreelist = r->next;
  return r;
}

// Warning: this function does not garantee the allocated pages are continuous.
void *alloc_pages(size_t nrpages) {
  struct kfreelist *r = kfreelist;
  for (size_t i = 0; i < nrpages; i++) {
    kfreelist = kfreelist->next;
  }
  return r;
}

void free_pages(void *addr) {
  struct kfreelist *r = (void *)PGROUNDDOWN((uintptr_t)addr);
  r->next = kfreelist;
  kfreelist = r;
}

void mm_init(void) {
  kfreelist = INVALID_PA;
  uint8_t *s = (void *)PGROUNDUP((uintptr_t)_ekernel);
  const uint8_t *e = (void *)(PHY_END + PA2VA_OFFSET);
  for (uint8_t *p = (uint8_t*)e - PGSIZE; p >= s; p -= PGSIZE) {
    free_pages(p);
  }

  // Init ref_cnt
  uint64_t ref_cnt_size = ((uint64_t)PHY_SIZE >> PAGE_SHIFT) * sizeof(uint32_t);
  ref_cnt = (uint32_t*)alloc_pages(ref_cnt_size >> PAGE_SHIFT);
  memset(ref_cnt, 0, ref_cnt_size);
}

int ref_page(void *va) {
  uint64_t pfn = PHYS2PFN(VA2PA(va));
  if (ref_cnt[pfn] == 0) {
    return -1;
  }
  ref_cnt[pfn]++;
  return 0;
}

int deref_page(void *va) {
  uint64_t pfn = PHYS2PFN(VA2PA(va));
  if (ref_cnt[pfn] == 0) {
    return -1;
  }
  ref_cnt[pfn]--;
  return 0;
}

uint64_t get_ref_cnt(void *pa) {
  uint64_t pfn = PHYS2PFN(pa);
  return ref_cnt[pfn];
}

#else

#include <mm.h>
#include <vm.h>
#include <string.h>
#include <printk.h>
#include <sbi.h>
#include <proc.h>

extern uint8_t _ekernel[];
extern struct task_struct* current;

#define unlikely(x) __builtin_expect(!!(x), 0)

// fine, write buddy system here
#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) (((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x) & ((x) - 1)))

#define __MAX(a, b)     \
  ({                    \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a > _b ? _a : _b;  \
  })

static void *free_page_start = &_ekernel;
static struct {
  uint64_t *bitmap;      // length of longest continuous free pages
  uint64_t *ref_cnt;
  uint64_t size;
} buddy;

static uint64_t fixsize(uint64_t size) {
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size |= size >> 32;
  return size + 1;
}

static void buddy_free(uint64_t pfn) {
  // if ref_cnt is not zero, do nothing
  if (buddy.ref_cnt[pfn]) {
    return;
  }

  uint64_t node_size, index = 0;
  uint64_t left_longest, right_longest;

  node_size = 1;
  index = pfn + buddy.size - 1;

  for (; buddy.bitmap[index]; index = PARENT(index)) {
    node_size *= 2;
    if (index == 0)
      break;
  }

  buddy.bitmap[index] = node_size;

  while (index) {
    index = PARENT(index);
    node_size *= 2;

    left_longest = buddy.bitmap[LEFT_LEAF(index)];
    right_longest = buddy.bitmap[RIGHT_LEAF(index)];

    if (left_longest + right_longest == node_size)
      buddy.bitmap[index] = node_size;
    else
      buddy.bitmap[index] = __MAX(left_longest, right_longest);
  }
}

static void buddy_pfn_incref(uint64_t pfn) {
  buddy.ref_cnt[pfn]++;
}

static void buddy_pfn_decref(uint64_t pfn) {
  if (buddy.ref_cnt[pfn] && !--buddy.ref_cnt[pfn]) {
    buddy_free(pfn);
  }
}

static uint64_t buddy_alloc(size_t nrpages) {
  uint64_t index = 0;

  if (nrpages == 0) {
    nrpages = 1;
  } else if (!IS_POWER_OF_2(nrpages)) {
    nrpages = fixsize(nrpages);
  }

  if (unlikely(buddy.bitmap[index] < nrpages)) {
    printk("\x1b[1;37;41mERROR\x1b[31;49m kernel out of memory\n"
           "Try increasing PHY_SIZE (currently %#x) in private_kdefs.h\x1b[0m\n",
           PHY_SIZE);
    sbi_ecall(0x53525354, 0, 0, 1, 0, 0, 0, 0);
    __builtin_unreachable();
  }

  uint64_t node_size;
  for (node_size = buddy.size; node_size != nrpages; node_size /= 2) {
    if (buddy.bitmap[LEFT_LEAF(index)] >= nrpages) {
      index = LEFT_LEAF(index);
    } else {
      index = RIGHT_LEAF(index);
    }
  }

  buddy.bitmap[index] = 0;
  uint64_t pfn = (index + 1) * node_size - buddy.size;
  // set ref_cnt to 1
  buddy.ref_cnt[pfn] = 1;

  while (index) {
    index = PARENT(index);
    buddy.bitmap[index] = __MAX(buddy.bitmap[LEFT_LEAF(index)], buddy.bitmap[RIGHT_LEAF(index)]);
  }

  return pfn;
}

static void buddy_init(void) {
  uint64_t buddy_size = (uint64_t)PHY_SIZE / PGSIZE;

  if (!IS_POWER_OF_2(buddy_size))
    buddy_size = fixsize(buddy_size);

  buddy.size = buddy_size;
  buddy.bitmap = free_page_start;
  free_page_start = (uint8_t *)free_page_start + 2 * buddy.size * sizeof(*buddy.bitmap);
#ifndef ONBOARD
  memset(buddy.bitmap, 0, 2 * buddy.size * sizeof(*buddy.bitmap));
#endif
  // alloc space for ref_cnt
  buddy.ref_cnt = free_page_start;
  free_page_start = (uint8_t *)free_page_start + buddy.size * sizeof(*buddy.ref_cnt);
#ifndef ONBOARD
  memset(buddy.ref_cnt, 0, buddy.size * sizeof(*buddy.ref_cnt));
#endif

  uint64_t node_size = 2 * buddy.size;
  for (uint64_t i = 0; i < 2 * buddy.size - 1; ++i) {
    if (IS_POWER_OF_2(i + 1)) {
      node_size /= 2;
    }
    buddy.bitmap[i] = node_size;
  }

  for (uint64_t pfn = PHYS2PFN(VA2PA(free_page_start)); pfn; --pfn) {
    buddy_alloc(1);
  }

  printk("...buddy_init done! size = %lu\n", buddy.size);
  return;
}

void *alloc_pages(size_t nrpages) {
  uint64_t pfn = buddy_alloc(nrpages);
  return (void *)PA2VA(PFN2PHYS(pfn));
}

void *alloc_page(void) {
  return alloc_pages(1);
}

void free_pages(void *va) {
  buddy_free(PHYS2PFN(VA2PA(va)));
}

int ref_page(void *va) {
  uint64_t pfn = PHYS2PFN(VA2PA(va));
  // check if the page is allocated
  if (buddy.ref_cnt[pfn] == 0) {
    return -1;
  }
  buddy_pfn_incref(pfn);
  return 0;
}

int deref_page(void *va) {
  uint64_t pfn = PHYS2PFN(VA2PA(va));
  // check if the page is allocated
  if (buddy.ref_cnt[pfn] == 0) {
    return -1;
  }
  buddy_pfn_decref(pfn);
  return 0;
}

uint64_t get_ref_cnt(void *pa) {
  uint64_t pfn = PHYS2PFN(pa);
  return buddy.ref_cnt[pfn];
}

void mm_init(void) __attribute__((alias("buddy_init")));

#endif
