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
#include <config.h>
#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <board_helper.h>
#include <android_image.h>

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
#include <arch/rtc.h>
#endif

#ifdef CFG_SUNXI_GPT
#include <part_efi.h>
#endif

#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
#include <libfdt.h>
#endif
#endif

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
#define RV_PARTITION_A    "riscv0"
#define RV_PARTITION_B    "riscv0-r"
#endif

#if 0
#define kernel_dbg(fmt, arg...) printf(fmt, ##arg)
#else
#define kernel_dbg(fmt, arg...)
#endif

#define ALIGN(x, a) __ALIGN_MASK((x), (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

#ifndef CFG_SUNXI_ENV
/*
 * CFG_BOOT0_LOAD_KERNEL=y
 * CFG_KERNEL_BOOTIMAGE=y
 * CFG_KERNEL_CHECKSUM=n #y will check kernel checksum in bimage, but slower
 * CFG_SPINOR_KERNEL_OFFSET=0x20 #first partition, 0x20
 * CFG_SPINOR_LOGICAL_OFFSET=2016
 * CFG_KERNEL_LOAD_ADDR=0x41008000
 * CFG_SUNXI_FDT_ADDR=0x41800000
 */

#ifdef CFG_SUNXI_SPINOR
#if !defined(CFG_SPINOR_LOGICAL_OFFSET) ||                                     \
	!defined(CFG_SPINOR_KERNEL_OFFSET) ||                                  \
	!defined(CFG_KERNEL_LOAD_ADDR) || !defined(CFG_SUNXI_FDT_ADDR)
#error CFG_KERNEL_LOAD_ADDR CFG_SPINOR_LOGICAL_OFFSET CFG_SPINOR_KERNEL_OFFSET CFG_SUNXI_FDT_ADDR \
required for boot kernel function !

#endif
#define LOGICAL_OFFSET    CFG_SPINOR_LOGICAL_OFFSET
#define KERNEL_OFFSET     CFG_SPINOR_KERNEL_OFFSET
#endif /* CFG_SUNXI_SPINOR */

#ifdef CFG_SUNXI_SDMMC
#if !defined(CFG_MMC_KERNEL_OFFSET) ||                                     \
	!defined(CFG_MMC_LOGICAL_OFFSET) ||                                  \
	!defined(CFG_KERNEL_LOAD_ADDR) || !defined(CFG_SUNXI_FDT_ADDR)
#error CFG_KERNEL_LOAD_ADDR CFG_MMC_KERNEL_OFFSET CFG_MMC_LOGICAL_OFFSET CFG_SUNXI_FDT_ADDR \
required for boot kernel function !
#endif

#define LOGICAL_OFFSET    CFG_MMC_LOGICAL_OFFSET
#define KERNEL_OFFSET     CFG_MMC_KERNEL_OFFSET

int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);
int get_card_num(void);
#endif /* CFG_SUNXI_SDMMC */

#ifdef CFG_SUNXI_SPINAND

extern int spinand_get_boot_start(int copy);
#define LOGICAL_OFFSET    0
#define KERNEL_OFFSET     spinand_get_boot_start(0)
#define KERNEL_BACKUP_OFFSET     spinand_get_boot_start(1)

#endif /* CFG_SUNXI_SPINAND */

#if !defined(CFG_SUNXI_SPINOR) &&                                     \
	!defined(CFG_SUNXI_SDMMC) && \
	!defined(CFG_SUNXI_SPINAND)
#error Only supports mmc and nor and spinand
#endif

/* Failed to start or read the flag
in the flash to enter the system backup*/
int load_kernel_from_flash(u32 *kernel_addr,
			   int (*flash_read)(uint, uint, void *))
{
	int ret = 0;
#ifdef CFG_KERNEL_BOOTIMAGE
	u32 kernel_start = KERNEL_OFFSET + LOGICAL_OFFSET;
#ifdef CFG_SPINOR_KERNEL_BACKUP_OFFSET
	u32 kernel_backup =
			CFG_SPINOR_KERNEL_BACKUP_OFFSET + LOGICAL_OFFSET;
	if (!get_switch_kernel_flag()) {
		printf("use kernel backup\n");
		ret = load_bimage(kernel_backup, CFG_KERNEL_LOAD_ADDR,
				  flash_read, kernel_addr);
		if (ret < 0) {
			printf("bootimage backup0 erro\n");
		}
	} else {
#endif /*CFG_SPINOR_KERNEL_BACKUP_OFFSET*/
		ret = load_bimage(kernel_start, CFG_KERNEL_LOAD_ADDR,
				  flash_read, kernel_addr);
		if (ret < 0) {
			printf("load bootimage erro\n");
//CFG_SPINOR_KERNEL_BACKUP_OFFSET=0x37A0
#ifdef CFG_SPINOR_KERNEL_BACKUP_OFFSET
			ret = load_bimage(kernel_backup, CFG_KERNEL_LOAD_ADDR,
					  flash_read, kernel_addr);
			if (ret < 0) {
				printf("bootimage backup0 erro\n");
			}

		}
#endif /*CFG_SPINOR_KERNEL_BACKUP_OFFSET*/

#ifdef CFG_SUNXI_SPINAND
		u32 kernel_backup = KERNEL_BACKUP_OFFSET;
		ret = load_bimage(kernel_backup, CFG_KERNEL_LOAD_ADDR,
				  flash_read, kernel_addr);
		if (ret < 0) {
			printf("bootimage backup0 erro\n");
		}
#endif /* CFG_SUNXI_SPINAND */
	}
#endif

//CFG_KERNEL_UIMAGE=y
#ifdef CFG_KERNEL_UIMAGE
#define UIMAGE_SIZE_KB (2 * 1024)
	ret = load_uimage(KERNEL_OFFSET + LOGICAL_OFFSET,
			  CFG_KERNEL_LOAD_ADDR, UIMAGE_SIZE_KB, spinor_read);
	if (ret < 0) {
		printf("load uimage erro\n");
	}
#endif

#ifdef CFG_SUNXI_SPINAND
extern __s32 SpiNand_PhyExit(void);
	SpiNand_PhyExit();
#endif

	return ret;
}
#else
/*
 * CFG_BOOT0_LOAD_KERNEL=y
 * CFG_KERNEL_BOOTIMAGE=y
 * CFG_KERNEL_CHECKSUM=n #y will check kernel checksum in bimage, but slower
 * CFG_SUNXI_GPT=y
 * CFG_MMC_GPT_ARD=0 # 0 or 40960 sector
 * CFG_SUNXI_ENV=y
 * CFG_SUNXI_ENV_SIZE=0x20000 #linux should be the same with LICHEE_REDUNDANT_ENV_SIZE
 * CFG_SUNXI_HAVE_REDUNDENV=y #if have env and env-redund in partition
 * CFG_KERNEL_LOAD_ADDR=0x41008000
 * CFG_SUNXI_FDT_ADDR=0x41800000
 */

#ifdef CFG_SUNXI_SDMMC
int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);
int get_card_num(void);
#endif /* CFG_SUNXI_SDMMC */

int gpt_env_init(void)
{
	int ret = -1;

	if (init_gpt()) {
		pr_emerg("init gpt fail\n");
		return ret;
	}

	if (fw_env_open() == -1) {
		pr_emerg("open env error\n");
		return ret;
	}

	return 0;
}

#ifndef CFG_SUNXI_NO_UPDATE_FDT_CHOSEN
char rootfs_name[32];
#endif

int load_kernel_from_flash(u32 *kernel_addr,
			   int (*flash_read)(uint, uint, void *))
{
	int ret = -1;
	uint kernel_start = 0;
	char *kernel_name = NULL;

#ifndef CFG_EARLY_FASTBOOT
	ret = gpt_env_init();
	if (ret) {
		goto load_out;
	}
	record_time("env_init");
#endif

#ifndef CFG_SUNXI_NO_UPDATE_FDT_CHOSEN
	//must get rootfs name from env here, because load kernel will destroy heap:CONFIG_HEAP_BASE
	char *tmp_name = fw_getenv("root_partition");
	memcpy(rootfs_name, tmp_name, strlen(tmp_name));
#endif

	kernel_name = fw_getenv("boot_partition");
	if (kernel_name == NULL) {
		pr_emerg("no boot_partition in env\n");
		goto load_out;
	}

	if (get_part_info_by_name(kernel_name, &kernel_start, NULL)) {
		goto load_out;
	}
	record_time("get_boot_partition");

	//must close env first, because load kernel will destroy heap:CONFIG_HEAP_BASE
	fw_env_close();
#ifdef CFG_SUNXI_LOGIC_OFFSET
	ret = load_bimage(kernel_start + CFG_SUNXI_LOGIC_OFFSET, CFG_KERNEL_LOAD_ADDR, flash_read, kernel_addr);
#else
	ret = load_bimage(kernel_start, CFG_KERNEL_LOAD_ADDR, flash_read, kernel_addr);
#endif
	if (ret < 0) {
		pr_emerg("load kernel erro\n");
	}

load_out:
	return ret;
}
#endif //CFG_SUNXI_ENV


void load_and_run_kernel(u32 optee_base, u32 opensbi_base, u32 monitor_base)
{
	int load_flash_success = -1;
	u32 kernel_addr;
	int (*flash_read)(uint, uint, void *);
#ifdef CFG_SUNXI_SDMMC
	flash_read = mmc_bread_ext;
#elif CFG_SUNXI_SPINOR
	flash_read = spinor_read;
#elif CFG_SUNXI_SPINAND
extern int spinand_read_kernel(uint sector_num, uint N, void *buffer);
	flash_read = spinand_read_kernel;
#else
	return 0;
#endif

	load_flash_success = load_kernel_from_flash(&kernel_addr, flash_read);
	if (load_flash_success)
		return;
	record_time("load_kernel");

#ifdef CFG_SUNXI_FDT
#ifndef CFG_SUNXI_NO_UPDATE_FDT_PARA
	sunxi_update_fdt_para_for_kernel();
	record_time("update_fdt_para");
#endif
#endif
	mmu_disable();

#ifdef CFG_SUNXI_SDMMC
	sunxi_mmc_exit(get_card_num(), BT0_head.prvt_head.storage_gpio, 16);
#endif
	record_time("jump_opensbi");
	dump_record_time();

#ifdef CFG_ARCH_RISCV
	if (opensbi_base) {
		printf("Jump to OpenSBI: opensbi_base = 0x%x, dtb_base = 0x%x, kernel_addr = 0x%x\n",
			opensbi_base, CFG_SUNXI_FDT_ADDR, kernel_addr);
		boot0_jmp_opensbi_v1x(0, CFG_SUNXI_FDT_ADDR, opensbi_base, kernel_addr);
	} else {
		pr_emerg("opensbi_base is 0, Failed to jump to opensbi\n");
		asm volatile("WFI");
	}
#else
	if (monitor_base) {
		struct spare_monitor_head *monitor_head =
			(struct spare_monitor_head *)((phys_addr_t)monitor_base);
		monitor_head->secureos_base = optee_base;
		monitor_head->nos_base = kernel_addr; //kernel_base
		boot0_jmp_monitor(monitor_base);
	} else if (optee_base) {
		optee_jump_kernel(optee_base, CFG_SUNXI_FDT_ADDR,
				  kernel_addr);
	} else {
		boot0_jump_kernel(CFG_SUNXI_FDT_ADDR, kernel_addr);
	}
#endif
}

#ifdef CFG_EARLY_FASTBOOT
#define SUNXI_FLASH_READ_FIRST_SIZE (32 * 1024)
extern int android_image_get_dtb(const struct andr_img_hdr *hdr, ulong *dtb_data, ulong *dtb_len);

int early_fastboot_load_dtb(void)
{
	int ret = -1;
	uint kernel_start = 0;
	char *kernel_name = NULL;
	void *addr = (void *)CFG_KERNEL_LOAD_ADDR;
	struct andr_img_hdr *fb_hdr;
	int (*flash_read)(uint, uint, void *);

	flash_read = spl_flash_read;

	kernel_name = fw_getenv("boot_partition");
	if (kernel_name == NULL) {
		pr_emerg("no boot_partition in env\n");
		return ret;
	}

	if (get_part_info_by_name(kernel_name, &kernel_start, NULL)) {
		return ret;
	}

	if (flash_read(kernel_start, (SUNXI_FLASH_READ_FIRST_SIZE / 512), addr)) {
		pr_emerg("Fail to load kernel head\n");
		return ret;
	}

	fb_hdr = (struct andr_img_hdr *)addr;

#ifdef CFG_LOAD_DTB_FROM_KERNEL
	kernel_dbg("dtb_size = 0x%x\n", fb_hdr->dtb_size);
	kernel_dbg("dtb_addr = 0x%x\n", fb_hdr->dtb_addr);
	working_fdt = (struct fdt_header *)CFG_SUNXI_FDT_ADDR;
	if (fb_hdr->dtb_size) {
		ulong dtb_data = 0, dtb_len = 0;
		android_image_get_dtb(fb_hdr, &dtb_data, &dtb_len);
		ret = flash_read(kernel_start + (dtb_data - (unsigned long)fb_hdr) / 512,
				ALIGN(fb_hdr->dtb_size, 512) / 512, (void *)CFG_SUNXI_FDT_ADDR);
#ifdef SCP_DTS_BASE
		memcpy((void *)SCP_DTS_BASE, (void *)CFG_SUNXI_FDT_ADDR, fb_hdr->dtb_size);
#endif // SCP_DTS_BASE
		if (ret) {
			pr_emerg("load dtb fail\n");
			return ret;
		}
#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
		u32 fdt_ext_size = ALIGN(CFG_RESERVE_FDT_SIZE, 32);
		u32 new_fdt_totalsize = fdt_totalsize(working_fdt);
		if (new_fdt_totalsize > fdt_ext_size) {
			printf("new fdt(0x%08x) biger than now fdt(0x%08x)\n", new_fdt_totalsize, fdt_ext_size);
		} else {
			fdt_set_totalsize(working_fdt, fdt_ext_size);
		}
#endif
#endif
	}
#endif // CFG_LOAD_DTB_FROM_KERNEL

	return 0;
}

#ifdef CFG_EARLY_LOAD_MODEL

#define MAX_MODEL_COUNT 10
#define MAX_MODEL_NAME_LENGTH 16

typedef struct ModelInfo {
	int model_count;
	char model_name[MAX_MODEL_COUNT][MAX_MODEL_NAME_LENGTH];
	int model_size[MAX_MODEL_COUNT];
	uint32_t model_start_address[MAX_MODEL_COUNT];
} ModelInfo;

int early_fastboot_load_model(void)
{
	int (*flash_read)(uint, uint, void *);
	char *modelparts;
	uint load_start_sector = 0;
	uint load_sectors = 0;
	int ret = 0;

	char loadname[16];
	int i = 0;

	uint model_start_sector[MAX_MODEL_COUNT] = {0};
	uint model_load_sectors[MAX_MODEL_COUNT] = {0};
	uint size_diff = 0;

	flash_read = spl_flash_read;

	ModelInfo *modelInfo_head = (ModelInfo *)malloc(sizeof(ModelInfo));
	memset(modelInfo_head, 0, sizeof(ModelInfo));

	modelparts = fw_getenv("model_partition");
	if (modelparts == NULL) {
		pr_emerg("no model_partition in env!\n");
		goto load_part_out;
	}

	printf("modelparts:%s\n", modelparts);

	while ((modelparts != NULL) && (*modelparts != 0)) {
		i = 0;
		memset(loadname, 0, 16);
		while ((modelparts != NULL) && (*modelparts != '@') && (*modelparts != 0))
			loadname[i++] = *modelparts++;

		if ((modelparts != NULL) && (*modelparts != '@')) {
			printf("%d: not @\n", i);
			break;
		}
		i = 0;

		load_start_sector = 0;
		load_sectors = 0;
		get_part_info_by_name(loadname, &load_start_sector, &load_sectors);

#ifdef CFG_SUNXI_SPINOR
		load_start_sector = load_start_sector + CFG_SPINOR_GPT_ARD;
#endif

		model_start_sector[modelInfo_head->model_count] = load_start_sector;
		model_load_sectors[modelInfo_head->model_count] = load_sectors;

		memcpy(modelInfo_head->model_name[modelInfo_head->model_count], loadname, sizeof(loadname));
		modelInfo_head->model_size[modelInfo_head->model_count] = load_sectors * 512;
		if (modelInfo_head->model_count == 0) {
			modelInfo_head->model_start_address[0] = CFG_MODEL_ADDR + 512;
		} else {
			modelInfo_head->model_start_address[modelInfo_head->model_count] =
				modelInfo_head->model_start_address[modelInfo_head->model_count - 1] +
				modelInfo_head->model_size[modelInfo_head->model_count - 1];
		}

		modelInfo_head->model_count++;

		if (modelparts != NULL)
			modelparts++;
	}

	for (i = 0; i < modelInfo_head->model_count; i++) {
		ret = flash_read(model_start_sector[i], model_load_sectors[i],
				(void *)CFG_MODEL_ADDR + 512 + size_diff);
		if (ret) {
			printf("flash read model data error!\n");
			goto load_part_out;
		}
		size_diff += model_load_sectors[i] * 512;
	}

	memcpy((void *)CFG_MODEL_ADDR, (void *)modelInfo_head, 512);

	return 0;

load_part_out:
	free(modelInfo_head);
	return ret;
}
#endif // CFG_EARLY_LOAD_MODEL

int early_fastboot_init(void)
{
	int ret = -1;

	ret = gpt_env_init();
	if (ret) {
		return ret;
	}

	ret = early_fastboot_load_dtb();
	if (ret) {
		return ret;
	}

#ifdef CFG_EARLY_LOAD_MODEL
	ret = early_fastboot_load_model();
	if (ret) {
		return ret;
	}
#endif

	return 0;
}
#endif // CFG_EARLY_FASTBOOT

#ifdef CFG_EARLY_BOOT_RISCV
extern void boot_riscv(phys_addr_t base, unsigned long fdt_addr);
int early_fastboot_boot_riscv(char *partition_name)
{
	int ret = -1;
	uint riscv_start = 0;
	uint riscv_size = 0;
	char *riscv_name = NULL;
	phys_addr_t rv_image_base;
	int (*flash_read)(uint, uint, void *);

#ifdef CONFIG_RTOS_LOAD_ADDR
	rv_image_base = CONFIG_RTOS_LOAD_ADDR;
#else
#error "Not define CONFIG_RTOS_LOAD_ADDR!"
#endif

	flash_read = spl_flash_read;

	if (!partition_name) {
		riscv_name = fw_getenv("riscv_partition");
		if (riscv_name == NULL) {
			pr_emerg("no riscv_partition in env\n");
			return ret;
		}
	} else {
		riscv_name = partition_name;
	}

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
	u32 rtc_data = rtc_read_data(CFG_FLASHFLAG_RTC_INDEX);
	if (strcmp(riscv_name, RV_PARTITION_A) == 0) {
		rtc_set_data(CFG_FLASHFLAG_RTC_INDEX,
					rtc_data & ~(1 << CFG_FLASHFLAG_RTC_RV_PARTITION_BIT));
	} else if (strcmp(riscv_name, RV_PARTITION_B) == 0) {
		rtc_set_data(CFG_FLASHFLAG_RTC_INDEX,
					rtc_data | (1 << CFG_FLASHFLAG_RTC_RV_PARTITION_BIT));
	} else {
		pr_emerg("riscv partition name not match\n");
	}
#endif

	if (get_part_info_by_name(riscv_name, &riscv_start, &riscv_size)) {
		pr_emerg("Fail to get partition %s sector\n", riscv_name);
		return ret;
	}

	if (flash_read(riscv_start, riscv_size, (void *)rv_image_base)) {
		pr_emerg("Fail to load riscv\n");
		return ret;
	}
	record_time("load_rtos_partition");

	if (rv_image_base)
		boot_riscv(rv_image_base, (unsigned long)working_fdt);
	else {
		pr_emerg("rv_image_base is NULL!");
		return ret;
	}
	record_time("boot_riscv");

	return 0;
}
#endif // CFG_EARLY_BOOT_RISCV


