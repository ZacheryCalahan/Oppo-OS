.global start

.section .init
start:
	/* Setup stack */
	la sp, stack_top

	/* Clear the BSS section */
	la t5, bss_start
	la t6, bss_end
	
bss_clear:
	sd zero, (t5)
	addi t5, t5, 8
	bltu t5, t6, bss_clear

	call kmain
.end
