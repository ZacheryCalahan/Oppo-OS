#include "headers/kernel.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdlib.h"
#include "../arch/headers/trap.h"
#include "headers/proc.h"
#include "../arch/headers/paging.h"
#include "../drivers/headers/blkio.h"
#include "../drivers/headers/fat.h"
#include "headers/fs.h"

#include <stdint.h>

extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

void kmain(void) {
	printf("\n\n");
	// Initialize kernel
	init_memory();
	init_trap_handler();
	init_proc();

	virtio_blk_init();
	init_fat32();

	struct FILE *lorem = open_file("TEST/LOREM.TXT");

	// Print the found file
	printf("Printing file below: {\n\n%s\n}\n\n", lorem->data);

	// Allow the scheduler to take control, and move to userspace!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);
	yield();
	
	PANIC("Exited out of scheduler!?");
}