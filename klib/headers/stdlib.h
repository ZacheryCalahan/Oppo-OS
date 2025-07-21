#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096 // This should never change, it's the intended page size for sv39!
extern char __free_ram[], __free_ram_end[];

/**
 * Initialize the kernel memory management system.
 *
 * init_memory() sets up the memory management system by initializing
 * the free list with pages from the specified heap range. Each page
 * is added to the free list, making it available for allocation.
 *
 * @param heap_start The starting address of the heap.
 * @param heap_end The ending address of the heap.
 */
void init_memory();

/**
 * Allocate a page of memory.
 *
 * kalloc() allocates a single page of memory. If no more pages are available,
 * it returns NULL.
 *
 * @return A pointer to the allocated page of memory, or NULL if no more
 *         pages are available.
 */
void* kalloc(size_t n);

/**
 * Free a previously allocated page of memory.
 *
 * kfree() returns a page of memory back to the free list, making it available
 * for future allocations. The pointer passed must be a valid page previously
 * allocated by kalloc().
 *
 * @param ptr A pointer to the page of memory to be freed.
 */
void kfree_order(void *ptr, size_t order);

/**
 * Fill a block of memory with a given value.
 *
 * memset() fills the first n bytes of the memory area pointed to by dest
 * with the constant byte value. The memory area must be at least n bytes
 * large.
 *
 * @param dest The memory area to fill.
 * @param value The byte value to use for filling.
 * @param n The number of bytes to fill.
 * @return dest, for convenience.
 */
void* memset(void *dest, int value, size_t n);

/**
 * Copy a block of memory from one location to another.
 *
 * memcpy() copies the first n bytes of the memory area pointed to by src
 * to the memory area pointed to by dest. The memory areas must not overlap.
 *
 * @param dest The memory area to fill.
 * @param src The memory area to read from.
 * @param len The number of bytes to copy.
 * @return dest, for convenience.
 */
void* memcpy(void* dest, const void* src, size_t len);

static inline int is_aligned(uint64_t value, uint64_t align) {
    if (align == 0)
        align = sizeof(void*);
    return value % align == 0;
}

static inline uint64_t align_up(uint64_t value, uint64_t align) {
    return (value + align - 1) & ~(align - 1);
}
int order_for_pages(size_t pages);


#endif