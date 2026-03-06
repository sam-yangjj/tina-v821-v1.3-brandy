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
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom wangwei <wangwei@allwinnertech.com>
 */

#include <boot_param.h>
#include <common.h>
#include <config.h>
#include <private_boot0.h>
#include <sunxi_flashmap.h>
#include <private_toc.h>
#include <config.h>

#if defined(CFG_SUNXI_NAND) || defined(CFG_SUNXI_SPINAND)
#include <spinand_boot0.h>
#include <nand_boot0.h>
#endif


#define BOOT_PARA_DEBUG 0
#define  ADD_CHECK_SUM 1

#if BOOT_PARA_DEBUG
#define BOOT_PARAM_DBG(fmt, arg...)                                            \
	printf("boot param- " fmt, ##arg)
#define BOOT_PARAM_INF(fmt, arg...)                                            \
	printf("boot param - " fmt, ##arg)
#define BOOT_PARAM_ERR(fmt, arg...)                                            \
	printf("boot param- " fmt, ##arg)

#else
#define BOOT_PARAM_DBG(fmt, arg...)
#define BOOT_PARAM_INF(fmt, arg...)
#define BOOT_PARAM_ERR(fmt, arg...)                                            \
	printf("boot param - " fmt, ##arg)
#endif

static typedef_sunxi_boot_param sunxi_boot_param = { 0 };
bool flash_disable_dma;

__maybe_unused static int sunxi_flash_init_early(void)
{
#if defined(CFG_SUNXI_MMC) || defined(CFG_SUNXI_SDMMC)
	return sunxi_mmc_init_ext();
#elif defined(CFG_SUNXI_NAND) || defined(CFG_SUNXI_SPINAND)
#if CFG_SUNXI_SBOOT
	toc0_private_head_t *toc0 =
		(toc0_private_head_t *)CONFIG_TOC0_HEAD_BASE;
	if ((toc0->platform[0] & 0xf) == 4)
#else
	if ((BT0_head.boot_head.platform[0] & 0xf) == 4)
#endif
		SpiNand_PhyInit();
	else
		BOOT_PARAM_ERR("This media is not supported\n");
#else
	BOOT_PARAM_ERR("This media is not supported\n");
#endif
	return 0;
}

__maybe_unused static int sunxi_bootparam_cpu_read(int size_sector, void *buf)
{
#if defined(CFG_SUNXI_MMC) || defined(CFG_SUNXI_SDMMC)
#define MMC_BOOT_PARAM_COMMON_OFFSET 8
	u32 start_sector = sunxi_flashmap_bootparam_start(FLASHMAP_SDMMC) -
			   MMC_BOOT_PARAM_COMMON_OFFSET;

	int i = 0;
	for (i = 0; i < size_sector; i++) {
		sunxi_mmc_cpu_read(start_sector + i, 1, buf + (512 * i));
	}

#elif defined(CFG_SUNXI_NAND) || defined(CFG_SUNXI_SPINAND)
#if CFG_SUNXI_SBOOT
	toc0_private_head_t *toc0 =
		(toc0_private_head_t *)CONFIG_TOC0_HEAD_BASE;
	if ((toc0->platform[0] & 0xf) == 4) {
#else
	if ((BT0_head.boot_head.platform[0] & 0xf) == 4) {
#endif
		printf("BOOT_PARAM_OFFSET:0x%x \n", BOOT_PARAM_OFFSET);
		SpiNand_Read(BOOT_PARAM_OFFSET >> SCT_SZ_WIDTH,
				buf, BOOT_PARAM_SIZE >> SCT_SZ_WIDTH);
	} else {
		BOOT_PARAM_ERR("Media not supported%s...%d\n", __func__, __LINE__);
	}
#else
	BOOT_PARAM_ERR("Media not supported%s...%d\n", __func__, __LINE__);
#endif
	return 0;
}

static int sunxi_bootparam_check_boot0_version(void)
{
#ifdef CFG_SUNXI_FES
	//nothing
	return 0;
#elif CFG_SUNXI_SBOOT
	toc0_private_head_t *toc0 =
		(toc0_private_head_t *)CONFIG_TOC0_HEAD_BASE;
	if (sunxi_boot_param.header.boot0_checksum == toc0->check_sum) {
		return 0;
	}
#else
	if (sunxi_boot_param.header.boot0_checksum ==
	    BT0_head.boot_head.check_sum) {
		return 0;
	}
#endif
	BOOT_PARAM_ERR("boot0 checksum not match\n");
	return -1;
}

__maybe_unused static void sunxi_bootparam_record_boot0_version(void)
{
#ifdef CFG_SUNXI_FES
	//nothing
#elif CFG_SUNXI_SBOOT
	toc0_private_head_t *toc0 =
		(toc0_private_head_t *)CONFIG_TOC0_HEAD_BASE;
	sunxi_boot_param.header.boot0_checksum = toc0->check_sum;
#else
	sunxi_boot_param.header.boot0_checksum = BT0_head.boot_head.check_sum;
#endif
}

__maybe_unused static int sunxi_bootparam_check_legality(void)
{
	if (strncmp((char *)sunxi_boot_param.header.magic, BOOT_PARAM_MAGIC,
		    strlen(BOOT_PARAM_MAGIC))) {
		BOOT_PARAM_ERR("magic error\n");
		return -1;
	}
	if (sunxi_bootparam_check_boot0_version() < 0) {
		BOOT_PARAM_ERR("version error\n");
		return -1;
	}

#if ADD_CHECK_SUM
	if (sunxi_verify_checksum((void *)&sunxi_boot_param,
				  sizeof(typedef_sunxi_boot_param),
				  sunxi_boot_param.header.check_sum) < 0) {
		BOOT_PARAM_ERR("bootparam checksum not match\n");
		return -1;
	}

#endif
	return 0;
}

boot_dram_info_t *sunxi_bootparam_get_dram_buf(void)
{
	return (boot_dram_info_t *)sunxi_boot_param.ddr_info;
}

// sunxi_bootparam_get_buf(void)
typedef_sunxi_boot_param *sunxi_bootparam_get_buf(void)
{
	return (typedef_sunxi_boot_param *)&sunxi_boot_param;
}

#ifdef CFG_SUNXI_FES
int sunxi_bootparam_load(void)
{
	sunxi_boot_param.header.transfer_flag |= BOOTPARAM_DOWNLOAD_MASK;
	memcpy(sunxi_boot_param.ddr_info, (void *)fes1_head.prvt_head.dram_para,
	       sizeof(fes1_head.prvt_head.dram_para));
	strcpy((char *)sunxi_boot_param.header.magic, BOOT_PARAM_MAGIC);
	return 0;
}

#else
int sunxi_bootparam_load(void)
{
	flash_disable_dma = 1;
	sunxi_flash_init_early();
	// 512 byte align
	assert((sizeof(typedef_sunxi_boot_param) && 0x1ff) > 0);
	u32 size_sector = sizeof(typedef_sunxi_boot_param) / 512;
	sunxi_bootparam_cpu_read(size_sector, (void *)(&sunxi_boot_param));

	if (sunxi_bootparam_check_legality() < 0) {
		BOOT_PARAM_DBG("use head param\n");
		memset((void *)&sunxi_boot_param, 0,
		       sizeof(typedef_sunxi_boot_param));
#if CFG_SUNXI_SBOOT
		memcpy(sunxi_boot_param.ddr_info,
		       (void *)toc0_config->dram_para,
		       sizeof(toc0_config->dram_para));
#else
		memcpy(sunxi_boot_param.ddr_info,
		       (void *)BT0_head.prvt_head.dram_para,
		       sizeof(BT0_head.prvt_head.dram_para));
#endif
		sunxi_boot_param.header.transfer_flag |=
			BOOTPARAM_DOWNLOAD_MASK;

		strcpy((char *)sunxi_boot_param.header.magic, BOOT_PARAM_MAGIC);
		sunxi_bootparam_record_boot0_version();
	} else {
		BOOT_PARAM_DBG("use boot param\n");
	}

#if BOOT_PARA_DEBUG
	ndump((void *)&sunxi_boot_param, sizeof(typedef_sunxi_boot_param));
#endif
	flash_disable_dma = 0;
	return 0;
}
#endif

int sunxi_bootparam_format_and_transfer(void *dest)
{

#if ADD_CHECK_SUM
	sunxi_boot_param.header.check_sum = sunxi_generate_checksum(
		(void *)&sunxi_boot_param, sizeof(typedef_sunxi_boot_param), 1,
		sunxi_boot_param.header.check_sum);
#endif

	memcpy(dest, (void *)sunxi_bootparam_get_buf(), sizeof(typedef_sunxi_boot_param));
#if BOOT_PARA_DEBUG
	BOOT_PARAM_DBG("check_sum=0x%x\n", sunxi_boot_param.header.check_sum);
	BOOT_PARAM_DBG("boot0 check_sum=0x%x\n", sunxi_boot_param.header.boot0_checksum);
	BOOT_PARAM_DBG("dump sunxi_boot_param(0x%x)\n",
			       (unsigned int)dest);
	ndump((void *)dest, sizeof(typedef_sunxi_boot_param));
#endif

	return 0;
}
