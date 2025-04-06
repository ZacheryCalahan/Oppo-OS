// Handler of the mtvec stuffs.
#include <stdint.h>
#include "headers/riscv.h" // Header file for RISC-V specific implementations
#include "headers/trap.h"
#include "../klib/headers/stdio.h"

void init_trap_handler() {
    // Enable global interrupts
    uint64_t mstatus;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));   // Read mstatus
    mstatus |= (1UL << 3);                              // Flip bit
    asm volatile("csrw mstatus, %0" : : "r" (mstatus)); // Writeback

    // Enable external interrupts
    uint64_t mie = r_mie(); // Read mie
    mie |= (1UL << 11);
    asm volatile ("csrw mie, %0" : : "r"(mie));

    // Set Trap Vector
    uint64_t trap_vector_address = (uint64_t)&trap_handler;
    asm volatile("csrw mtvec, %0" : : "r"(trap_vector_address));
}

// Non-vectored trap handler.
void trap_handler() {
    uint64_t cause = r_mcause();
    uint64_t epc = r_mepc();
    
    if (cause & INTERRUPT_FLAG) {
        // Interrupt
        uint64_t irq = cause & 0xFF; // Mask the upper bits to get the interrupt type
        switch (irq) {
            case TIMER_INTERRUPT: handle_timer_interrupt(); break;
            case EXTERNAL_INTERRUPT: handle_external_interrupt(); break;
            case SOFTWARE_INTERRUPT: printf("Software Interrupt: %d\n", irq); break;
            default: {
                printf("Unhandled Interrupt of \"%x\". Halting.\n", irq); 
                while (1);
            }
        }
    } else {
        // Exception
        if (cause == ILLEGAL_INSTRUCTION) {
            handle_illegal_instruction(epc);
        } else if (cause == ENVIROMENT_CALL_U) {
            handle_syscall(epc);
        } else {
            printf("Unhandled Exception: 0x%x at 0x%x\n", cause, epc);
            while(1);
        }
    }

    // After handling the trap, return to prior state
    w_mepc(epc);
    asm volatile("mret"); 
}

void handle_timer_interrupt() {
    printf("timer!\n");
}

void handle_external_interrupt() {
    printf("external int!\n");
}

void handle_illegal_instruction(uint64_t epc) {
    printf("Illegal instruction @%x\n", epc);
    while (1);
}

void handle_syscall(uint64_t epc) {
    printf("Syscall!\n");
}