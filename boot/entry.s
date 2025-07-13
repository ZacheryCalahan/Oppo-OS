.global start
.section .init

start:
	/* Setup stack */
	la sp, stack_top
	call kmain
.end
