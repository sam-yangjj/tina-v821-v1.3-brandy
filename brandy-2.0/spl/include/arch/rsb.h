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
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on allwinner u-boot sources rsb code which is:
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * lixiang <lixiang@allwinnertech.com>
 */

#ifndef __SUNXI_RSB_H
#define __SUNXI_RSB_H

#include <common.h>
#include <asm/io.h>

struct sunxi_rsb_reg {
	u32 ctrl;	/* 0x00 */
	u32 ccr;	/* 0x04 */
	u32 inte;	/* 0x08 */
	u32 stat;	/* 0x0c */
	u32 addr;	/* 0x10 */
	u8 res0[8];	/* 0x14 */
	u32 data;	/* 0x1c */
	u8 res1[4];	/* 0x20 */
	u32 lcr;	/* 0x24 */
	u32 dmcr;	/* 0x28 */
	u32 cmd;	/* 0x2c */
	u32 devaddr;	/* 0x30 */
};

#define RSB_CTRL_SOFT_RST		(1 << 0)
#define RSB_CTRL_START_TRANS		(1 << 7)

#define RSB_STAT_TOVER_INT		(1 << 0)
#define RSB_STAT_TERR_INT		(1 << 1)
#define RSB_STAT_LBSY_INT		(1 << 2)

#define RSB_DMCR_DEVICE_MODE_DATA	0x7c3e00
#define RSB_DMCR_DEVICE_MODE_START	(1 << 31)

#define RSB_CMD_BYTE_WRITE		0x4e
#define RSB_CMD_BYTE_READ		0x8b
#define RSB_CMD_SET_RTSADDR		0xe8

#define RSB_DEVADDR_RUNTIME_ADDR(x)	((x) << 16)
#define RSB_DEVADDR_DEVICE_ADDR(x)	((x) << 0)

int rsb_init(void);
int rsb_set_device_address(u16 device_addr, u16 runtime_addr);
int rsb_write(const u16 runtime_device_addr, const u8 reg_addr, u8 data);
int rsb_read(const u16 runtime_device_addr, const u8 reg_addr, u8 *data);

#endif
