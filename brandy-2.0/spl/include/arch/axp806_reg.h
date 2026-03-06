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
 * Copyright (C) 2016 Allwinner.
 * wangwei <wangwei@allwinnertech.com>
 *
 * SUNXI AXP806  Driver
 *
 */

#ifndef __AXP806_REGS_H__
#define __AXP806_REGS_H__

#include <arch/axp.h>

/*PMIC chip id reg03:bit7-6  bit3-0 */
#define AXP806_CHIP_ID (0x40)

#ifdef CFG_SUNXI_TWI
#define AXP806_DEVICE_ADDR              400000
#define AXP806_RUNTIME_ADDR             (0x36)
#else
#define AXP806_DEVICE_ADDR              (0x745)
#define AXP806_RUNTIME_ADDR		(0x2d)
#endif
/*define AXP806 REGISTER*/
#define   AXP806_STARUP_SRC             			(0x00)
#define   AXP806_IC_TYPE         	   			(0x03)
#define   AXP806_DATA_BUFFER0        			(0x04)
#define   AXP806_DATA_BUFFER1        			(0x05)
#define   AXP806_DATA_BUFFER2        			(0x06)
#define   AXP806_DATA_BUFFER3        			(0x07)

#define   AXP806_OUTPUT_CTL1     	   			(0x10)
#define   AXP806_OUTPUT_CTL2     	   			(0x11)

#define   AXP806_DCAOUT_VOL                  	(0x12)
#define   AXP806_DCBOUT_VOL          			(0x13)
#define   AXP806_DCCOUT_VOL          			(0x14)
#define   AXP806_DCDOUT_VOL          			(0x15)
#define   AXP806_DCEOUT_VOL          			(0x16)
#define   AXP806_ALDO1OUT_VOL                    (0x17)
#define   AXP806_ALDO2OUT_VOL                    (0x18)
#define   AXP806_ALDO3OUT_VOL                    (0x19)
#define   AXP806_DCMOD_CTL1                  	(0x1A)
#define   AXP806_DCMOD_CTL2                  	(0x1B)
#define   AXP806_DCFREQ_SET                  	(0x1C)
#define   AXP806_DCMONITOR_CTL                  	(0x1D)
#define   AXP806_IRQ_SETTING                  	(0x1F)

#define   AXP806_BLDO1OUT_VOL                    (0x20)
#define   AXP806_BLDO2OUT_VOL                    (0x21)
#define   AXP806_BLDO3OUT_VOL                    (0x22)
#define   AXP806_BLDO4OUT_VOL                    (0x23)

#define   AXP806_CLDO1OUT_VOL                    (0x24)
#define   AXP806_CLDO2OUT_VOL                    (0x25)
#define   AXP806_CLDO3OUT_VOL                    (0x26)

#define   AXP806_VOFF_SETTING                    (0x31)
#define   AXP806_DIASBLE				(0x32)
#define   AXP806_POK_SETTING                     (0x36)

#define   AXP806_INTEN1              			(0x40)
#define   AXP806_INTEN2              			(0x41)
#define   AXP806_INTSTS1             			(0x48)
#define   AXP806_INTSTS2             			(0x49)


#define PMU_POWER_KEY_STATUS                  (0x49)
#define PMU_POWER_KEY_OFFSET                  (0)

int axp806_set_ddr_voltage(int set_vol);
int axp806_set_ddr4_2v5_voltage(int set_vol);
int axp806_set_pll_voltage(int set_vol);
int axp806_set_sys_voltage(int set_vol, int onoff);
int axp806_set_sys_voltage_ext(char *name, int set_vol, int onoff);
int axp806_axp_init(u8 power_mode);
int axp806_probe_power_key(void);
int axp806_get_power_source(void);
int axp806_reg_read(u8 addr, u8 *val);
int axp806_reg_write(u8 addr, u8 val);


#endif /* __AXP806_REGS_H__ */

