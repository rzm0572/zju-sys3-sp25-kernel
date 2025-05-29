#include <malloc.h>
#include <mman.h>
#include <unistd.h>

#define MAX_COUNT_SLAB(data_size) ((PGSIZE - sizeof(slab)) / (data_size + sizeof(mblock)))
#define UNIT_SIZE_SLAB(idx) ((idx) < 8 ? (1 << (idx)) * PGSIZE : (1 << ((idx) - 12)))

size_t slab_max_count[] = {
    1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1,
    MAX_COUNT_SLAB(1),
    MAX_COUNT_SLAB(2),
    MAX_COUNT_SLAB(4),
    MAX_COUNT_SLAB(8),
    MAX_COUNT_SLAB(16),
    MAX_COUNT_SLAB(32),
    MAX_COUNT_SLAB(64),
    MAX_COUNT_SLAB(128),
    MAX_COUNT_SLAB(256),
    MAX_COUNT_SLAB(512),
    MAX_COUNT_SLAB(1024),
    MAX_COUNT_SLAB(2048),
};

size_t slab_num_page[] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1
};

static struct heap_content {
    slab* used_slab[24];
    slab* avail_slab[24];
    void* ufree_start;
    void* heap_start;
    size_t heap_size;
} heap;

extern uint8_t _edata_user[];

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

static int get_used_slab_idx(size_t size) {
    int idx = 0;
    if (size <= PGSIZE / 2) {
        size = fixsize(size);
        idx = 12;
        while (size) {
            size >>= 1;
            idx++;
        }
    } else {
        printf("%d\n", size + sizeof(slab) + sizeof(mblock));
        size = fixsize(size + sizeof(slab) + sizeof(mblock));
        size /= PGSIZE;
        while (size) {
            size >>= 1;
            idx++;
        }
    }
    return idx - 1;
}

static int get_block_idx(size_t size) {
    size = fixsize(size);
    int idx = 0;
    if (size < PGSIZE) {
        idx = 12;
        while (size) {
            size >>= 1;
            idx++;
        }
    } else {
        size /= PGSIZE;
        while (size) {
            size >>= 1;
            idx++;
        }
    }
    return idx - 1;
}

void heap_init() {
    for (int i = 0; i < 20; i++) {
        heap.used_slab[i] = NULL;
        heap.avail_slab[i] = NULL;
    }
    heap.heap_start = (void*)PGROUNDUP((size_t)_edata_user);
    heap.heap_size = (size_t)PGSIZE;
    void* heap_start = mmap(heap.heap_start, heap.heap_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (heap_start == MAP_FAILED) {
        printf("heap init failed\n");
        while (1);
    }
    heap.heap_start = heap_start;
    heap.ufree_start = heap.heap_start;
    printf("...heap_init done!\n");
}

int heap_inflate() {
    void* heap_start = mmap(heap.heap_start + heap.heap_size, heap.heap_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (heap_start == MAP_FAILED) {
        printf("heap inflate failed\n");
        return -1;
    }
    heap.heap_size *= 2;
    return 0;
}

mblock* alloc_block(size_t size) {
    int idx = get_used_slab_idx(size);
    if (idx < 0) {
        printf("alloc_block failed: idx = %d\n", idx);
        return NULL;
    }

    slab* slab_ = get_avail_slab(idx);
    mblock* block = (mblock*)slab_->bfreelist;
    
    if (block == NULL) {
        printf("alloc_block failed: block == NULL\n");
        return NULL;
    }

    slab_->bfreelist = slab_->bfreelist->next;
    block->meta = slab_;
    
    if (slab_->unit_count && !--slab_->unit_count) {
        if (slab_ == heap.avail_slab[idx]) heap.avail_slab[idx] = slab_->next;
        if (slab_->prev != NULL) slab_->prev->next = slab_->next;
        if (slab_->next != NULL) slab_->next->prev = slab_->prev;
        slab_->prev = NULL;
        slab_->next = heap.used_slab[idx];
        heap.used_slab[idx]->prev = slab_;
        heap.used_slab[idx] = slab_;
        // printf("slab full, idx = %d, slab = %p\n", idx, slab_);
    }

    // printf("alloc_block: idx = %d, block = %p, slab = %p, unit_size = %d, unit_count = %d\n", idx, block, slab_, slab_->unit_size, slab_->unit_count);
    return block;
}

void free_block(mblock* block) {
    slab* slab_ = block->meta;
    block->next = slab_->bfreelist;
    slab_->bfreelist = block;
    
    int idx = get_block_idx(slab_->unit_size);
    if (!slab_->unit_count && ++slab_->unit_count) {
        if (slab_ == heap.used_slab[idx]) heap.used_slab[idx] = slab_->next;
        if (slab_->prev != NULL) slab_->prev->next = slab_->next;
        if (slab_->next != NULL) slab_->next->prev = slab_->prev;
        slab_->prev = NULL;
        slab_->next = heap.avail_slab[idx];
        heap.avail_slab[idx]->prev = slab_;
        heap.avail_slab[idx] = slab_;
        // printf("slab avail, idx = %d, slab = %p\n", idx, slab_);
    }
}

slab* get_avail_slab(int idx) {
    if (heap.avail_slab[idx] == NULL) {
        heap.avail_slab[idx] = (slab*)alloc_slab(idx);
    }
    // printf("get_avail_slab: idx = %d, avail_slab = %p\n", idx, heap.avail_slab[idx]);
    return heap.avail_slab[idx];
}

slab* alloc_slab(int idx) {
    void* new_heap_end = heap.ufree_start + slab_num_page[idx] * PGSIZE;
    while (new_heap_end > heap.heap_start + heap.heap_size) {
        heap_inflate();
    }

    slab* slab_ = (slab*)heap.ufree_start;
    slab_->va = heap.ufree_start;
    heap.ufree_start = (void*)heap.ufree_start + slab_num_page[idx] * PGSIZE;
    slab_->unit_size = UNIT_SIZE_SLAB(idx);
    slab_->unit_count = slab_max_count[idx];

    slab_->prev = NULL;
    slab_->next = heap.avail_slab[idx];
    heap.avail_slab[idx]->prev = slab_;
    heap.avail_slab[idx] = slab_;

    slab_->bfreelist = (mblock*)(slab_->va + sizeof(slab));
    mblock* block = slab_->bfreelist;
    for (size_t i = 0; i < slab_max_count[idx] - 1; i++) {
        block->next = (mblock*)((char*)block + slab_->unit_size + sizeof(mblock));
        block = block->next;
    }
    block->next = NULL;
    
    return slab_;
}

void free_slab(slab* slab_) {
    if (slab_ == NULL) {
        return;
    }
    // TODO: free slab
}

// Only support size <= 2^23 - sizeof(slab) - sizeof(mblock) ≈ 8 MiB
void* malloc(size_t size) {
    mblock* block = alloc_block(size);
    return (void*)block + sizeof(mblock);
}

// Only support ptr = malloc(size)
void free(void* ptr) {
    mblock* block = (mblock*)(ptr - sizeof(mblock));
    free_block(block);
}
