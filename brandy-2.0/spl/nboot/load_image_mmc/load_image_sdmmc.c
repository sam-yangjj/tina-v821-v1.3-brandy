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

#include <common.h>
#include <spare_head.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <mmc_boot0.h>
#include <sunxi_flashmap.h>
#include <libfdt.h>
#ifdef CFG_SUNXI_GPT
#include <part_efi.h>
#endif

int mmc_config_addr;

typedef struct _boot_sdcard_info_t
{
	__s32	card_ctrl_num;                //总共的卡的个数
	__s32	boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
	__s32	card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
	__s32	speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	__s32	line_sel[4];                  //卡的线制，0: 1线，其它，4线
	__s32	line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;

//card num: 0-sd 1-card3 2-emmc
int get_card_num(void)
{
	int card_num = 0;

	card_num = BT0_head.boot_head.platform[0] & 0xf;
	card_num = (card_num == 1)? 3: card_num;
	return card_num;
}

int mmc_bread_ext(uint start, uint blkcnt,
			void *dst)
{
	return !mmc_bread(get_card_num(), start, blkcnt, dst);
}

int mmc_bread_ext_async(uint start, uint blkcnt,
			void *dst)
{
	return !mmc_bread_async(get_card_num(), start, blkcnt, dst);
}

int mmc_bread_count(void)
{
	return mmc_query_cmd_send(get_card_num());
}

int mmc_wait_bread_ext_async_done(void)
{
	return !mmc_wait_bread_done(get_card_num());
}

void update_flash_para(phys_addr_t uboot_base)
{
	int card_num;
	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) uboot_base;

	card_num = get_card_num();
	set_mmc_para(card_num, (void *)&BT0_head.prvt_head.storage_data, uboot_base);
#ifdef CONFIG_ARCH_SUN50IW11
	int card_type = get_card_type();
	if (card_num == 0) {
		if (card_type == CARD_TYPE_MMC)
			bfh->boot_data.storage_type = STORAGE_EMMC0;
		else
			bfh->boot_data.storage_type = STORAGE_SD;
	}
#else
	if (card_num == 0) {
		bfh->boot_data.storage_type = STORAGE_SD;
	}
#endif
	else if (card_num == 2) {
		bfh->boot_data.storage_type = STORAGE_EMMC;
	} else if (card_num == 3) {
		bfh->boot_data.storage_type = STORAGE_EMMC3;
	}
}

int sunxi_mmc_init_ext(void)
{
	int card_no		   = 0;
	int error_num		   = E_SDMMC_OK;
	static int early_init_flag = MMC_NOT_INIT;
	if (early_init_flag == MMC_EARLY_INIT) {
		return 0;
	}
	card_no = get_card_num();
	boot_sdcard_info_t *sdcard_info =
		(boot_sdcard_info_t *)(char *)BT0_head.prvt_head.storage_data;

	printf("card no is %d\n", card_no);
	if (card_no < 0) {
		error_num = E_SDMMC_NUM_ERR;
		return error_num;
	}

	if (!sdcard_info->line_sel[card_no]) {
		sdcard_info->line_sel[card_no] = 4;
	}
	printf("sdcard %d line count %d\n", card_no,
	       sdcard_info->line_sel[card_no]);

	if (sunxi_mmc_init(card_no, sdcard_info->line_sel[card_no],
			   BT0_head.prvt_head.storage_gpio, 16) == -1) {
		error_num = E_SDMMC_INIT_ERR;
		return error_num;
	}
	early_init_flag = MMC_EARLY_INIT;
	return 0;
}

#define ONE_BLOCK (1)
int sunxi_mmc_cpu_read(uint start, uint blkcnt, void *dst)
{
	/* Using the CPU to read flash, only one sector
	 * can be read at a time
	 */
	if (blkcnt != ONE_BLOCK) {
		printf("only read one sector\n");
		return -1;
	}
	return !mmc_bread(get_card_num(), start, ONE_BLOCK, dst);
}
#ifdef CFG_SUNXI_ENV
int spl_flash_init(void)
{
	return 0;
}

int spl_flash_read(uint start, uint sector_cnt, void *buffer)
{
	int	card_no = get_card_num();
#ifdef CFG_SUNXI_LOGIC_OFFSET
	mmc_bread(card_no, start + CFG_SUNXI_LOGIC_OFFSET, sector_cnt, buffer);
#else
	mmc_bread(card_no, start, sector_cnt, buffer);
#endif
	return 0;
}
#endif

#ifdef CFG_SUNXI_GPT
int load_gpt(void *buffer)
{
	uint sectors;
#ifdef CFG_SUNXI_LOGIC_OFFSET
	uint start_sector = 0;
#else
	uint start_sector = CFG_MMC_GPT_ARD;
#endif
	int	card_no;
//	int ret = 0;

	card_no = get_card_num();
	sectors = 1 * SUNXI_MBR_SIZE / 512;
	//printf("GPT load from sector:0x%x, sectors: 0x%x, save in dram: %08x\n",
	//		start_sector, sectors, buffer);
//	ret = mmc_bread(card_no, start_sector, sectors, buffer);
	mmc_bread(card_no, start_sector, sectors, buffer);
//	if (ret) {
//		printf("load GPT from sdmmc fail ret:%d\n", ret);
//		return -1;
//	}
	return 0;
}
#endif

#ifdef CFG_SUNXI_OTA_ENABLE
int load_toc1_ota_from_sdmmc(char *buf)
{
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint total_size;
	sbrom_toc1_head_info_t	*toc1_head;
	int  card_no;
	int ret =0;
	uint start_sector = 0, sector_num = 0, i = 0;
	int error_num = E_SDMMC_OK;
	char *boot_partation = NULL;

	mmc_config_addr = (u32)((phys_addr_t)(BT0_head.prvt_head.storage_data));
	card_no = get_card_num();

	error_num = sunxi_mmc_init_ext();
	if (error_num != E_SDMMC_OK)
	{
		error_num = E_SDMMC_INIT_ERR;
		goto __ERROR_EXIT;;
	}

	if (init_gpt()) {
		printf("init GPT fail\n");
	}
	if (fw_env_open() == -1) {
		printf("fw_env_open failed\n");
		return -1;
	}
#if ENV_DEBUG
	fw_env_dump();
#endif
	boot_partation = fw_getenv("boot_partation");
	fw_env_close();

	printf("%s %d %s boot_partation:%s\n", __FILE__, __LINE__, __func__, boot_partation);
	get_part_info_by_name(boot_partation, &start_sector, &sector_num);

	for(i=0; i < 4; i++)
	{
		tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
		if(start_sector == 0)
		{
			error_num = E_SDMMC_FIND_BOOT1_ERR;
			goto __ERROR_EXIT;
		}
		ret = mmc_bread(card_no, start_sector, 64, tmp_buff);
		if(!ret)
		{
			error_num = E_SDMMC_READ_ERR;
			goto __ERROR_EXIT;
		}
		toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
		if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
		{
			printf("error:bad magic.\n");
			continue;
		}
		total_size = toc1_head->valid_len;
		if(total_size > 64 * 512)
		{
			tmp_buff += 64*512;
			ret = mmc_bread(card_no, start_sector + 64, (total_size - 64*512 + 511)/512, tmp_buff);
			if(!ret)
			{
				error_num = E_SDMMC_READ_ERR;
				goto __ERROR_EXIT;
			}
		}

		if( verify_addsum( (__u32 *)CONFIG_BOOTPKG_BASE, total_size) != 0 )
		{
			printf("error:bad checksum.\n");
			continue;
		}
		break;
	}
	printf("Loading boot-pkg Succeed(index=%d).\n", (BT0_head.boot_head.platform[0] & 0xf0)>>4);
#if !defined(CFG_BOOT0_LOAD_KERNEL) && !defined(CFG_DEFAULT_BOOT_RTOS)
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16);
#endif
	deinit_gpt();

	return 0;

__ERROR_EXIT:
	printf("Loading boot-pkg fail(error=%d)\n",error_num);
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16 );
	return -1;

}
#else
int load_toc1_from_sdmmc(char *buf)
{
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint total_size;
	sbrom_toc1_head_info_t	*toc1_head;
	int  card_no;
	int ret = 0;
	int start_sector = 0, i;
	int error_num = E_SDMMC_OK;
	int start_sectors[4] = {0};

	mmc_config_addr = (u32)((phys_addr_t)(BT0_head.prvt_head.storage_data));
	card_no = get_card_num();

	error_num = sunxi_mmc_init_ext();
	if (error_num != E_SDMMC_OK) {
		error_num = E_SDMMC_INIT_ERR;
		goto __ERROR_EXIT;;
	}

	start_sectors[0] = sunxi_flashmap_toc1_start(FLASHMAP_SDMMC);
	start_sectors[1] = sunxi_flashmap_toc1_bak_start(FLASHMAP_SDMMC);

	for (i = 0; i < 4; i++) {
		start_sector = start_sectors[i];
		tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
		if (start_sector == 0) {
			error_num = E_SDMMC_FIND_BOOT1_ERR;
			goto __ERROR_EXIT;
		}
		ret = mmc_bread(card_no, start_sector, 64, tmp_buff);
		if (!ret) {
			error_num = E_SDMMC_READ_ERR;
			goto __ERROR_EXIT;
		}
		toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
		if (toc1_head->magic != TOC_MAIN_INFO_MAGIC) {
			printf("error:bad magic.\n");
			continue;
		}
		total_size = toc1_head->valid_len;
		if (total_size > 64 * 512) {
			tmp_buff += 64*512;
			ret = mmc_bread(card_no, start_sector + 64, (total_size - 64*512 + 511)/512, tmp_buff);
			if (!ret) {
				error_num = E_SDMMC_READ_ERR;
				goto __ERROR_EXIT;
			}
		}

		if (verify_addsum((__u32 *)CONFIG_BOOTPKG_BASE, total_size) != 0) {
			printf("error:bad checksum.\n");
			continue;
		}
		break;
	}
	printf("Loading boot-pkg Succeed(index=%d).\n", (BT0_head.boot_head.platform[0] & 0xf0) >> 4);
#if !defined(CFG_BOOT0_LOAD_KERNEL) && !defined(CFG_DEFAULT_BOOT_RTOS)
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16);
#endif
	return 0;

__ERROR_EXIT:
	printf("Loading boot-pkg fail(error=%d)\n", error_num);
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16);
	return -1;

}

#endif

#ifdef CFG_LOAD_UBOOT_PARTITIO
int load_uboot_partition(phys_addr_t *uboot_base)
{
	int i;
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint total_size;
	struct spare_boot_ctrl_head *uboot_head;
	int  card_no;
	int ret = 0;
	int error_num = E_SDMMC_OK;
	uint start_sectors[2] = {0};
	uint start_sector;
	uint sector_num;
	phys_addr_t uboot_run_addr;

	mmc_config_addr = (u32)((phys_addr_t)(BT0_head.prvt_head.storage_data));
	card_no = get_card_num();

	error_num = sunxi_mmc_init_ext();
	if (error_num != E_SDMMC_OK) {
		error_num = E_SDMMC_INIT_ERR;
		goto __ERROR_EXIT;
	}

	if (init_gpt()) {
		printf("init GPT fail\n");
		goto __ERROR_EXIT;
	}
	get_part_info_by_name("uboot", start_sectors, &sector_num);
	get_part_info_by_name("uboot-bak", start_sectors + 1, &sector_num);

	printf("uboot_start_sector= 0x%x, uboot_sector_num= %d\n",  start_sectors[0], sector_num);
	printf("uboot_backup_start_sector= 0x%x, uboot_sector_num= %d\n",  start_sectors[1], sector_num);

	for (i = 0; i < 2; i++) {
		start_sector = start_sectors[i];
		if (start_sector == 0) {
			 printf("get uboot part info Fail!\n");
			goto __ERROR_EXIT;
		}

		ret = mmc_bread(card_no, start_sector, 64, tmp_buff);
		if (!ret) {
			printf("read uboot_head Fail!\n");
			goto __ERROR_EXIT;
		}

		uboot_head = (struct spare_boot_ctrl_head *)tmp_buff;
		total_size = uboot_head->length;
		uboot_run_addr = uboot_head->reserved;

		printf("uboot_length= 0x%x\n",  total_size);
		printf("uboot_run_addr= 0x%x\n", uboot_head->reserved);

		if (!total_size) {
			printf("error: fail to get right length.\n");
			continue;
		}

		ret = mmc_bread(card_no, start_sector, total_size / 512, (void *)uboot_run_addr);
		if (!ret) {
			printf("read uboot Fail!\n");
			goto __ERROR_EXIT;
		}

		if (sunxi_verify_checksum((void *)uboot_run_addr, total_size, uboot_head->check_sum) < 0) {
			printf("error: bad checksum.\n");
			if (i == 1) {
				printf("uboot and uboot-bak partition both check fail!\n");
				goto __ERROR_EXIT;
			} else {
				continue;
			}
		}
		break;
	}

	/* load uboot-dtb */
	phys_addr_t uboot_dtb_addr = 0;
	uboot_dtb_addr = (uboot_run_addr + 2 * 1024 * 1024);
	printf("uboot_dtb_addr = 0x%x\n", uboot_dtb_addr);

	working_fdt = (struct fdt_header *)uboot_dtb_addr;
	get_part_info_by_name("uboot-dtb", start_sectors, &sector_num);
	if (start_sector == 0) {
		printf("get uboot-dtb part info Fail!\n");
	goto __ERROR_EXIT;
    }

	printf("uboot-dtb_start_sectors= 0x%x, uboot-dtb_start_sectors = %d\n",  start_sectors[0], sector_num);
	start_sector = start_sectors[0];
	ret = mmc_bread(card_no, start_sector, sector_num, (void *)uboot_dtb_addr);
	if (!ret) {
		printf("read uboot-dtb Fail!\n");
		goto __ERROR_EXIT;
	}
	if (fdt_check_header((void *)uboot_dtb_addr)) {
		printf("fdt_check_header Fail!\n");
		goto __ERROR_EXIT;
    }

	*uboot_base = (phys_addr_t)uboot_run_addr;
	printf("Loading uboot partition Succeed.\n");
	return 0;

__ERROR_EXIT:
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16);
	return -1;
}
#endif
int load_package(phys_addr_t *uboot_base)
{
#ifdef CFG_LOAD_UBOOT_PARTITIO
	int ret;
	ret = load_uboot_partition(uboot_base);
	if (ret)
		return ret;
#endif
#ifdef CFG_SUNXI_OTA_ENABLE
	//memcpy((void *)DRAM_PARA_STORE_ADDR, (void *)BT0_head.prvt_head.dram_para, 
	//	SUNXI_DRAM_PARA_MAX * 4);
	return load_toc1_ota_from_sdmmc((char *)BT0_head.prvt_head.storage_data);
#else
	return load_toc1_from_sdmmc((char *)BT0_head.prvt_head.storage_data);
#endif
}

int flash_init(void)
{
	int error_num, card_no;

	card_no = get_card_num();
	error_num = sunxi_mmc_init_ext();

	if (error_num != E_SDMMC_OK) {
		pr_emerg("sdmmc%d init fail\n", card_no);
		sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16);
		return -1;
	}

	return 0;
}
