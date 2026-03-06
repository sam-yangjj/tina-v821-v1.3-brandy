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
 * Copyright (C) 2019 Allwinner.
 * weidonghui <weidonghui@allwinnertech.com>
 *
 */

#include <common.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <arch/axp.h>
#ifdef CFG_AXP858_POWER
#include <arch/axp858_reg.h>
#endif
#ifdef CFG_AXP81X_POWER
#include <arch/axp81x_reg.h>
#endif
#ifdef CFG_AXP806_POWER
#include <arch/axp806_reg.h>
#endif
#ifdef CFG_AXP809_POWER
#include <arch/axp809_reg.h>
#endif
#ifdef CFG_AXP2101_POWER
#include <arch/axp2101_reg.h>
#endif
#ifdef CFG_AXP2202_POWER
#include <arch/axp2202_reg.h>
#endif
#ifdef CFG_AXP152_POWER
#include <arch/axp152_reg.h>
#endif
#ifdef CFG_AXP1530_POWER
#include <arch/axp1530_reg.h>
#endif
#ifdef CFG_AXP221_POWER
#include <arch/axp221_reg.h>
#endif
#ifdef CFG_AXP8191_POWER
#include <arch/axp8191_reg.h>
#endif



static int pmu_chip_id;

int get_power_source(void)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_get_power_source();
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_get_power_source();
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_get_power_source();
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_get_power_source();
#endif
	default:
		return -1;
	}
}

int set_power_reset(void)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_power_reset();
#endif
	default:
		return -1;
	}
}

int probe_power_key(void)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_probe_power_key();
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_probe_power_key();
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_probe_power_key();
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_probe_power_key();
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_probe_power_key();
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_probe_power_key();
#endif

#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_probe_power_key();
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_probe_power_key();
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_probe_power_key();
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_probe_power_key();
#endif
	default:
		return -1;
	}
}

int set_ddr_voltage(int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_ddr_voltage(set_vol);
#endif

#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_set_ddr_voltage(set_vol);
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_ddr_voltage(set_vol);
#endif
	default:
		if (get_power_mode() == 1)
			return 0;
		return -1;
	}
}

int set_ddr4_2v5_voltage(int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_ddr4_2v5_voltage(set_vol);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_ddr4_2v5_voltage(set_vol);
#endif
	default:
		return -1;
	}
}

int set_sys_voltage(int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_sys_voltage(set_vol, 1);
#endif

#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_set_sys_voltage(set_vol, 1);
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_sys_voltage(set_vol, 1);
#endif
	default:
		return -1;
	}
}

int set_sys_voltage_ext(char *name, int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_set_sys_voltage_ext(name, set_vol, 1);
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_sys_voltage_ext(name, set_vol, 1);
#endif
	default:
		return -1;
	}
}

int close_sys_voltage_ext(char *name, int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_sys_voltage_ext(name, set_vol, 0);
#endif
	default:
		return -1;
	}
}

int set_camera_voltage_ext(char *name, int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_sys_voltage_ext(name, set_vol, 1);
#endif
	default:
		return -1;
	}
}

int set_ddr_voltage_ext(char *name, int set_vol, int on)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_sys_voltage_ext(name, set_vol, on);
#endif
	default:
		return -1;
	}
}

int set_voltage_ext(char *name, int set_vol, int on)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_set_sys_voltage_ext(name, set_vol, on);
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_sys_voltage_ext(name, set_vol, on);
#endif
	default:
		return -1;
	}
}

int set_pll_voltage(int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_set_pll_voltage(set_vol);
#endif
#ifdef CFG_AXP8191_POWER
	case AXP8191_CHIP_ID:
		return axp8191_set_pll_voltage(set_vol);
#endif
	default:
		return -1;
	}
}

int set_efuse_voltage(int set_vol)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_set_efuse_voltage(set_vol);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_set_efuse_voltage(set_vol);
#endif
#ifdef CFG_AXP2202_POWER
	case AXP2202_CHIP_ID:
		return axp2202_set_efuse_voltage(set_vol);
#endif

#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_set_efuse_voltage(set_vol);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_set_efuse_voltage(set_vol);
#endif
	default:
		return -1;
	}
}

int axp_reg_read(u8 addr, u8 *val)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_reg_read(addr, val);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_reg_read(addr, val);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_reg_read(addr, val);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_reg_read(addr, val);
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_reg_read(addr, val);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_reg_read(addr, val);
#endif
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_reg_read(addr, val);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_reg_read(addr, val);
#endif
	default:
		return -1;
	}
}

int axp_reg_write(u8 addr, u8 val)
{
	switch (pmu_chip_id) {
#ifdef CFG_AXP152_POWER
	case AXP152_CHIP_ID:
		return axp152_reg_write(addr, val);
#endif
#ifdef CFG_AXP1530_POWER
	case AXP1530_CHIP_ID:
		return axp1530_reg_write(addr, val);
#endif
#ifdef CFG_AXP2101_POWER
	case AXP2101_CHIP_ID:
		return axp2101_reg_write(addr, val);
#endif
#ifdef CFG_AXP806_POWER
	case AXP806_CHIP_ID:
		return axp806_reg_write(addr, val);
#endif
#ifdef CFG_AXP809_POWER
	case AXP809_CHIP_ID:
		return axp809_reg_write(addr, val);
#endif
#ifdef CFG_AXP81X_POWER
	case AXP81X_CHIP_ID:
		return axp81X_reg_write(addr, val);
#endif
#ifdef CFG_AXP858_POWER
	case AXP858_CHIP_ID:
		return axp858_reg_write(addr, val);
#endif
#ifdef CFG_AXP221_POWER
	case AXP221_CHIP_ID:
		return axp221_reg_write(addr, val);
#endif

	default:
		return -1;
	}
}


int axp_init(u8 power_mode)
{
	pmu_chip_id = -1;
#ifdef CFG_AXP858_POWER
	if (axp858_axp_init(power_mode) == AXP858_CHIP_ID) {
		return (pmu_chip_id = AXP858_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP81X_POWER
		if (axp81X_axp_init(power_mode) == AXP81X_CHIP_ID) {
		return (pmu_chip_id = AXP81X_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP806_POWER
		if (axp806_axp_init(power_mode) == AXP806_CHIP_ID) {
		return (pmu_chip_id = AXP806_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP809_POWER
		if (axp809_axp_init(power_mode) == AXP809_CHIP_ID) {
		return (pmu_chip_id = AXP809_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP2101_POWER
		if (axp2101_axp_init(power_mode) == AXP2101_CHIP_ID) {
		return (pmu_chip_id = AXP2101_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP2202_POWER
		if (axp2202_axp_init(power_mode) == AXP2202_CHIP_ID) {
		return (pmu_chip_id = AXP2202_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP152_POWER
		if (axp152_axp_init(power_mode) == AXP152_CHIP_ID) {
		return (pmu_chip_id = AXP152_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP1530_POWER
		if (axp1530_axp_init(power_mode) == AXP1530_CHIP_ID) {
		return (pmu_chip_id = AXP1530_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP221_POWER
		if (axp221_axp_init(power_mode) == AXP221_CHIP_ID) {
			pmu_chip_id = AXP221_CHIP_ID;
			printf("pmu_chip_id = %d\n", pmu_chip_id);
		return (pmu_chip_id = AXP221_CHIP_ID);
	} else
#endif
#ifdef CFG_AXP8191_POWER
		if (axp8191_axp_init(power_mode) == AXP8191_CHIP_ID) {
			pmu_chip_id = AXP8191_CHIP_ID;
			printf("pmu_chip_id = %d\n", pmu_chip_id);
		return (pmu_chip_id = AXP8191_CHIP_ID);
	} else
#endif
		return -1;
}

int get_power_mode(void)
{
	#ifdef CFG_SUNXI_FES
		return fes1_head.prvt_head.power_mode;
	#elif CFG_SUNXI_SBOOT
		return toc0_config->power_mode;
	#else
		return BT0_head.prvt_head.power_mode;
	#endif
}

int get_pmu_exist(void)
{
	/* ensure whether pmu is exist or not */
	/* If have pmu: return that pmu's id ,If not: return -1*/
	return pmu_chip_id;
}
