

	.section .text

	.globl gdt_flush
	.type gdt_flush, @function

gdt_flush:
	# Disable Interrupts
	cli

	# Load the GDT
	xchgw %bx, %bx
	movl 4(%esp), %eax
	lgdt (%eax)

	# Re-enable interrupts
	#sti

	# Load Data Segment Registers
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	# Longjump to load code register
	jmp $0x08,$.flush
.flush:
	ret
