#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
extern char __free_ram[], __free_ram_end[];

void init_memory();
void *kmalloc(size_t size);
void* kalloc(void);
void kfree(void *ptr);
void* memset(void *dest, int value, size_t n);
void* kaligned_alloc(size_t alignment, size_t size);
void* memcpy(void* dest, const void* src, size_t len);
//void* realloc(void *ptr, size_t size); Implement this soon!

#endif