ENTRY (_start)

SECTIONS
{
	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */
	. = 0xC0100000;
	/* Add a symbol that indicates the start address of the kernel. */
	_kernel_start = .;
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	__initcall0_ptrs_start = .;
	.initcall0.init : AT (ADDR (.initcall0.init) - 0xC0000000)
	{
		*(.initcall0.init)
	}
	__initcall0_ptrs_end = .;
	__initcall1_ptrs_start = .;
	.initcall1.init : AT (ADDR (.initcall1.init) - 0xC0000000)
	{
		*(.initcall0.init)
	}
	__initcall1_ptrs_end = .;
	__initcall2_ptrs_start = .;
	.initcall2.init : AT (ADDR (.initcall2.init) - 0xC0000000)
	{
		*(.initcall0.init)
	}
	__initcall2_ptrs_end = .;
	__initcall3_ptrs_start = .;
	.initcall3.init : AT (ADDR (.initcall3.init) - 0xC0000000)
	{
		*(.initcall0.init)
	}
	__initcall3_ptrs_end = .;
	__initcall4_ptrs_start = .;
	.initcall4.init : AT (ADDR (.initcall4.init) - 0xC0000000)
	{
		*(.initcall0.init)
	}
	__initcall4_ptrs_end = .;
	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
	/* Add a symbol that indicates the end address of the kernel. */
	_kernel_end = .;
}