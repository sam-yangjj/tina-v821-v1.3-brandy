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
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Sunxi PMIC bus access helpers
 *
 * The axp152 & axp209 use an i2c bus, the axp221 uses the p2wi bus and the
 * axp223 uses the rsb bus, these functions abstract this.
 *
 */

#include <common.h>
#include <arch/rsb.h>
#ifdef CFG_SUNXI_TWI
#include <arch/i2c.h>
#endif
#include <arch/pmic_bus.h>


#define AXP152_I2C_ADDR			0x30

#define AXP209_I2C_ADDR			0x34

#define AXP221_CHIP_ADDR		0x68
#define AXP221_CTRL_ADDR		0x3e
#define AXP221_INIT_DATA		0x3e

#define AXP858_DEVICE_ADDR              0x745
/* AXP818 device and runtime addresses are same as AXP223 */
#define AXP223_DEVICE_ADDR		0x3a3
#define AXP223_RUNTIME_ADDR		0x2d


int pmic_bus_init(u32 device_addr, u32 runtime_addr)
{
	/* This cannot be 0 because it is used in SPL before BSS is ready */
	/*static int needs_init = 1;*/
	__maybe_unused int ret = 0;

	/*if (!needs_init)
		return 0;
	*/
# ifdef CONFIG_MACH_SUN6I
	p2wi_init();
	ret = p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR, AXP221_CTRL_ADDR,
				       AXP221_INIT_DATA);
# elif defined CFG_SUNXI_TWI
	i2c_init_cpus(device_addr, runtime_addr);
# else
	ret = rsb_init();
	if (ret)
		return ret;
	ret = rsb_set_device_address(device_addr, runtime_addr);
# endif
	if (ret)
		return ret;

	/*needs_init = 0;*/
	return 0;
}

int pmic_bus_read(u32 runtime_addr, u8 reg, u8 *data)
{
#if defined CFG_SUNXI_TWI
	return i2c_read(runtime_addr, reg, 1, data, 1);
#else
	return rsb_read(runtime_addr, reg, data);
#endif
}

int pmic_bus_write(u32 runtime_addr, u8 reg, u8 data)
{
# ifdef CONFIG_MACH_SUN6I
	return p2wi_write(reg, data);
# elif defined CONFIG_MACH_SUN8I_R40 || defined CFG_SUNXI_TWI
	return i2c_write(runtime_addr, reg, 1, &data, 1);
# else
	return rsb_write(runtime_addr, reg, data);
# endif
}

int pmic_bus_setbits(u32 runtime_addr, u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(runtime_addr, reg, &val);
	if (ret)
		return ret;

	val |= bits;
	return pmic_bus_write(runtime_addr, reg, val);
}

int pmic_bus_clrbits(u32 runtime_addr, u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(runtime_addr, reg, &val);
	if (ret)
		return ret;

	val &= ~bits;
	return pmic_bus_write(runtime_addr, reg, val);
}
