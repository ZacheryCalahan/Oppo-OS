#include "kernel.h"
#include "schedule/proc.h"
#include "fs/fs.h"
#include "../klib/string.h"
#include "../klib/stdlib.h"
#include "../arch/trap.h"
#include "../arch/paging.h"
#include "../drivers/virtio/virtio_blkio.h"
#include "../drivers/fs/ztfs.h"
#include "../drivers/virtio/virtio_gpu.h"
#include "../drivers/time/rtc.h"

#include <stdint.h>

extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

void kmain(void) {
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	
	// Initialize kernel
	init_memory();
	init_trap_handler();

	// Kernel Processes
	init_proc();

	// Kernel Device Drivers
	virtio_blk_init();
	//virtio_gpu_init();

	// Pass control to user programs!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size); // Creator of processes
	yield();
	
	PANIC("Exited out of scheduler. Halting!");
}