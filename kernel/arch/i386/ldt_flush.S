

	.section .text

	.globl ldt_flush
	.type ldt_flush, @function
ldt_flush:
	movw $0x2B, %ax
	ltr %ax
	ret
