#include "headers/kernel.h"
#include "headers/proc.h"
#include "headers/fs.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdlib.h"
#include "../arch/headers/trap.h"
#include "../arch/headers/paging.h"
#include "../drivers/headers/blkio.h"
#include "../drivers/headers/fat.h"

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

	// Create new FILE (or edit if you've already made it!)
	char *path = "TEST/NEWFILE.TXT";
	char *new_text = "EDITED NEW FILE! WOO!";
	uint64_t new_file_size = strlen(new_text);

	fat32_write_file_by_path(path, new_text, new_file_size, 0);
	struct FILE *new_file = open_file(path);
	if (new_file == NULL) {
		PANIC("Error after file write.");
	}
	printf("Printing new file below: {\n\n%s\n}\n\n", new_file->data);
	close_file(new_file);

	// Allow the scheduler to take control, and move to userspace!
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);
	yield();
	
	PANIC("Exited out of scheduler!?");
}