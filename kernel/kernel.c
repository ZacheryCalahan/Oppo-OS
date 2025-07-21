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

	uint32_t file_size;
	uint8_t* file = (uint8_t *) fat32_get_file_by_path("USER/USER", &file_size);
	printf("File size: %d\n", file_size);
	if (file == NULL) {
		PANIC("File was null?");
	}

	// Print the found file
	printf("Printing file below: {\n%s\n}\n", file);

	
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);

	yield();
	
	PANIC("Exited out of scheduler!?");
}