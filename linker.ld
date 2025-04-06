SECTIONS {
	. = 0x80000000; /* Start of DRAM */

	/* Include entry point at start of binary */
	.text : ALIGN(4K) {
		KEEP(*(.init));
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
}
