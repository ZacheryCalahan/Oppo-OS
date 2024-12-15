#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

void init_memory(uint64_t heap_start, uint64_t heap_end);
void *kmalloc(size_t size);
void* kalloc(void);
void kfree(void *ptr);
void* memset(void *dest, int value, size_t n);
void* kaligned_alloc(size_t alignment, size_t size);

#endif