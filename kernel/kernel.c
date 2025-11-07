#include "kernel.h"
#include "schedule/proc.h"
#include "fs/fs.h"
#include "../klib/string.h"
#include "../klib/stdlib.h"
#include "../arch/trap.h"
#include "../arch/paging.h"
#include "../drivers/virtio/virtio_blkio.h"
#include "../drivers/fs/ext2.h"
#include "../drivers/virtio/virtio_gpu.h"

#include <stdint.h>

extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

void kmain(void) {
	printf("\n\n");
	
	// Initialize kernel
	printf("\nkernel: Init of arch code\n");
	init_memory();
	init_trap_handler();
	init_proc();

	// Device Drivers
	printf("\nkernel: Init of device drivers\n");
	virtio_blk_init();
	//virtio_gpu_init();

	printf("\nkernel: Init of File systems\n");
	init_ext2();

	printf("\nkernel: Loading userland\n");

	// Allow the scheduler to take control, and move to userspace!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);

	printf("\nkernel: Passing control to programs.\n\n\n");

	// Kernel init finished, run user programs.
	yield();
	
	PANIC("Exited out of scheduler!?");
}