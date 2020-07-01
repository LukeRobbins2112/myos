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
.global boot_page_directory
boot_page_directory:
	.skip 4096
boot_page_table0:
	.skip 4096

# save the start and # of pages for pmm
.section .data
.global pmm_frames
.global pmm_num_frames
pmm_frames:	 .long 0x0
pmm_num_frames:	 .long 0x0

# maintain pointer for kernel workspace
.global k_workspace_end
k_workspace_end: .long 0x0

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# immediate breakpoint
	xchgw %bx, %bx
	# physical addr of boot_page_table0
	movl $(boot_page_table0 - 0xC0000000), %edi

	# First physical address to map
	movl $0, %esi
	# Number of pages to map (1024)
	# loop instruction decrements %ecx
	movl $1024, %ecx

1:
	# Map the first 4MB
	# identity map: grab phys addr, mark as present, add PTE
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
	# get the next 4K-aligned address after the kernel end
	# set the current kernel workspace ptr to this addr
	xchgw %bx, %bx
	movl $(_kernel_end), %ecx
	addl $0xFFF, %ecx
	andl $0xFFFFF000, %ecx
	movl %ecx, (k_workspace_end - 0xC0000000)
	
	# setup pmm pages - get first 4K-aligned page after kernel
	# just doing one page for now
	movl %ecx, (pmm_frames - 0xC0000000)
	movl $0x8000, (pmm_num_frames - 0xC0000000)

	# update kernel workspace to start after pmm frames
	addl $4096, (k_workspace_end - 0xC0000000)

	# Map VGA video memory to 0xC03FF000 as "present, writable"
	# No longer have to do this now that you map the whole first 4MB
	# Just use the linking adjustment, e.g. 0xC00B8000
	# movl $(0x000B8000 | 0x003), (boot_page_table0 - 0xC0000000 + (1023*4))

identity_map:	
	# Now it's time to actually map the kernel
	# we map it to 2 places - one for identity, one for higher half
	# The 768th page table is what begins at 0xC0000000
	movl $(boot_page_table0 - 0xC0000000 + 0x003), (boot_page_directory - 0xC0000000 + 0)
	movl $(boot_page_table0 - 0xC0000000 + 0x003), (boot_page_directory - 0xC0000000 + (768 * 4))

recursive_map:
	# Set the last entry in the page directory to itself
	xchgw %bx, %bx
	movl $(boot_page_directory - 0xC0000000 + 0x003), (boot_page_directory - 0xC0000000 + (1023 * 4))

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
	lea higher_half, %ecx
	jmp *%ecx

higher_half:
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
4:	hlt
	jmp 4b



# -----------------
# Helper functions
# -----------------

	
.size _start, . - _start
