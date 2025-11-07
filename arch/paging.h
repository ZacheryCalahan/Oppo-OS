#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define SATP_SV39   (8UL << 60) // 64-bit version MMU, 512 GiB mem
#define SATP_SV32   (1U << 31)  // 32-bit version MMU, 4GiB (Unused in this implementation)
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

/**
 * Maps a page of memory at the specified virtual address to the page tables used by the MMU
 * 
 * @param vaddr Virtual address where the page should be mapped.
 * @param paddr Physical address of the page to map.
 * @param flags Permissions of the page
 */
void map_page(uint64_t *root_table, uint64_t vaddr, uint64_t paddr, uint32_t flags);

/**
 * Translates a virtual address to a physical address for use in the kernel.
 * 
 * @param root_table The address of the base of the root table
 * @param vaddr The virtual address to convert
 * 
 * @returns Physical address of the given virtual address
 */
uint64_t *vaddr_to_paddr(uint64_t *root_table, uint64_t vaddr);
#endif
