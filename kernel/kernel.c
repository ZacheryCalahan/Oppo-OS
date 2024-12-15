#include <stdint.h>
#include "headers/kernel.h"
#include "stdlib/headers/string.h"
#include "stdlib/headers/stdio.h"
#include "stdlib/headers/stdlib.h"
#include "../drivers/headers/virtio.h"
#include "../drivers/headers/video.h"
#include "headers/trap.h"



void init_drivers() {
	int status = virtio_gpu_init_mmio();
	if (status != 0) {
		printf("Error initializing the GPU driver. Return code: %d\n", status);
	} else {
		printf("GPU Driver initialized.\n");
	}
}

void kmain(void) {
	// Initialize all the drivers we need here.
	init_drivers();
	init_trap_handler();
	init_memory(KERNEL_HEAP_START, KERNEL_HEAP_END);

	

	// Hang
	while(1);

	return;
}

void panic() {
	
}



