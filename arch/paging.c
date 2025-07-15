#include "headers/paging.h"
#include "../klib/headers/stdlib.h"
#include "headers/riscv.h"
#include "../kernel/headers/kernel.h"

/**
   Maps a page of memory at the specified virtual address to the page tables used by the MMU
  
   @param vaddr Virtual address where the page should be mapped.
   @param paddr Physical address of the page to map.
   @param flags Permissions of the page
 */
void map_page(uint64_t *root_table, uint64_t vaddr, uint64_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE)) {
        PANIC("unaligned vaddr %x", vaddr);
    }
    if (!is_aligned(paddr, PAGE_SIZE)) {
        PANIC("unaligned paddr %x", paddr);
    }

    if (flags & 0xe == 0) {
        PANIC("Attempted map with no permissions!");
    }
    // NOTE: This assumes that we want the smallest granularity of page size, 4096B

    uint64_t vpn2 = (vaddr >> VPN2_SHIFT) & VPN_MASK; // Retrieve the virtual page number for table 2
    if ((root_table[vpn2] & PAGE_V) == 0) { // Check for no entry, and make an entry for next table if none exists.
        uint64_t table1_paddr = (uint64_t) kalloc(); // Allocate new page to hold next level of table
        root_table[vpn2] = ((table1_paddr / PAGE_SIZE) << 10) | PAGE_V; // Assign a new PTE. (Branch to level 1)
    }
    // Get p_address of table pointed to by root_table PTE
    uint64_t *table1 = (uint64_t *) ((root_table[vpn2] >> 10) << PAGE_SHIFT);
    

    uint64_t vpn1 = (vaddr >> VPN1_SHIFT) & VPN_MASK;
    if ((table1[vpn1] & PAGE_V) == 0) { // Check for no entry, and make an entry for next table if none exists.
        // Create the 1st level page table if it doesn't exist
        uint64_t table0_paddr = (uint64_t) kalloc();
        table1[vpn1] = ((table0_paddr / PAGE_SIZE) << 10) | PAGE_V; // Assign a new PTE. (Branch to level 0)
    }
    uint64_t *table0 = (uint64_t *) ((table1[vpn1] >> 10) << PAGE_SHIFT);

    uint64_t vpn0 = (vaddr >> VPN0_SHIFT) & VPN_MASK;
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V; // Assign a new PTE. (Leaf)
}


