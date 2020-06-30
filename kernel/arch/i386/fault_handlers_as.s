
.section .text
.global page_fault
.global page_fault_handler

# setup page fault handler
page_fault:

	# push all registers before handling 
	pusha

	# Push condition code and faulting address
	# page_fault is not a function call, so esp was the error code
	# pusha pushes 8*4-byte registers, aso err code is 32 bytes up
	xchgw %bx, %bx
	pushl 32(%esp)
	movl %cr2, %eax
	pushl %eax
	call page_fault_handler
	xchgw %bx, %bx

	# reload CR3 with updated page directory
	movl %cr3, %ecx
	movl %ecx, %cr3
	
	# pop page_fault_handler args
	popl %eax
	popl %ecx
	

page_present:
	# pop all registers
	popa

	# pop error code
	popl %ecx

	#return
	iret

	
