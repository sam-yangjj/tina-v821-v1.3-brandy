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
#ifndef _SUNXI_MMC_BOOT0_H
#define _SUNXI_MMC_BOOT0_H

#define CARD_TYPE_MMC	0x8000000
#define CARD_TYPE_SD	0x8000001

#define MMC_EARLY_INIT 1
#define MMC_NOT_INIT   0

enum {
	E_SDMMC_OK	       = 0,
	E_SDMMC_NUM_ERR	       = 1,
	E_SDMMC_INIT_ERR       = 2,
	E_SDMMC_READ_ERR       = 3,
	E_SDMMC_FIND_BOOT1_ERR = 4,
};

void set_mmc_para(int smc_no, void *sdly_addr, phys_addr_t uboot_base);
int get_card_type(void);
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
unsigned long mmc_bread_async(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
unsigned long mmc_wait_bread_done(int dev_num);
int mmc_query_cmd_send(int dev_num);
int sunxi_mmc_init(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset);
int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);

#endif /* _SUNXI_MMC_BOOT0_H */
