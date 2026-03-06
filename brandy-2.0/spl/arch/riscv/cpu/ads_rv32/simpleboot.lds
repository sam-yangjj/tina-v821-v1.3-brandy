#include <config.h>
OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
	. = SIMPLEBOOTADDR;
	. = ALIGN(4);

	.head   :
	{
		main/boot0_head.o	(.rodata)
	}
	. = ALIGN(1);

	.e907_text :
	{
		KEEP(CPUDIR/e907_boot.o (.data))
	}

	.text :
	{
		CPUDIR/simple_entry.o (.text*)
		*(.text*)
	}
	. = ALIGN(16);
	.rodata : { *(.rodata*) }
	. = ALIGN(16);
	.data : { *(.data*) }
	. = ALIGN(16);
	.sdata : { *(.sdata*) }

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
	ASSERT(. <= (SIMPLEBOOTADDR + SIMPLEBOOTSIZE), "boot0 image has exceeded its limit.")

#ifdef CFG_AMP_BOOT0_BIN_PATH
	ASSERT((SIMPLEBOOTADDR + SIMPLEBOOTSIZE) <= CFG_AMP_BOOT0_RUN_ADDR, "amp_boot0 has overhap.")
	. = CFG_AMP_BOOT0_RUN_ADDR;
	.amp_boot0 :
	{
		KEEP(*(.amp_boot0*))
	}
#endif
}
