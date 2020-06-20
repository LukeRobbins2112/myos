# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# setup Page Directory and first Page Table
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table0:
	.skip 4096

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:

	xchgw %bx, %bx
	# physical addr of boot_page_table0
	movl $(boot_page_table0 - 0xC0000000), %edi

	# First physical address to map
	movl $0, %esi
	# Number of pages to map (1023) - 1024th is VGA
	movl $1023, %ecx

1:
	# only map the kernel (for now)
	# First comparison skips addresses 0-kernel_start
	# Second comparison skips addresses kernel_end+
	cmpl $(_kernel_start - 0xC0000000), %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	# at this point we're in kernel addresses
	# identity map these entries
	xchgw %bx, %bx

	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:	
	# increment boot_page_table0 pointer %edi to next entry
	# increment physical address pointer %esi to next 4K chunk
	# then continue looping to fill the next entry
	addl $4096, %esi
	addl $4, %edi
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable"
	xchgw %bx, %bx
	movl $(0x000B8000 | 0x003), (boot_page_table0 - 0xC0000000 + (1023*4))

identity_map:	
	# Now it's time to actually map the kernel
	# we map it to 2 places - one for identity, one for higher half
	# The 768th page table is what begins at 0xC0000000
	movl $(boot_page_table0 - 0xC0000000 + 0x003), (boot_page_directory - 0xC0000000 + 0)
	movl $(boot_page_table0 - 0xC0000000 + 0x003), (boot_page_directory - 0xC0000000 + (768 * 4))

enable_paging:
	# Set the control registers

	# load the boot_page_directory into cr3
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# enable paging and write-protect bit
	movl %cr0, %ecx
	orl 0x80010000, %ecx
	movl %ecx, %cr0

	# jump to higher half with absolute jump
	lea 4f, %ecx
	jmp *%ecx

4:
	# At this point, paging is set up and enabled
	# Unmap the identity mapping as it is now unnecessary. 
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# -------------------------------
	# Enter main kernel functionality
	# -------------------------------
	
	# Setup stack
	movl $stack_top, %esp

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
5:	hlt
	jmp 5b
	
.size _start, . - _start
