

.section .text
.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15
 
.global load_idt
 
.global irq0_handler
.global irq1_handler
.global irq2_handler
.global irq3_handler
.global irq4_handler
.global irq5_handler
.global irq6_handler
.global irq7_handler
.global irq8_handler
.global irq9_handler
.global irq10_handler
.global irq11_handler
.global irq12_handler
.global irq13_handler
.global irq14_handler
.global irq15_handler	

# Page Fault Handler
.global page_fault
.global page_fault_handler

# IRQ definitions
	
irq0:
	pusha
	call irq0_handler
	popa
	iret
 
irq1:
	pusha
	call irq1_handler
	popa
	iret
 
irq2:
	pusha
	call irq2_handler
	popa
	iret
 
irq3:
	pusha
	call irq3_handler
	popa
	iret
 
irq4:
	pusha
	call irq4_handler
	popa
	iret
 
irq5:
	pusha
	call irq5_handler
	popa
	iret
 
irq6:
	pusha
	call irq6_handler
	popa
	iret
 
irq7:
	pusha
	call irq7_handler
	popa
	iret
 
irq8:
	pusha
	call irq8_handler
	popa
	iret
 
irq9:
	pusha
	call irq9_handler
	popa
	iret
 
irq10:
	pusha
	call irq10_handler
	popa
	iret
 
irq11:
	pusha
	call irq11_handler
	popa
	iret
 
irq12:
	pusha
	call irq12_handler
	popa
	iret
 
irq13:
	pusha
	call irq13_handler
	popa
	iret
 
irq14:
	pusha
	call irq14_handler
	popa	
	iret
 
irq15:
	pusha
	call irq15_handler
	popa	
	iret


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

	
load_idt:
	xchgw %bx, %bx
	movl 4(%esp), %eax
	lidt (%eax)
	sti
	ret
	
