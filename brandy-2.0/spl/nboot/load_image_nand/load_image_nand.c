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
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <config.h>
#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#ifdef CFG_SUNXI_BOOT_PARAM
#include <boot_param.h>
#endif
void update_flash_para(phys_addr_t uboot_base)
{
	struct spare_boot_head_t *bfh = (struct spare_boot_head_t *)uboot_base;
	bfh->boot_data.storage_type =
		(BT0_head.boot_head.platform[0] & 0xf) == 4 ? STORAGE_SPI_NAND :
								    STORAGE_NAND;
}

#ifdef CFG_SUNXI_SPINAND
#include <spinand_boot0.h>
__s32 load_toc1_from_spinand(void)
{
	__u32 i;
	__s32 status;
	__u32 length;
	__u32 read_blks;
	sbrom_toc1_head_info_t *toc1_head;
	char *buffer = (void *)CONFIG_BOOTPKG_BASE;

	if (SpiNand_PhyInit() != 0) {
		printf("fail in opening nand flash\n");
		return -1;
	}

	printf("block from %d to %d\n", UBOOT_START_BLK_NUM,
	       UBOOT_LAST_BLK_NUM);
	for (i = UBOOT_START_BLK_NUM; i <= UBOOT_LAST_BLK_NUM; i++) {
		if (SpiNand_Check_BadBlock(i) == SPINAND_BAD_BLOCK) {
			printf("spi nand block %d is bad\n", i);
			continue;
		}
		if (SpiNand_Read(i * (SPN_BLOCK_SIZE >> NF_SCT_SZ_WIDTH),
				 (void *)buffer, 1) == NAND_OP_FALSE) {
			printf("the first data is error\n");
			continue;
		}
		toc1_head = (sbrom_toc1_head_info_t *)buffer;
		if (toc1_head->magic != TOC_MAIN_INFO_MAGIC) {
			printf("%s err:  magic is invalid\n", __func__);
			continue;
		}

		//check align
		length = toc1_head->valid_len;
		if ((length & (ALIGN_SIZE - 1)) != 0) {
			printf("the boot1 is not aligned by 0x%x\n",
			       ALIGN_SIZE);
			continue;
		}

		status = Spinand_Load_Boot1_Copy(i, (void *)buffer, length,
						 SPN_BLOCK_SIZE, &read_blks);
		if (status == NAND_OP_FALSE) {
			printf("SPI nand load uboot copy fail\n");
			continue;
		}
		if (verify_addsum(buffer, length) == 0) {
			printf("Check is correct. Find a good uboot copy at block %u\n",
			       i);
#ifndef CFG_BOOT0_LOAD_KERNEL
			SpiNand_PhyExit();
#endif
			if (i ==
			    UBOOT_START_BLK_NUM) { /* check which uboot to load */
				uboot_backup = UBOOTA;
			} else {
				uboot_backup = UBOOTB;
			}
			return 0;
		}
	}

	printf("Can't find a good Boot1 copy in spi nand.\n");
#ifndef CFG_BOOT0_LOAD_KERNEL
	SpiNand_PhyExit();
#endif
	return -1;
}

#else
__s32 load_toc1_from_spinand(void)
{
	return -1;
}

#endif

int BOOT_NandGetPara(void *param, uint size)
{
#ifdef CFG_SUNXI_BOOT_PARAM
	typedef_sunxi_boot_param *sunxi_boot_param = sunxi_bootparam_get_buf();

	if (sunxi_boot_param->header.check_sum == 0) {
		memcpy((void *)param, BT0_head.prvt_head.storage_data, size);
	} else {
		if ((BT0_head.boot_head.platform[0] & 0xf) == 4)
			memcpy((void *)param, sunxi_boot_param->spiflash_info,
			       size);
		else
			memcpy((void *)param, sunxi_boot_param->nand_info,
			       size);
	}
#else
	memcpy((void *)param, BT0_head.prvt_head.storage_data, size);
#endif
	return 0;
}

int load_package(phys_addr_t *uboot_base)
{
	if ((BT0_head.boot_head.platform[0] & 0xf) == 4) {
		return load_toc1_from_spinand();
	} else {
		return -1;
	}
}

int flash_init(void)
{
#ifdef CFG_SUNXI_SPINAND
	if ((BT0_head.boot_head.platform[0] & 0xf) == 4) {
		if (SpiNand_PhyInit() != 0) {
			pr_emerg("fail in opening nand flash\n");
			return -1;
		}
	}
#endif
	return 0;
}
