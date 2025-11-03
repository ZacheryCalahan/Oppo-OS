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


	struct inode *node = get_inode("hello.txt");
	if (node == NULL) {
		PANIC("inode not found!");
	}

	char* text = read_block(node->direct_block_pointer[0]);
	printf("\n%s\n", text);

	//virtio_gpu_init();

	// Allow the scheduler to take control, and move to userspace!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);
	yield();
	
	PANIC("Exited out of scheduler!?");
}