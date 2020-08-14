
	
	.section .text

.equ esp_offset,  0
.equ esp0_offset, 4
.equ cr3_offset,  8
	
.equ tss_esp0,    4
	
# address of "current" task control block
.extern curr_tcb
.extern context_tss

# task initialization
.extern setup_new_task
	
#  Caller must disable IRQs before calling, and enable after return	
.globl switch_to_task_asm
.type switch_to_task_asm, @function
switch_to_task_asm:

	# Save previous task's state
	# %eax, %ecx, edx are caller saved;
	# %eip was pushed by call instruction

	# xchgw %bx, %bx
	
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %ebp

	#  Load previous task
	# save prev task %esp
	movl curr_tcb, %edi
	movl %esp, esp_offset(%edi) 

	#  Load next task's state

	# 5 = past 4 pushed regs, return val
	movl $(4+1), %ebx
	movl (%esp, %ebx, 4), %esi 
	movl %esi, curr_tcb

	# Get values from new task
	movl esp_offset(%esi), %esp
	movl cr3_offset(%esi), %eax
	movl esp0_offset(%esi), %ebx

	#  Set new esp0 in the TSS
	movl %ebx, (context_tss + tss_esp0)

	#  swap address space if different
	movl %cr3, %ecx
	cmpl %eax, %ecx
	je .VAS_equal
	movl %eax, %cr3

.VAS_equal:
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx

	ret


# setup new task
.globl setup_new_task_asm
.type setup_new_task_asm, @function
setup_new_task_asm:
	# we do not CALL this function, we RETURN to it
	# when we begin the function, entry_EIP is already pushed onto the stack
	# just call the actual setup helper
	call setup_new_task
	
	
