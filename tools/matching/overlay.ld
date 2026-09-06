OUTPUT_ARCH(mips)

SECTIONS
{
	.overlay __overlay_load_address :
	{
		LONG(__overlay_id)
		*(.rodata)
		. = ALIGN(4);
		*(.text)
		. = ALIGN(4);
		*(.data)
		*(.sdata)
	}

	/DISCARD/ :
	{
		*(.bss)
		*(.reginfo)
		*(.MIPS.abiflags)
		*(.pdr)
		*(.gnu.attributes)
		*(.comment)
	}
}
