#include "headers/kernel.h"
#include "../klib/headers/string.h"
#include "../klib/headers/stdlib.h"
#include "../arch/headers/trap.h"
#include "headers/proc.h"
#include "../arch/headers/paging.h"

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
	
	create_process(_binary_shell_bin_start, (size_t) _binary_shell_bin_size);

	yield();
	
	PANIC("Exited out of scheduler!?");
}