#include "headers/stdlib.h"
#include "headers/stdio.h"

typedef struct Page {
    struct Page *next;
} Page;

Page *free_list = NULL;

static uint64_t heap_top = (uint64_t) __free_ram;
static uint64_t KERNEL_HEAP_END = (uint64_t) __free_ram_end;


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

void init_memory() {
    for (uint64_t addr = (uint64_t) __free_ram; addr < (uint64_t) __free_ram_end; addr += PAGE_SIZE) {
        Page *page = (Page*)addr;
        page -> next = free_list;
        free_list = page;
    }

    printf("Start/End of kernel RAM:\n\t0x%x\n\t0x%x\n", heap_top, KERNEL_HEAP_END);
}

/**
 * Allocate a page of memory.
 *
 * kalloc() allocates a single page of memory. If no more pages are available,
 * it returns NULL.
 *
 * @return A pointer to the allocated page of memory, or NULL if no more
 *         pages are available.
 */
void* kalloc() {
    if (free_list == NULL) {
        return NULL; // Out of memory
    }
    Page *page = free_list;
    free_list = free_list -> next;

    // Clean page of data
    memset(page, 0, PAGE_SIZE);
    return (void*) page;
}

/**
 * Free a previously allocated page of memory.
 *
 * kfree() returns a page of memory back to the free list, making it available
 * for future allocations. The pointer passed must be a valid page previously
 * allocated by kalloc().
 *
 * @param ptr A pointer to the page of memory to be freed.
 */
void kfree(void *ptr) {
    Page *page = (Page*)ptr;
    page -> next = free_list;
    free_list = page;
}

/**
 * Allocate a block of memory.
 *
 * kmalloc() allocates a block of memory of the given size, aligned to 8 bytes.
 * If the allocation exceeds the available heap memory, it returns NULL.
 *
 * @param size The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if allocation fails.
 */

void *kmalloc(size_t size) {
    void *addr = (void *) heap_top;
    heap_top += (size + 7) & ~7; // Align to 8 bytes
    if (heap_top > KERNEL_HEAP_END) {
        return NULL; // Out of memory
    }

    return addr;
}

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
void* memset(void *dest, int value, size_t n) {
    unsigned char *ptr = dest;
    unsigned char byte = (unsigned char) value;

    while (n--) {
        *ptr++ = byte;
    }

    return dest;
}

/**
 * Allocates a block of memory with a specified alignment.
 *
 * kaligned_alloc() allocates a block of memory of the given size with the 
 * specified alignment, which must be a power of 2 and at least the size of a 
 * pointer. It utilizes kmalloc to allocate the necessary memory, ensuring that 
 * the returned address is correctly aligned. If allocation fails, it returns 
 * NULL.
 *
 * @param alignment The alignment for the memory block. Must be a power of 2.
 * @param size The size of the memory block to allocate, in bytes.
 * @return A pointer to the aligned memory block, or NULL if allocation fails.
 */
void* kaligned_alloc(size_t alignment, size_t size) {
    // Ensure alignment is a power of 2 and at least the size of a pointer
    if ((alignment & (alignment - 1)) != 0 || alignment < sizeof(void*)) {
        return NULL;
    }

    // Allocate extra memory for alignment adjustment and metadata storage
    size_t total_size = size + alignment - 1 + sizeof(void*);
    void* raw_memory = kmalloc(total_size);
    if (!raw_memory) {
        return NULL; // Allocation failed
    }

    // Calculate the aligned memory address
    uintptr_t raw_address = (uintptr_t)raw_memory + sizeof(void*);
    uintptr_t aligned_address = (raw_address + alignment - 1) & ~(alignment - 1);

    // Store the original pointer just before the aligned memory
    ((void**) aligned_address)[-1] = raw_memory;

    return (void*) aligned_address;
}

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
void* memcpy(void* dest, const void* src, size_t len) {
    char* d = (char*) dest;
    const char* s = (const char*) src;

    while (len--) {
        *d++ = *s++;
    }

    return dest;
}

static inline int is_aligned(uint64_t value, uint64_t align) {
    if (align == 0)
        align = sizeof(void*);
    return value % align == 0;
}

static inline uint64_t align_up(uint64_t value, uint64_t align) {
    return (value + align - 1) & ~(align - 1);
}
