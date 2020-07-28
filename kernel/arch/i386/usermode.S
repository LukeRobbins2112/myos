
	.section .text
	.globl usermode_func
	.type usermode_func @function
usermode_func:
	movl $1, %eax
	movl $2, %edi
	movl $3, %esi
	cli
	ret
	
