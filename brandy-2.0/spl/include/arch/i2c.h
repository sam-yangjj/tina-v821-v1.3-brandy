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
 * Copyright 2014 - Hans de Goede <hdegoede@redhat.com>
 */
#ifndef _SUNXI_I2C_H_
#define _SUNXI_I2C_H_

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_I2C0_ENABLE
#define CONFIG_I2C_MVTWSI_BASE0	SUNXI_TWI0_BASE
#endif
#ifdef CONFIG_I2C1_ENABLE
#define CONFIG_I2C_MVTWSI_BASE1	SUNXI_TWI1_BASE
#endif
#ifdef CONFIG_I2C2_ENABLE
#define CONFIG_I2C_MVTWSI_BASE2	SUNXI_TWI2_BASE
#endif
#ifdef CONFIG_I2C3_ENABLE
#define CONFIG_I2C_MVTWSI_BASE3	SUNXI_TWI3_BASE
#endif
#ifdef CONFIG_I2C4_ENABLE
#define CONFIG_I2C_MVTWSI_BASE4	SUNXI_TWI4_BASE
#endif
#ifdef CONFIG_R_I2C_ENABLE
#define CONFIG_I2C_MVTWSI_BASE5 SUNXI_R_TWI_BASE
#endif

enum {
	SUNXI_PHY_I2C0 = 0,

	SUNXI_PHY_I2C1,

	SUNXI_PHY_I2C2,

	SUNXI_PHY_I2C3,

	SUNXI_PHY_I2C4,

	SUNXI_PHY_I2C5,

	SUNXI_PHY_R_I2C0,

	SUNXI_PHY_R_I2C1,

	/*The new i2c bus must be added before SUNXI_PHY_I2C_BUS_MAX*/
	SUNXI_PHY_I2C_BUS_MAX,

};

/* This is abp0-clk on sun4i/5i/7i / abp1-clk on sun6i/sun8i which is 24MHz */
#define CONFIG_SYS_TCLK		24000000
#define TWI_CONTROL_OFFSET             0x400
#define SUNXI_I2C_CONTROLLER             3

/* TWI Control Register Bit Fields & Masks, default value: 0x0000_0000*/
#define TWI_CTL_ACK     (0x1<<2)
#define TWI_CTL_INTFLG  (0x1<<3)
#define TWI_CTL_STP     (0x1<<4)
#define TWI_CTL_STA     (0x1<<5)
#define TWI_CTL_BUSEN   (0x1<<6)
#define TWI_CTL_INTEN   (0x1<<7)
#define TWI_LCR_WMASK   (TWI_CTL_STA|TWI_CTL_STP|TWI_CTL_INTFLG)


struct sunxi_twi_reg {
	volatile unsigned int addr;        /* slave address     */
	volatile unsigned int xaddr;       /* extend address    */
	volatile unsigned int data;        /* data              */
	volatile unsigned int ctl;         /* control           */
	volatile unsigned int status;      /* status            */
	volatile unsigned int clk;         /* clock             */
	volatile unsigned int srst;        /* soft reset        */
	volatile unsigned int eft;         /* enhanced future   */
	volatile unsigned int lcr;         /* line control      */
	volatile unsigned int dvfs;        /* dvfs control      */
};

#ifdef I2C_DEBUG
#define i2c_info(fmt...) printf("[i2c][info]: " fmt)
#define i2c_err(fmt...) printf("[i2c][err]: " fmt)
#else
#define i2c_info(fmt...)
#define i2c_err(fmt...) printf("[i2c][err]: " fmt)
#endif


void i2c_init_cpus(int speed, int slaveaddr);
int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len);
int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len);



#endif
