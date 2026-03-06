#include <config.h>
OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
	. = BOOT0ADDR;
	. = ALIGN(4);

	.head   :
	{
		main/boot0_head.o	(.rodata)
	}
	. = ALIGN(1);

#ifndef CFG_SUNXI_SIMPLEBOOT
	.e907_text :
	{
		KEEP(CPUDIR/e907_boot.o (.data))
	}
#endif

	.text :
	{
		CPUDIR/boot0_entry.o (.text)
		SOLUTIONDIR/libsolution.o (.text)
		*(.text)
	}
	. = ALIGN(16);
  	.rodata : { *(.rodata) }
	. = ALIGN(16);
  	.data : { *(.data) }
	. = ALIGN(16);
	.sdata : { *(.sdata) }

	. = ALIGN(4);
	.bss :
	{
	__bss_start = .;
		*(.sbss)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
	}
	. = ALIGN(4);
	__bss_end = .;

	_end = .;
	ASSERT(. <= (BOOT0ADDR + BOOT0SIZE), "boot0 image has exceeded its limit.")
}



