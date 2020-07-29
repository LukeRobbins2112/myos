
	.section .text
	.globl jump_usermode
	.type jump_usermode @function
jump_usermode:
	cli
	movw $0x23, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	movl %esp, %eax
	pushl $0x23
	pushl %eax
	pushf
	pushl $0x1B
	pushl $usermode_func
	iret
