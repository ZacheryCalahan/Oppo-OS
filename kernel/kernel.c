#include <stdint.h>
#include "headers/kernel.h"
#include "stdlib/headers/string.h"
#include "stdlib/headers/stdio.h"
#include "stdlib/headers/stdlib.h"
#include "../drivers/headers/virtio.h"
#include "../drivers/headers/video.h"
#include "headers/trap.h"
#include "riscv.h"

void init_drivers() {
	int status = virtio_gpu_init_mmio();
	if (status != 0) {
		printf("Error initializing the GPU driver. Return code: %d\r\n", status);
	} else {
		printf("GPU Driver initialized.\r\n");
	}
}

void kmain(void) {
	// uint64_t fdt_addr = r_a1();
	// printf("Value of a1 register is: 0x%x", fdt_addr);
	// Initialize all the drivers we need here.
	init_drivers();
	init_trap_handler();
	init_memory(KERNEL_HEAP_START, KERNEL_HEAP_END);

	printf("Test of the snprintf:\n\r");
	char* buffer[64];

	snprintf(*buffer, 64, "This is a text, with a buffer %x in size", 64);
	printf("%s", buffer);

	// Hang
	while(1);

	return;
}

void panic() {
	// Hang
	while(1);
}



