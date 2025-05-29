#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <mman.h>
#include <stddef.h>
#include <stdint.h>

#define PGSIZE 0x1000
#define PGROUNDDOWN(addr) ((addr) & ~(PGSIZE - 1))
#define PGROUNDUP(addr) PGROUNDDOWN((addr) + PGSIZE - 1)

typedef struct meta slab;

typedef struct malloc_block {
    union {
        slab* meta;
        struct malloc_block* next;
    };
    uint8_t data[];
} mblock;

typedef struct meta {
    void* va;
    size_t unit_size;
    int unit_count;
    mblock* bfreelist;
    slab* prev;
    slab* next;
} slab;

void heap_init();

mblock* alloc_block(size_t size);
void free_block(mblock* block);

slab* get_avail_slab(int idx);
slab* alloc_slab(int idx);
void free_slab(slab* slab_);

void* malloc(size_t size);
void free(void* ptr);

#endif /* _MALLOC_H_ */
