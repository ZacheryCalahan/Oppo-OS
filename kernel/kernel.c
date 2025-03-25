#include <stdint.h>
#include "headers/kernel.h"
#include "stdlib/headers/string.h"
#include "stdlib/headers/stdio.h"
#include "stdlib/headers/stdlib.h"
#include "../drivers/headers/virtio.h"
#include "../drivers/headers/video.h"
#include "headers/trap.h"
#include "riscv.h"
#include "../drivers/headers/fdt.h"
#include "stdlib/headers/scanner.h"

void init_drivers() {
	int status = virtio_gpu_init_mmio();
	if (status != 0) {
		printf("Error initializing the GPU driver. Return code: %d\n", status);
	} else {
		printf("GPU Driver initialized.\n");
	}
}

void parse_fdt(uint64_t address) {
	// Handle all DTB information for further setup. This'll require memory management tools, which will then use the values
	// of the DTB to restrict memory access to memory reservation areas in the DTB.
	printf("Initializing the FDT at location 0x%x\n", address);
	uint32_t returnValueFDT = 0;
	if (returnValueFDT == init_fdt(address)) {
		// In the case that the initialization of the FDT doesn't succeed, handle it here.
		switch (returnValueFDT) {
			case 1:
				printf("FDT not found.\n");
				break;

			case -1:
				printf("FDT ended without an end node.\n");
				break;

			default:
				printf("Unknown error with FDT parse.\n");
		}
	}
	
}

void kmain(void) {
	// Get the address of the device tree and parse it. This must be done first after entering the kernel, with no calls prior.
	uint64_t fdt_addr = r_a1();

	// Initialize all the drivers we need here.
	//init_drivers();
	init_trap_handler();
	init_memory(KERNEL_HEAP_START, KERNEL_HEAP_END);
	parse_fdt(fdt_addr);

	// Hang
	while(1);

	return;
}

void panic() {
	// Hang
	while(1);
}



