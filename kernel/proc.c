#include "headers/proc.h"
#include "headers/kernel.h"
#include <stddef.h>

#define PROCS_MAX 8
#define PROC_UNUSED 0
#define PROC_RUNNABLE 1

struct process *current_proc;
struct process *idle_proc;

__attribute__((naked))
void switch_context(uint64_t *prev_sp, uint64_t *next_sp) {
    __asm__ __volatile__ (
        // Save callee-saved registers onto the current process's stack.
        "addi sp, sp, -13 * 8\n" 
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
        "ld sp, (a1)\n"         // Switch stack pointer (sp) here

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
        "addi sp, sp, 13 * 8\n"
        "ret\n"
    );
}

struct process procs[PROCS_MAX];

void init_proc() {
    idle_proc = create_process((uint64_t) NULL);
	idle_proc->pid = 0;
    current_proc = idle_proc;
}

struct process *create_process(uint64_t pc) {
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc) {
        PANIC("No free process slots!");
    }

    // Stack callee-saved registers
    uint64_t *sp = (uint64_t *) (&proc->stack[sizeof(proc->stack)]);
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
    *--sp = (uint64_t) pc;          // ra

    // Init fields
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint64_t) sp;
    return proc;
}

void yield(void) {
    // Search for a runnable process
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++) {
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
            next = proc;
            break;
        }
    }

    // In case of no runnable process other than the current one, just return to current process.
    if (next == current_proc) {
        return;
    }

    // Reset the kernel stack for exception handling
    __asm__ __volatile__ (
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint64_t) &next->stack[sizeof(next->stack)])
    );

    // Context switch
    struct process *prev = current_proc;
    current_proc = next;
    switch_context(&prev->sp, &next->sp);

}

