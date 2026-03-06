/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the people's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#include <common.h>
#include <linux/types.h>
#include <linux/elf.h>

#ifdef CFG_ELF_64BIT
#define Elf_Ehdr				Elf64_Ehdr
#define Elf_Phdr				Elf64_Phdr
#define Elf_Shdr				Elf64_Shdr
#else
#define Elf_Ehdr				Elf32_Ehdr
#define Elf_Phdr				Elf32_Phdr
#define Elf_Shdr				Elf32_Shdr
#endif

/* copy from u-boot-2018/drivers/riscv/common/riscv_img.h */
struct vaddr_range_t {
	unsigned long vstart;
	unsigned long vend;
	unsigned long pstart;
};

/*
 * riscv need to remap addresses for some addr.
 */
static struct vaddr_range_t addr_mapping[] = {
#ifdef CFG_RISCV_E906
	{ 0x3FFC0000, 0x4003FFFF, 0x07280000 },
	{ 0x40400000, 0x7FFFFFFF, 0x40400000},
#endif
/*others (c906/e907) has the same addresses mapping as a7 */
};

static unsigned long set_img_va_to_pa(unsigned long vaddr,
				struct vaddr_range_t *map, int size)
{
	unsigned long paddr = vaddr;
	int i;

	for (i = 0; i < size; i++) {
		if (vaddr >= map[i].vstart
				&& vaddr <= map[i].vend) {
			paddr = vaddr - map[i].vstart + map[i].pstart;
			break;
		}
	}

	return paddr;
}

phys_addr_t elf_get_entry_addr(phys_addr_t base)
{
	Elf_Ehdr *ehdr;

	ehdr = (Elf_Ehdr *)base;

	return ehdr->e_entry;
}

int load_elf_image(phys_addr_t img_addr)
{
	int i;
	Elf_Ehdr *ehdr;
	Elf_Phdr *phdr;

	void *dst = NULL;
	void *src = NULL;

	int size = sizeof(addr_mapping) / sizeof(struct vaddr_range_t);

	ehdr = (Elf_Ehdr *)img_addr;
	phdr = (Elf_Phdr *)((phys_addr_t)(img_addr + ehdr->e_phoff));

	/* load elf program segment */
	for (i = 0; i < ehdr->e_phnum; ++i, ++phdr) {
		//remap addresses
		dst = (void *)(phys_addr_t)set_img_va_to_pa((unsigned long)phdr->p_paddr, \
					addr_mapping, size);

		src = (void *)((phys_addr_t)(img_addr + phdr->p_offset));

		if (phdr->p_type != PT_LOAD)
			continue;

		if ((phdr->p_memsz == 0) || (phdr->p_filesz == 0))
			continue;

		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);

		if (phdr->p_filesz != phdr->p_memsz)
			memset((u8 *)dst + phdr->p_filesz, 0x00,
						phdr->p_memsz - phdr->p_filesz);
	}

	return 0;
}

static Elf_Shdr *elf_find_segment(phys_addr_t elf_addr, const char *seg_name)
{
	int i;
	Elf_Shdr *shdr;
	Elf_Ehdr *ehdr;
	const char *name_table;
	const uint8_t *elf_data = (void *)elf_addr;

	ehdr = (Elf_Ehdr *)elf_data;
	shdr = (Elf_Shdr *)(elf_data + ehdr->e_shoff);
	name_table = (const char *)(elf_data + shdr[ehdr->e_shstrndx].sh_offset);

	for (i = 0; i < ehdr->e_shnum; i++, shdr++) {
		if (strcmp(name_table + shdr->sh_name, seg_name))
			continue;

		return shdr;
	}

	return NULL;
}

void *elf_find_segment_offset(phys_addr_t elf_addr, const char *seg_name)
{
	Elf_Shdr *shdr;

	shdr = elf_find_segment(elf_addr, seg_name);
	if (!shdr)
		return NULL;

	return (void *)((phys_addr_t)(elf_addr + shdr->sh_offset));
}

void *elf_find_segment_addr(phys_addr_t elf_addr, const char *seg_name)
{
	Elf_Shdr *shdr;

	shdr = elf_find_segment(elf_addr, seg_name);
	if (!shdr)
		return NULL;

	return (void *)((phys_addr_t)shdr->sh_addr);
}

#ifdef CFG_SUNXI_FDT
void boot_riscv(phys_addr_t base, unsigned long fdt_addr)
{
	if (base) {
#if defined(CFG_RISCV_E907)
		void boot_e907(phys_addr_t base, unsigned long fdt_addr);
		boot_e907(base, fdt_addr);
#elif defined(CFG_RISCV_E906)
		void boot_e906(phys_addr_t base, unsigned long fdt_addr);
		boot_e906(base, fdt_addr);
#elif defined(CFG_RISCV_C906)
		void boot_c906(phys_addr_t base, unsigned long fdt_addr);
		boot_c906(base, fdt_addr);
#else
#error "Unsupport this riscv!"
#endif
	} else {
		pr_emerg("not get riscv base addr!\n");
	}
}
#endif
