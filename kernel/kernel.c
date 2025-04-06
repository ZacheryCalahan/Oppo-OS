#include <stdint.h>
#include "headers/kernel.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdio.h"
#include "../klib/headers/stdlib.h"
#include "headers/trap.h"
#include "headers/riscv.h"

void kmain(void) {
	// Get the address of the device tree and parse it. This must be done first after entering the 
	// kernel, with ABSOLUTELY no calls prior to ensure that the register isn't overwritten.
	uint64_t fdt_addr = r_a1();
	klog("Kernel initialization", INFO);
	
	// Enable CPU Features
	init_trap_handler();

	// Initialize the Heap for use (page allocator)
	init_memory(KERNEL_HEAP_START, KERNEL_HEAP_END);

	// Initialize all the drivers we need here.
	
	// Hang, as kernel has nothing left to do.
	while(1);
	return;
}

void panic() {
	printf("Kernel panic!");
	// Hang
	while(1);
}

void klog(char* log, enum LogType type) {
	printf("[%s\033[1;39m] | %s\n", log_type_strings[type], log);
}

