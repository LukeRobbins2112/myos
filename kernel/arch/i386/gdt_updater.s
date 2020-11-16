

	.section .text

	.globl gdt_flush
	.type gdt_flush, @function

gdt_flush:
	# Disable Interrupts
	cli

	# Load the GDT
	movl 4(%esp), %eax
	lgdt (%eax)

	# Load Data Segment Registers
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	# Far jump to load code register
	jmp $0x08,$.flush
.flush:
	ret
