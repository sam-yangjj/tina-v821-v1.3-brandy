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
#include <libfdt.h>
#include <common.h>
#include <private_uboot.h>
#include <boot_param.h>
#include <linux/elf.h>

#define FDT_DEBUG 0

#if FDT_DEBUG
#define FDT_DBG(fmt, arg...) printf("%s()%d - " fmt, __func__, __LINE__, ##arg)
#define FDT_ERR(fmt, arg...) printf("%s()%d - " fmt, __func__, __LINE__, ##arg)
#else
#define FDT_DBG(fmt, arg...)
#define FDT_ERR(fmt, arg...) printf("%s()%d - " fmt, __func__, __LINE__, ##arg)
#endif

#ifdef CFG_SUNXI_SDMMC
extern void mmc_update_config_for_sdly(void);
#endif
#ifndef CFG_SUNXI_NO_UPDATE_FDT_CHOSEN
#define ROOTFS_NAME	   "rootfs@"
#define ROOTFS_BACKUP_NAME "rootfs_backup@"
#define BLOCK_ROOTFS_NUM   "root=/dev/"


/* During the first mass production,
 * this area of ​​the nor scheme is erased to 1,
 * and the mmc scheme is erased to 0
 * */
#ifdef CFG_SUNXI_SPINOR
#define USE_BACKUP_BIT_IS_ZERO   0x00
#elif CFG_SUNXI_SDMMC
#define USE_BACKUP_BIT_IS_ZERO   0xFF
#else
#define USE_BACKUP_BIT_IS_ZERO   0x00
#endif


//#define USE_BACKUP_IS_ZERO   0
#define KERNEL_AND_ROOTFS_MASK 0x01

#define ROOTFS_SWITCH_MASK 0x02
#define KERNEL_SWITCH_MASK 0x01

/* switch_flag default -- 0xff,
 * value  : 1 -- system  0 -- system_backup
 * bit 1~7: reserve
 * bit 0  : rootfs and kernel
 */

char get_switch_flag(void)
{
#ifdef CFG_BOOT0_LOAD_ISPPARM
	char switch_flag = read_switch_flag_from_kernel();
#else
	char switch_flag = 0xff;
#endif
	switch_flag = (switch_flag && KERNEL_AND_ROOTFS_MASK)^USE_BACKUP_BIT_IS_ZERO;
	FDT_DBG("switch_flag = 0x%x\n", switch_flag);
	return switch_flag;
}

/* switch_flag default -- 0xff,
 * value  : 1 -- system  0 -- system_backup
 * bit 3~7: reserve
 * bit 2  ：extend partition，/usr directory,boot0 not use,system use
 * bit 1  : rootfs: 1:system  0:system_backup
 * bit 0  : kernel, 1:system  0:system_backup
 */

char get_switch_kernel_flag(void)
{
#ifdef CFG_BOOT0_LOAD_ISPPARM
	char switch_flag = read_switch_flag_from_kernel();
#else
	char switch_flag = 0xff;
#endif
	switch_flag = (switch_flag & KERNEL_SWITCH_MASK)^USE_BACKUP_BIT_IS_ZERO;
	printf("switch_kernel_flag = 0x%x\n", switch_flag);
	return switch_flag;
}

/* switch_flag default -- 0xff,
 * value  : 1 -- system  0 -- system_backup
 * bit 3~7: reserve
 * bit 2  ：extend partition，/usr directory,boot0 not use,system use
 * bit 1  : rootfs: 1:system  0:system_backup
 * bit 0  : kernel, 1:system  0:system_backup
 */



static void sunxi_switch_rootfs(char *rootfs_name)
{
	u32 bootargs_node;
	int len = 0;
	int err;
	char *bootargs_str     = NULL;
	char cmdline[2048]      = { 0 };
	char temp_blocknum[32] = { 0 };
	bootargs_node	       = fdt_path_offset(working_fdt, "/chosen");
	bootargs_str	       = (void *)fdt_getprop(working_fdt, bootargs_node,
						     "bootargs", &len);
	if (!bootargs_str) {
		FDT_ERR("bootargs_str is NULL!!!\n");
	}
	char *rootfs_start = strstr(bootargs_str, rootfs_name);
	if (!rootfs_start) {
		FDT_ERR("cont find rootfs partition!!!\n");
	}
	/* The last partition of the sunxi platform is udisk */
	char *rootfs_end = strchr(rootfs_start, ':');
	if (!rootfs_end) {
		FDT_DBG("error:cant find rootfs end\n");
	}
	char *spacer_symbol = strchr(rootfs_start, '@');
	memcpy(temp_blocknum, spacer_symbol + 1,
	       rootfs_end - spacer_symbol -1);
	FDT_DBG("temp_blocknum : %s\n", temp_blocknum);

	char *blck_root_start = strstr(bootargs_str, BLOCK_ROOTFS_NUM);
	if (!blck_root_start) {
		FDT_ERR("cont find rootfs partition!!!\n");
	}
	char *blck_root_end = strchr(blck_root_start, ' ');
	if (!blck_root_end) {
		blck_root_end = bootargs_str + strlen(bootargs_str) - 1;
		FDT_DBG("error:cant find rootfs end\n");
	}
	strncat(cmdline, bootargs_str, blck_root_start - bootargs_str);
	strncat(cmdline, BLOCK_ROOTFS_NUM, sizeof(BLOCK_ROOTFS_NUM));
	strncat(cmdline, temp_blocknum, sizeof(temp_blocknum));
	strncat(cmdline, blck_root_end,
		bootargs_str + strlen(bootargs_str) - blck_root_end);

	FDT_DBG("cmdline:%s\nstrlen(cmdline = %d)\n", cmdline, strlen(cmdline));
	err = fdt_setprop(working_fdt, bootargs_node, "bootargs", cmdline,
			  strlen(cmdline) + 1);
	if (err < 0) {
		printf("libfdt fdt_setprop(): %s\n", fdt_strerror(err));
		return;
	}
	return;
}

#ifdef CFG_SUNXI_ENV
extern char rootfs_name[32];

static void sunxi_update_chosen(void)
{
	sunxi_switch_rootfs(rootfs_name);
}

#else

char get_switch_rootfs_flag(void)
{
#ifdef CFG_BOOT0_LOAD_ISPPARM
	char switch_flag = read_switch_flag_from_kernel();
#else
	char switch_flag = 0xff;
#endif
	switch_flag = (switch_flag & ROOTFS_SWITCH_MASK)^USE_BACKUP_BIT_IS_ZERO;
	printf("switch_rootfs_flag = 0x%x\n", switch_flag);
	return switch_flag;
}
static void sunxi_update_chosen(void)
{
	if (get_switch_rootfs_flag()) {
		sunxi_switch_rootfs(ROOTFS_NAME);
	} else {
		sunxi_switch_rootfs(ROOTFS_BACKUP_NAME);
	}
}
#endif //CFG_SUNXI_ENV
#endif //CFG_SUNXI_NO_UPDATE_FDT_CHOSEN

__weak void mmc_update_config_for_sdly(void)
{
}

#ifdef CFG_SUNXI_BOOT_PARAM
extern int sprintf(char *buf, const char *fmt, ...);
int update_fdt_dram_para_from_bootpara(void *dtb_base)
{
	/*fix dram para*/
	int i, nodeoffset = 0;
	char dram_str[16]   = { 0 };
	uint32_t *dram_para = NULL;
	typedef_sunxi_boot_param *sunxi_boot_param = sunxi_bootparam_get_buf();

	nodeoffset = fdt_path_offset(dtb_base, "/dram");
	if (nodeoffset < 0) {
		FDT_ERR("## error: %s : %s\n", __func__,
		       fdt_strerror(nodeoffset));
		return -1;
	}

	dram_para = (uint32_t *)sunxi_boot_param->ddr_info;
	memset(dram_str, 0, sizeof(dram_str));
	for (i = MAX_DRAMPARA_SIZE - 1; i >= 0; i--) {
		sprintf(dram_str, "dram_para[%02d]", i);
		fdt_setprop_u32(dtb_base, nodeoffset, dram_str, dram_para[i]);
	}
	return 0;
}
#endif

int sunxi_update_fdt_para_for_kernel(void)
{
#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	int err	    = fdt_check_header(working_fdt);
	if (err < 0) {
		printf("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}
#ifndef CFG_SUNXI_NO_UPDATE_FDT_CHOSEN
	sunxi_update_chosen();
#endif

#ifdef CFG_SUNXI_BOOT_PARAM
	update_fdt_dram_para_from_bootpara(working_fdt);
#endif

#ifdef CFG_SUNXI_SDMMC
	mmc_update_config_for_sdly();
#endif
#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif
	printf("update dts\n");
	return 0;
}

int fdt_enable_node(char *name, int onoff)
{
	int nodeoffset = 0;
	int ret = 0;

	nodeoffset = fdt_path_offset(working_fdt, name);
	ret = fdt_set_node_status(working_fdt, nodeoffset,
		onoff ? FDT_STATUS_OKAY : FDT_STATUS_DISABLED, 0);

	if (ret < 0) {
		printf("disable nand error: %s\n", fdt_strerror(ret));
	}
	return ret;
}

#define CHECK_ELF_MAGIC(ehdr) ((ehdr)->e_ident[EI_MAG0] == ELFMAG0 && \
		      (ehdr)->e_ident[EI_MAG1] == ELFMAG1 && \
		      (ehdr)->e_ident[EI_MAG2] == ELFMAG2 && \
		      (ehdr)->e_ident[EI_MAG3] == ELFMAG3)

#ifdef CFG_VERIFY_RTOS_DIGEST_CRC32
extern uint32_t crc32(uint32_t crc, const uint8_t *buf, uint len);
static Elf32_Shdr *find_section(unsigned long img_addr, const char *seg)
{
	Elf32_Ehdr *ehdr = NULL;
	Elf32_Shdr *shdr = NULL;
	int i = 0;
	char *strtab = NULL;

	ehdr = (Elf32_Ehdr *)img_addr;
	shdr =  (Elf32_Shdr *)(img_addr + ehdr->e_shoff);

	strtab = (char *)img_addr + shdr[ehdr->e_shstrndx].sh_offset;

	shdr = NULL;
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *)(img_addr + ehdr->e_shoff
				+ (i * sizeof(Elf32_Shdr)));

		char *pstr = strtab + shdr->sh_name;

		if (!(shdr->sh_flags & SHF_ALLOC)
			|| (shdr->sh_addr == 0)
			|| (shdr->sh_size == 0)) {
			continue;
		}

		if (strcmp(pstr, seg) == 0)
			break;
	}

	return shdr;
}

#endif

int check_rtos_elf(u32 addr)
{
	/*check elf header*/
	if (!CHECK_ELF_MAGIC((Elf32_Ehdr *) addr)) {
		printf("invalid image header.\n");
		return -1;
	}

#ifdef CFG_VERIFY_RTOS_DIGEST_CRC32
	Elf32_Shdr *shdr;
	uint8_t src_digest[16];
	uint32_t img_size;

	/* format: 4bytes image size + 16bytes digest */
	shdr = find_section(addr, ".digest");
	if (!shdr) {
		/*rtos img not add digest, not check*/
		printf("Can't find .digest section from rtos img\r\n");
		return 1;
	}
	printf("find .digest section from rotfs img\r\n");

	img_size = *(uint32_t *)(addr + shdr->sh_offset);
	printf("the img_size is %u\n", img_size);
	if (img_size > 0) {
		memcpy(src_digest, (void *)(addr + shdr->sh_offset + 4), 16); //crc32 only use 4 byte
		memset((void *)(addr + shdr->sh_offset), 0, 20); //memset 0 to cal crc32

		uint32_t cal_crc32 = 0;
		cal_crc32 = crc32(0, (unsigned char *)addr, img_size);
		printf("[crc32]the cal crc32 is: 0x%08x\n", cal_crc32);
		printf("[crc32]the src crc32 is: 0x%08x\n", (*(uint32_t *)src_digest));
		/* restore digest data */
		memcpy((void *)(addr + shdr->sh_offset), &img_size, 4);
		memcpy((void *)(addr + shdr->sh_offset + 4), src_digest, 16);

		if (memcmp(src_digest, &cal_crc32, 4)) {
			printf("rtos img data damaged\r\n");
			return -1;
		}
	} else {
		printf("rtos img data damaged\r\n");
		return -1;
	}
#endif
	return 0;
}
extern void boot_riscv(phys_addr_t base, unsigned long fdt_addr);
#if defined(CFG_RISCV_E907) || defined(CFG_RISCV_E906) || defined(CFG_RISCV_C906) \
	|| defined(CFG_BOOT0_LOAD_KERNEL)
int load_and_run_fastboot(u32 uboot_base, u32 optee_base, u32 monitor_base,
		u32 rtos_base, u32 opensbi_base, u32 cpus_rtos_base)
{
#ifdef CFG_EARLY_FASTBOOT
	if (early_fastboot_init())
		return -1;
#endif

#ifndef CFG_SUNXI_PMBOOT
#ifndef CFG_EARLY_BOOT_RISCV
#ifdef CFG_DEFAULT_BOOT_RTOS
	/* if uboot is card product, check if boot rtos */
	if (uboot_base) {
		uboot_head_t *header = (uboot_head_t *)uboot_base;
		if (header->boot_data.work_mode == WORK_MODE_CARD_PRODUCT) {
			if (cpus_rtos_base)
				goto bootrv;
			else
				return 0;
		}
	}
	/*Load rtos partition to prepare for startup*/
	extern int load_rtos_partition(char *name, phys_addr_t load_base);
	if (load_rtos_partition("riscv0", CONFIG_RTOS_LOAD_ADDR) != 0) {
		printf("load rtos partition fail, skip run fastboot.\n");
		return -1;
	}
	cpus_rtos_base = CONFIG_RTOS_LOAD_ADDR;
	record_time("load_rtos_partition");
bootrv:
#endif
	if (check_rtos_elf(cpus_rtos_base) >= 0) {
		boot_riscv(cpus_rtos_base, (unsigned long)working_fdt);
		record_time("boot_riscv");
	} else {
		printf("rots elf is bad, not boot rtos\n");
	}
#endif
#endif

#if CFG_BOOT0_LOAD_KERNEL
	exist_uboot_jmp_cardproduct(uboot_base);
	load_and_run_kernel(optee_base, opensbi_base, monitor_base);
	//already jump to kernel, should never return, if so, go FEL
	return 1;
#endif

	return 0;
}
#endif

void sunxi_dump(void *addr, unsigned int size)
{
	int i, j;
	char *buf = (char *)addr;

	for (j = 0; j < size; j += 16) {
		for (i = 0; (i < 16 && i < size); i++) {
			printf("%02x ", buf[j + i] & 0xff);
		}
		printf("\n");
	}
	printf("\n");

}
