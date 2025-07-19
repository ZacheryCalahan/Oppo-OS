#include "headers/kernel.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdlib.h"
#include "../arch/headers/trap.h"
#include "headers/proc.h"
#include "../arch/headers/paging.h"
#include "../drivers/headers/blkio.h"
#include "../drivers/headers/fat.h"

#include <stdint.h>

struct process *proc_a;
struct process *proc_b;

extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

void kmain(void) {
	printf("\n\n");
	// Initialize kernel
	init_memory();
	init_trap_handler();
	init_proc();

	virtio_blk_init();
	init_fat32();

	// char buf[SECTOR_SIZE];
	// read_write_disk(buf, 0, 0);
	// printf("first sector: %s\n", buf);

	// strcpy(buf, "hello from kernel!!!\n");
	// read_write_disk(buf, 0, 1);
	
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);

	yield();
	
	PANIC("Exited out of scheduler!?");
}