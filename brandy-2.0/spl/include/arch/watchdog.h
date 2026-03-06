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
 * (C) Copyright 2014
 * Chen-Yu Tsai <wens@csie.org>
 *
 * Watchdog register definitions
 */

#ifndef _SUNXI_WATCHDOG_H_
#define _SUNXI_WATCHDOG_H_

#include <linux/types.h>
#include <asm/io.h>
#include <common.h>

#define WDT_CTRL_RESTART	(0x1 << 0)
#define WDT_CTRL_KEY		(0x0a57 << 1)

#if defined(CONFIG_MACH_SUN4I) || \
    defined(CONFIG_MACH_SUN5I) || \
    defined(CONFIG_MACH_SUN7I) || \
    defined(CONFIG_MACH_SUN8I_R40) || \
    defined(CONFIG_MACH_SUN8IW11)

#define WDT_MODE_EN		(0x1 << 0)
#define WDT_MODE_RESET_EN	(0x1 << 1)

struct sunxi_wdog {
	u32 ctl;		/* 0x00 */
	u32 mode;		/* 0x04 */
	u32 res[2];
};
#elif defined(CONFIG_ARCH_SUN8IW21) || defined(CONFIG_ARCH_SUN300IW1)

#define WDT_CFG_RESET		(1)
#define WDT_MODE_EN		(1)
#define	WDT_CTRL_RELOAD         ((1 << 0) | (0x0a57 << 1))
#define KEY_FIELD         	(0x16aa << 16)

struct sunxi_wdog {
	u32 irq_en;		/* 0xa0 */
	u32 irq_sta;		/* 0xa4 */
	u32 soft_rst;		/* 0xa8 */
	u32 res1[1];		/* 0xac */
	u32 ctl;		/* 0xb0 */
	u32 cfg;		/* 0xb4 */
	u32 mode;		/* 0xb8 */
	u32 output_cfg;		/* 0xbc */
};

#else

#define WDT_CFG_RESET		(0x1)
#define WDT_MODE_EN		(0x1)

struct sunxi_wdog {
	u32 irq_en;		/* 0x00 */
	u32 irq_sta;		/* 0x04 */
	u32 res1[2];
	u32 ctl;		/* 0x10 */
	u32 cfg;		/* 0x14 */
	u32 mode;		/* 0x18 */
	u32 res2;
};

#endif

void wdt_start(unsigned int timeout);
void wdt_stop(void);

#endif /* _SUNXI_WATCHDOG_H_ */
