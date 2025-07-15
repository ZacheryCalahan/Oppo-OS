#include "headers/kernel.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdlib.h"
#include "../arch/headers/trap.h"
#include "headers/proc.h"

#include <stdint.h>

struct process *proc_a;
struct process *proc_b;

void delay(void) {
	for (int i = 0; i < 30000000; i++) {
		__asm__ __volatile__("nop");
	}
}

void proc_a_entry(void) {
	printf("Starting process A\n");
	while (1) {
		putc('A');
		yield();
		delay();
	}
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        putc('B');
        yield();
    }
}

void kmain(void) {
	printf("\n\n");
	// Initialize kernel
	init_memory();
	init_proc();
	init_trap_handler();
	
	// Test interrupt
	//asm volatile ("unimp");

	

	// Test procs
	printf("Processes created: Test\n");
	proc_a = create_process((uint64_t) proc_a_entry);
	proc_b = create_process((uint64_t) proc_b_entry);
	printf("Kernel -> Scheduler\n");
	yield();
	
	PANIC("Exited out of scheduler!?");
	
	// Power save here when nothing else can be done.
	while(1) {
		asm volatile("wfi");
	}

	// Should never reach, but for sanity.
	PANIC("Kernel end!");
}