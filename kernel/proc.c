#include "headers/proc.h"
#include "headers/kernel.h"
#include "../klib/headers/stdlib.h"
#include "../klib/headers/stdio.h"
#include "../arch/headers/paging.h" // Must be changed if HAL is implemented.
#include "../drivers/headers/virtio.h"
#include <stddef.h>

#define PROCS_MAX 8 // Max processes that can be registered to the scheduler at a given time

#define PROC_UNUSED 0
#define PROC_RUNNABLE 1
#define PROC_EXITED 2

struct process *current_proc;
struct process *idle_proc; // Process of the kernel, denoted by id 0.
struct process procs[PROCS_MAX];

extern char __kernel_base[];
#define USER_BASE 0x1000000 // Arbitrary start address of all userspace programs.

__attribute__((naked))
void switch_context(uint64_t *prev_sp, uint64_t *next_sp) {
    __asm__ __volatile__ (
        
        // Save callee-saved registers onto the current process's stack.
        "addi sp, sp, -13 * 8\n" // Allocate space for 13 64-bit (8 byte) registers
        "sd ra,  0  * 8(sp)\n"   
        "sd s0,  1  * 8(sp)\n"
        "sd s1,  2  * 8(sp)\n"
        "sd s2,  3  * 8(sp)\n"
        "sd s3,  4  * 8(sp)\n"
        "sd s4,  5  * 8(sp)\n"
        "sd s5,  6  * 8(sp)\n"
        "sd s6,  7  * 8(sp)\n"
        "sd s7,  8  * 8(sp)\n"
        "sd s8,  9  * 8(sp)\n"
        "sd s9,  10 * 8(sp)\n"
        "sd s10, 11 * 8(sp)\n"
        "sd s11, 12 * 8(sp)\n"

        // Switch the stack pointer.
        "sd sp, (a0)\n"         // *prev_sp = sp;
        "ld sp, (a1)\n"         // Switch stack pointer (sp) here (Next process's stack)

        // Restore callee-saved registers from the next process's stack.
        "ld ra,  0  * 8(sp)\n"  
        "ld s0,  1  * 8(sp)\n"
        "ld s1,  2  * 8(sp)\n"
        "ld s2,  3  * 8(sp)\n"
        "ld s3,  4  * 8(sp)\n"
        "ld s4,  5  * 8(sp)\n"
        "ld s5,  6  * 8(sp)\n"
        "ld s6,  7  * 8(sp)\n"
        "ld s7,  8  * 8(sp)\n"
        "ld s8,  9  * 8(sp)\n"
        "ld s9,  10 * 8(sp)\n"
        "ld s10, 11 * 8(sp)\n"
        "ld s11, 12 * 8(sp)\n"
        "addi sp, sp, 13 * 8\n" // Update `sp` after pop of 13 64-bit (8 byte) registers from the stack
        "ret\n"
    );
}

__attribute__((naked))
void user_entry(void) {
    __asm__ __volatile__ (
        "csrw sepc, %[sepc]\n"
        "csrw sstatus, %[sstatus]\n"
        "sret\n"
        :
        : [sepc] "r" (USER_BASE),
          [sstatus] "r" (SSTATUS_SPIE)
    );
}

/*
    Create the kernel process within the scheduler as the idle process.
*/
void init_proc() {
    idle_proc = create_process(NULL, 0);
	idle_proc->pid = 0;
    current_proc = idle_proc;
}

struct process *create_process(const void *image, size_t image_size) {
    struct process *proc = NULL;
    int i; // Process slot
    for (i = 0; i < PROCS_MAX; i++) { // Search for empty process slot
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc) {
        PANIC("No free process slots!");
    }

    // Initialize processes stack callee-saved registers for context switching (saved on call from process)
    uint64_t *sp = (uint64_t *) &proc->stack[sizeof(proc->stack)];
    *--sp = 0;                      // s11
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint64_t) user_entry;  // ra

    proc->sp = (uint64_t) sp;

    // Create a page table (this is the root table for the process)
    uint64_t *page_table = (uint64_t *) kalloc(1); // Allocate one page to the page tables

    // Map kernel pages
    for (uint64_t paddr = (uint64_t) __kernel_base; 
        paddr < (uint64_t) __free_ram_end; paddr += PAGE_SIZE) {
        map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);
    }

    map_page(page_table, VIRTIO_PADDR, VIRTIO_PADDR, PAGE_R | PAGE_W); // Map virtio to kernel map
    
    // Map user pages
    for (uint64_t off = 0; off < image_size; off += PAGE_SIZE) {
        uint64_t page = (uint64_t) kalloc(1);

        // Handle case where the data to be copied is small than the page size
        size_t remaining = image_size - off;
        size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

        // Fill and map the page
        memcpy((void *) page, image + off, copy_size);
        map_page(page_table, USER_BASE + off, page, PAGE_U | PAGE_R | PAGE_W | PAGE_X);
    }


    proc->page_table = page_table;
        
    // Init remaining struct members
    proc->pid = i + 1; // PID is always the current process slot + 1
    proc->state = PROC_RUNNABLE;

    return proc;
}

/*
    Pass control to the scheduler
*/
void yield(void) {
    // Search for a runnable process
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++) {
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) { // Do not run idle task
            next = proc;
            break;
        }
    }

    // In case of no runnable process other than the current one, just return to current process.
    if (next == current_proc) {
        return;
    }

    // Context Switch here
    struct process *prev = current_proc;
    current_proc = next;
    // Reset the kernel stack for trap handling and switch top level page table pointer (satp) to next process
    __asm__ __volatile__ (
        
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint64_t) &next->stack[sizeof(next->stack)]) // Holds the sp for the context to resume under
    );
    
    
    uint64_t new_satp = (SATP_SV39 | (uint64_t) next->page_table / PAGE_SIZE);

    __asm__ __volatile__ (
        "sfence.vma\n"
        "csrw satp, %[satp]\n"
        "sfence.vma\n"
        :
        : [satp] "r" ((uint64_t) new_satp) // Stores the PPN (physical address / page size) to `satp`, along with sv39 mode.
    );

    switch_context(&prev->sp, &next->sp);


}

void proc_exit(void) {
    printf("Process %d exited\n", current_proc->pid);
    current_proc->state = PROC_EXITED;
    yield();
}