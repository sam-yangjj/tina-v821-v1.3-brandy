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

#ifndef __BOOT_PARAM_H
#define __BOOT_PARAM_H
#include <common.h>
#define BOOT_PARAM_MAGIC		"bootpara"

/* bit map
 * transfer_flag:bit0
 * 1:uboot download bootparam  0: nothing */

#define BOOTPARAM_DOWNLOAD_MASK  (0x1)
struct sunxi_boot_parameter_header {
	u8 magic[8]; //bootpara
	u32 version; // describe the region version
	u32 check_sum;
	u32 length;
	u32 boot0_checksum;
	u32 transfer_flag;
	u8 reserved[4];
};


#define MAX_DRAMPARA_SIZE (96)
typedef struct {
	unsigned int dram_para[MAX_DRAMPARA_SIZE];
	char res[512 - 4 * MAX_DRAMPARA_SIZE];
} boot_dram_info_t;

// BOOT_ PARAM_ SIZE maximum value is 4k
#define BOOT_PARAM_SIZE (2048)
typedef struct sunxi_boot_param_region{
	struct sunxi_boot_parameter_header header;
	char sdmmc_info[256 -32];
	char nand_info[256];
	char spiflash_info[256];
	char ufs[256];
	char ddr_info[512];
	u8 reserved[BOOT_PARAM_SIZE - 512*3];// = 2048 - 32 - sdmmc_size - nand_size - spi_size - ddr_size
} typedef_sunxi_boot_param;


int sunxi_bootparam_load(void);

boot_dram_info_t *sunxi_bootparam_get_dram_buf(void);
typedef_sunxi_boot_param *sunxi_bootparam_get_buf(void);
int sunxi_bootparam_format_and_transfer(void *dest);
#endif
