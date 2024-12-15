#ifndef TRAP_H
#define TRAP_H

/*
    The virt board has a CLINT and PLIC.
*/

#include <stdint.h>

// Interrupts
#define INTERRUPT_FLAG                      (1UL << 63)
#define SOFTWARE_INTERRUPT      3
#define TIMER_INTERRUPT         7
#define EXTERNAL_INTERRUPT      11

// Exceptions
#define INSTRUCTION_ADDRESS_MISALIGNED      0
#define INSTUCITION_ACCESS_FAULT            1
#define ILLEGAL_INSTRUCTION                 2
#define BREAKPOINT                          3
#define LOAD_ADDRESS_MISALIGNED             4
#define LOAD_ACCESS_FAULT                   5
#define STORE_AMO_ADDRESS_MISALIGNED        6
#define STORE_AMO_ACCESS_FAULT              7
#define ENVIROMENT_CALL_U                   8
#define ENVIROMENT_CALL_S                   9
#define ENVIROMENT_CALL_M                   11

void init_trap_handler(void);

void trap_handler(void);
void handle_timer_interrupt();
void handle_external_interrupt();

void handle_illegal_instruction(uint64_t epc);
void handle_syscall(uint64_t epc);

#endif