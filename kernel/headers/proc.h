#ifndef PROC_H
#define PROC_H

#include <stdint.h>

void init_proc(); // Initialize the scheduler
struct process *create_process(uint64_t pc); // Register a process to the scheduler
void yield(void);

// Structure holding all data needed to handle a process
struct process {
    uint32_t pid;           // Process ID
    uint32_t state;         // Process state
    uint64_t sp;           // Stack pointer
    uint64_t *page_table;   // Level 2 page table (root table in sv39)
    uint8_t stack[8192];    // Kernel stack
};

#endif