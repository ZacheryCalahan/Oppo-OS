#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define SATP_SV39   (8UL << 60) // 64-bit version MMU, 512 GiB mem
#define SATP_SV32   (1U << 31)  // 32-bit version MMU, 4GiB
#define PAGE_V      (1 << 0)    // "Valid" bit (entry is enabled)
#define PAGE_R      (1 << 1)    // Readable
#define PAGE_W      (1 << 2)    // Writable
#define PAGE_X      (1 << 3)    // Executable
#define PAGE_U      (1 << 4)    // User (accessible in user mode)
#define PAGE_H      (1 << 5)    // Global
#define PAGE_A      (1 << 6)    // Accessed
#define PAGE_D      (1 << 7)    // Dirty

#define PAGE_SHIFT  12
#define VPN_SHIFT   9
#define VPN_MASK    0x1ff

#define VA_BITS     39
#define VPN2_SHIFT  30
#define VPN1_SHIFT  21
#define VPN0_SHIFT  12

void map_page(uint64_t *root_table, uint64_t vaddr, uint64_t paddr, uint32_t flags);

#endif