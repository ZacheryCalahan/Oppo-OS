#include "headers/stdlib.h"
#include "headers/stdio.h"

/*
    Buddy allocator!
*/

typedef struct Page {
    struct Page *next;
} Page;

#define MAX_ORDER 10

Page *free_lists[MAX_ORDER + 1];

static uint64_t heap_start = (uint64_t) __free_ram;
static uint64_t heap_end = (uint64_t) __free_ram_end;

int order_for_pages(size_t pages) {
    int order = 0;
    size_t size = 1;
    while (size < pages && order <= MAX_ORDER) {
        size <<= 1;
        order++;
    }
    return order;
}

static uintptr_t buddy_addr(uintptr_t addr, int order) {
    size_t block_size = PAGE_SIZE << order;
    return addr ^ block_size;
}

void init_memory() {
    size_t total_pages = (heap_end - heap_start) / PAGE_SIZE;
    uintptr_t addr = heap_start;

    // Greedy initialization
    for (int order = MAX_ORDER; order >= 0; order--) {
        size_t block_pages = 1 << order;
        size_t block_size = block_pages * PAGE_SIZE;

        while (total_pages >= block_pages) {
            Page *block = (Page *) addr;
            block->next = free_lists[order];
            free_lists[order] = block;

            addr += block_size;
            total_pages -= block_pages;
        }
    }
}

void* kalloc_order(size_t order) {
    if (order > MAX_ORDER) return NULL;

    // Find smallest available block >= to order
    int curr_order = order;
    while (curr_order <= MAX_ORDER && !free_lists[curr_order]) {
        curr_order++;
    }

    if (curr_order > MAX_ORDER) return NULL; // Out of memory!

    // Split down to desired order
    while (curr_order > order) {
        Page *block = free_lists[curr_order];
        free_lists[curr_order] = block->next;

        // Split into two buddies
        uintptr_t buddy1 = (uintptr_t) block;
        uintptr_t buddy2 = buddy1 + (PAGE_SIZE << (curr_order - 1));

        Page *b1 = (Page *) buddy1;
        Page *b2 = (Page *) buddy2;

        b1-> next = b2;
        b2-> next = free_lists[curr_order - 1];
        free_lists[curr_order - 1] = b1;

        curr_order--;
    }

    // Allocate from desired order
    Page* result = free_lists[order];
    free_lists[order] = result->next;
    return (void *) result;
}

void* kalloc(size_t n) {
    int order = order_for_pages(n);
    void* page = kalloc_order(order);

    // Clean page of data
    memset(page, 0, PAGE_SIZE * n);
    return page;
}

void kfree_order(void *ptr, size_t order) {
    if (order > MAX_ORDER) return;

    uintptr_t addr = (uintptr_t) ptr;

    while (order < MAX_ORDER) {
        uintptr_t buddy = buddy_addr(addr, order);
        Page *prev = NULL;
        Page *curr = free_lists[order];

        // Search free list for buddy
        while (curr) {
            if ((uintptr_t) curr == buddy) {
                // Remove buddy from list
                if (prev) {
                    prev->next = curr->next;
                } else {
                    free_lists[order] = curr->next;
                }

                // Merge blocks
                addr = (addr < buddy) ? addr : buddy;
                order++;
                goto try_merge_again;
            }
            prev = curr;
            curr = curr->next;
        }
        break;
        try_merge_again:;
    }

    // Insert merged block
    Page *block = (Page *) addr;
    block->next = free_lists[order];
    free_lists[order] = block;
}

void* memset(void *dest, int value, size_t n) {
    unsigned char *ptr = dest;
    unsigned char byte = (unsigned char) value;

    while (n--) {
        *ptr++ = byte;
    }

    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    char* d = (char*) dest;
    const char* s = (const char*) src;

    while (len--) {
        *d++ = *s++;
    }

    return dest;
}
