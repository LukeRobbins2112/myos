

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

# IRQ definitions
	
irq0:
	pusha
	cld
	call irq0_handler
	popa
	iret
 
irq1:
	pusha
	cld
	call irq1_handler
	popa
	iret
 
irq2:
	pusha
	cld
	call irq2_handler
	popa
	iret
 
irq3:
	pusha
	cld
	call irq3_handler
	popa
	iret
 
irq4:
	pusha
	cld
	call irq4_handler
	popa
	iret
 
irq5:
	pusha
	cld
	call irq5_handler
	popa
	iret
 
irq6:
	pusha
	cld
	call irq6_handler
	popa
	iret
 
irq7:
	pusha
	cld
	call irq7_handler
	popa
	iret
 
irq8:
	pusha
	cld
	call irq8_handler
	popa
	iret
 
irq9:
	pusha
	cld
	call irq9_handler
	popa
	iret
 
irq10:
	pusha
	cld
	call irq10_handler
	popa
	iret
 
irq11:
	pusha
	cld
	call irq11_handler
	popa
	iret
 
irq12:
	pusha
	cld
	call irq12_handler
	popa
	iret
 
irq13:
	pusha
	cld
	call irq13_handler
	popa
	iret
 
irq14:
	pusha
	cld
	call irq14_handler
	popa	
	iret
 
irq15:
	pusha
	cld
	call irq15_handler
	popa	
	iret

load_idt:
	movl 4(%esp), %eax
	lidt (%eax)
	sti
	ret
	
