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
 * Sunxi A31 CPUCFG register definition.
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com
 */

#ifndef _SUNXI_CPUCFG_H
#define _SUNXI_CPUCFG_H

#include <linux/compiler.h>
#include <linux/types.h>

#ifndef __ASSEMBLY__

struct __packed sunxi_cpucfg_cpu {
	u32 rst;		/* base + 0x0 */
	u32 ctrl;		/* base + 0x4 */
	u32 status;		/* base + 0x8 */
	u8 res[0x34];		/* base + 0xc */
};

struct __packed sunxi_cpucfg_reg {
	u8 res0[0x40];		/* 0x000 */
	struct sunxi_cpucfg_cpu cpu[4];		/* 0x040 */
	u8 res1[0x44];		/* 0x140 */
	u32 gen_ctrl;		/* 0x184 */
	u32 l2_status;		/* 0x188 */
	u8 res2[0x4];		/* 0x18c */
	u32 event_in;		/* 0x190 */
	u8 res3[0xc];		/* 0x194 */
	u32 super_standy_flag;	/* 0x1a0 */
	u32 priv0;		/* 0x1a4 */
	u32 priv1;		/* 0x1a8 */
	u8 res4[0x4];		/* 0x1ac */
	u32 cpu1_pwr_clamp;	/* 0x1b0 sun7i only */
	u32 cpu1_pwroff;	/* 0x1b4 sun7i only */
	u8 res5[0x2c];		/* 0x1b8 */
	u32 dbg_ctrl1;		/* 0x1e4 */
	u8 res6[0x18];		/* 0x1e8 */
	u32 idle_cnt0_low;	/* 0x200 */
	u32 idle_cnt0_high;	/* 0x204 */
	u32 idle_cnt0_ctrl;	/* 0x208 */
	u8 res8[0x4];		/* 0x20c */
	u32 idle_cnt1_low;	/* 0x210 */
	u32 idle_cnt1_high;	/* 0x214 */
	u32 idle_cnt1_ctrl;	/* 0x218 */
	u8 res9[0x4];		/* 0x21c */
	u32 idle_cnt2_low;	/* 0x220 */
	u32 idle_cnt2_high;	/* 0x224 */
	u32 idle_cnt2_ctrl;	/* 0x228 */
	u8 res10[0x4];		/* 0x22c */
	u32 idle_cnt3_low;	/* 0x230 */
	u32 idle_cnt3_high;	/* 0x234 */
	u32 idle_cnt3_ctrl;	/* 0x238 */
	u8 res11[0x4];		/* 0x23c */
	u32 idle_cnt4_low;	/* 0x240 */
	u32 idle_cnt4_high;	/* 0x244 */
	u32 idle_cnt4_ctrl;	/* 0x248 */
	u8 res12[0x34];		/* 0x24c */
	u32 cnt64_ctrl;		/* 0x280 */
	u32 cnt64_low;		/* 0x284 */
	u32 cnt64_high;		/* 0x288 */
};

#endif /* __ASSEMBLY__ */
#endif /* _SUNXI_CPUCFG_H */
