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
 * Copyright (c) 2022-2024 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : flashmap.h
 * Description :
 * History :
 *    Author  :  <huangrongcun@allwinnertech.com>
 *    Date    : 2022/04/07
 *    Comment : mmc flash map
 */

#include <sunxi_flashmap.h>
#include <config.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <spare_head.h>
#include <common.h>
#include <boot_param.h>

#if 0
#define flashmap_debug(fmt, arg...) printf(fmt, ##arg)
#else
#define flashmap_debug(fmt, arg...)
#endif

/*note: Unit: sector*/
#ifdef SUNXI_SDMMC_PARAMETER_REGION_LBA_START
#define DEFAULT_SUNXI_SDMMC_PARAMETER_REGION_LBA_START    \
		SUNXI_SDMMC_PARAMETER_REGION_LBA_START
#else
#define DEFAULT_SUNXI_SDMMC_PARAMETER_REGION_LBA_START (24504)
#endif

#ifdef UBOOT_START_SECTOR_IN_SDMMC
#define DEFAULT_UBOOT_START_SECTOR_IN_SDMMC    \
		UBOOT_START_SECTOR_IN_SDMMC
#else
#define DEFAULT_UBOOT_START_SECTOR_IN_SDMMC (32800)
#endif

#ifdef UBOOT_START_SECTOR_IN_UFS
#define DEFAULT_UBOOT_START_SECTOR_IN_UFS    \
		UBOOT_START_SECTOR_IN_UFS
#else
#define DEFAULT_UBOOT_START_SECTOR_IN_UFS (32800)
#endif



#ifdef UBOOT_BACKUP_START_SECTOR_IN_SDMMC
#define DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_SDMMC    \
		UBOOT_BACKUP_START_SECTOR_IN_SDMMC
#else
#define DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_SDMMC (24576)
#endif

#ifdef UBOOT_BACKUP_START_SECTOR_IN_UFS
#define DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_UFS    \
		UBOOT_BACKUP_START_SECTOR_IN_UFS
#else
#define DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_UFS (24576)
#endif

#ifdef CFG_SPINOR_UBOOT_OFFSET
#define DEFAULT_CFG_SPINOR_UBOOT_OFFSET   CFG_SPINOR_UBOOT_OFFSET
#else
#define DEFAULT_CFG_SPINOR_UBOOT_OFFSET   (128)
#endif

#ifdef CFG_SPINOR_UBOOT_PARAMS_OFFSET
#define DEFAULT_BOOT_PARAM_START          CFG_SPINOR_UBOOT_PARAMS_OFFSET
#else
#define DEFAULT_BOOT_PARAM_START          (DEFAULT_CFG_SPINOR_UBOOT_OFFSET - 8)
#endif

int sunxi_flashmap_bootparam_start(enum flash_type flash_name)
{
#ifdef CFG_SUNXI_SBOOT
	if (sboot_head.flash_map.boot_param > 0)
		return sboot_head.flash_map.boot_param;
#else
	if (BT0_head.flash_map.boot_param > 0)
		return BT0_head.flash_map.boot_param;
#endif
	switch (flash_name) {
	case FLASHMAP_SPI_NOR:
		return DEFAULT_BOOT_PARAM_START;
		break;
	case FLASHMAP_SDMMC:
		return DEFAULT_SUNXI_SDMMC_PARAMETER_REGION_LBA_START;
		break;
	default:
		return 0;
		break;
	}
}

int sunxi_flashmap_toc1_start(enum flash_type flash_name)
{
#ifdef CFG_SUNXI_SBOOT
	if (sboot_head.flash_map.uboot_start_sector > 0) {
		return sboot_head.flash_map.uboot_start_sector;
	}
#else
	if (BT0_head.flash_map.uboot_start_sector > 0) {
		return BT0_head.flash_map.uboot_start_sector;
	}
#endif
	switch (flash_name) {
	case FLASHMAP_SPI_NOR:
		return DEFAULT_CFG_SPINOR_UBOOT_OFFSET;
		break;
	case FLASHMAP_SDMMC:
		return DEFAULT_UBOOT_START_SECTOR_IN_SDMMC;
		break;
	case FLASHMAP_UFS:
		return DEFAULT_UBOOT_START_SECTOR_IN_UFS;
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

int sunxi_flashmap_toc1_bak_start(enum flash_type flash_name)
{
#ifdef CFG_SUNXI_SBOOT
	if (sboot_head.flash_map.uboot_start_sector > 0) {
		return sboot_head.flash_map.uboot_bak_start_sector;
	}
#else
	if (BT0_head.flash_map.uboot_start_sector > 0) {
		return BT0_head.flash_map.uboot_bak_start_sector;
	}
#endif

	switch (flash_name) {
	case FLASHMAP_SPI_NOR:
		return 0;
		break;
	case FLASHMAP_SDMMC:
		return DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_SDMMC;
		break;
	case FLASHMAP_UFS:
		return DEFAULT_UBOOT_BACKUP_START_SECTOR_IN_UFS;
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

int sunxi_flashmap_info_dump(void)
{
#ifdef CFG_SUNXI_SBOOT
	flashmap_debug("SBOOT:flash map:boot_param:%d,uboot_start_sector:%d,uboot_start_bak_sector:%d\n",
				sboot_head.flash_map.boot_param,
				sboot_head.flash_map.uboot_start_sector,
				sboot_head.flash_map.uboot_bak_start_sector);
#else
	flashmap_debug("NBOOT:flash map:boot_param:%d,uboot_start_sector:%d,uboot_start_bak_sector:%d\n",
				BT0_head.flash_map.boot_param,
				BT0_head.flash_map.uboot_start_sector,
				BT0_head.flash_map.uboot_bak_start_sector);
#endif
	return 0;
}
