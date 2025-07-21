SECTIONS {
	. = 0x80200000; /* expected address after OpenSBI */
	__kernel_base = .;

	
	.text : ALIGN(4K) {
		KEEP(*(.init)); /* Include entry point at start of binary */
		*(.text);
	}
	.bss : ALIGN(4K) {
		PROVIDE(bss_start = .);
		*(.bss);
		. += 8192;
		PROVIDE(stack_top = .);
		. += 8192;
		PROVIDE(global_pointer = .);
		PROVIDE(bss_end = .);
	}
	.rodata : ALIGN(4K) {
		*(.rodata);
		*(.rodata.str1.1*)
	}
	.data : ALIGN(4K) {
		*(.data);
	}

	/* Prevent build-id from occupying this space */
  	/DISCARD/ : {
    	*(.note.gnu.build-id)
    	*(.eh_frame)
    	*(.riscv.attributes)
    	*(.comment)
  	}

	/* Stack */
	. = ALIGN(16);
	. += 128 * 1024; /* 128KB */
	__stack_top = .;

	/* Kernel RAM */
	. = ALIGN(4096);
	__free_ram = .;
	. += 128 * 1024 * 1024; /* 128MB */
	__free_ram_end = .;
}
