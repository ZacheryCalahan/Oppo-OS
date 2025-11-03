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
	init_memory();
	init_trap_handler();
	init_proc();

	// File Handling
	virtio_blk_init();
	init_ext2();

	// test of fs implementation
	filedec_t file = open_file("hello.txt", O_RW);
	char* buf = kalloc(1);
	while (read_file(file, buf, PAGE_SIZE)) { // Read in all available bytes
		printf("\n%s\n", buf);
	}

	close_file(file);
	kfree_size(buf, PAGE_SIZE);

	//virtio_gpu_init();

	// Allow the scheduler to take control, and move to userspace!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);
	yield();
	
	PANIC("Exited out of scheduler!?");
}