#ifndef PAGING_H
#define PAGING_H

// riscv_cpu_do_interrupt: hart:0, async:0, cause:000000000000000f, epc:0x0000000080200604, tval:0x0000000010000000, desc=store_page_fault
// 0x80201268
// Issue is with non-mapped uart!
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
#define VPN_MASK    0x1ff
#define VA_BITS     39
#define VPN2_SHIFT  30
#define VPN1_SHIFT  21
#define VPN0_SHIFT  12
#define PPN_MASK    0x00000FFFFFFFFFFF



void map_page(uint64_t *root_table, uint64_t vaddr, uint64_t paddr, uint32_t flags);

#endif
