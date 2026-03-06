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
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp8191_reg.h>
#include <private_boot0.h>
#include <cache_align.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);
static axp_contrl_info axp_ctrl_tbl[] = {
/*
	{ "dcdc1", 1000, 3800, AXP8191_DC1OUT_VOL, 0x1f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 0, 0,
	{ {1000, 3800, 100}, } },
*/

	{ "dcdc2", 500, 1540, AXP8191_DC2OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 1, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc3", 500, 1540, AXP8191_DC3OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 2, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc4", 500, 1540, AXP8191_DC4OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 3, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

/*
	{ "dcdc5", 500, 1540, AXP8191_DC5OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 4, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1800, 2400, 20}, {2440, 2760, 100}, } },

*/
	{ "dcdc6", 500, 2760, AXP8191_DC6OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 5, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1800, 2400, 20}, {2440, 2760, 40}, } },

	{ "dcdc7", 500, 1840, AXP8191_DC7OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 6, 0,
	{ {500, 1200, 10}, {1220, 1840, 20} } },

	{ "dcdc8", 500, 3400, AXP8191_DC8OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL1, 7, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1900, 3400, 100}, } },

/*
	{ "dcdc9", 500, 3400, AXP8191_DC9OUT_VOL, 0x7f, AXP8191_DCDC_POWER_ON_OFF_CTL2, 0, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1900, 3400, 100}, } },

	{ "aldo1", 500, 3400, AXP8191_ALDO1OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 0, 0,
	{ {500, 3400, 100}, } },

	{ "aldo2", 500, 3400, AXP8191_ALDO2OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 1, 0,
	{ {500, 3400, 100}, } },

	{ "aldo3", 500, 3400, AXP8191_ALDO3OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 2, 0,
	{ {500, 3400, 100}, } },

	{ "aldo4", 500, 3400, AXP8191_ALDO4OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 3, 0,
	{ {500, 3400, 100}, } },

	{ "aldo5", 500, 3400, AXP8191_ALDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 4, 0,
	{ {500, 3400, 100}, } },

	{ "aldo6", 500, 3400, AXP8191_ALDO6OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 5, 0,
	{ {500, 3400, 100}, } },


	{ "bldo1", 500, 3400, AXP8191_BLDO1OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 6, 0,
	{ {500, 3400, 100}, } },

	{ "bldo2", 500, 3400, AXP8191_BLDO2OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL1, 7, 0,
	{ {500, 3400, 100}, } },

	{ "bldo3", 500, 3400, AXP8191_BLDO3OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 0, 0,
	{ {500, 3400, 100}, } },

	{ "bldo4", 500, 3400, AXP8191_BLDO4OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 1, 0,
	{ {500, 3400, 100}, } },

	{ "bldo5", 500, 3400, AXP8191_BLDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 2, 0,
	{ {500, 3400, 100}, } },


	{ "cldo1", 500, 3400, AXP8191_CLDO1OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 3, 0,
	{ {500, 3400, 100}, } },

	{ "cldo2", 500, 3400, AXP8191_CLDO2OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 4, 0,
	{ {500, 3400, 100}, } },

	{ "cldo3", 500, 3400, AXP8191_CLDO3OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 5, 0,
	{ {500, 3400, 100}, } },

	{ "cldo4", 500, 3400, AXP8191_CLDO4OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 6, 0,
	{ {500, 3400, 100}, } },

	{ "cldo5", 500, 3400, AXP8191_CLDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL2, 7, 0,
	{ {500, 3400, 100}, } },


	{ "dldo1", 500, 3400, AXP8191_DLDO1OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 0, 0,
	{ {500, 3400, 100}, } },

	{ "dldo2", 500, 3400, AXP8191_DLDO2OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 1, 0,
	{ {500, 3400, 100}, } },

	{ "dldo3", 500, 3400, AXP8191_DLDO3OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 2, 0,
	{ {500, 3400, 100}, } },

	{ "dldo4", 500, 3400, AXP8191_DLDO4OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 3, 0,
	{ {500, 3400, 100}, } },

	{ "dldo5", 500, 3400, AXP8191_DLDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 4, 0,
	{ {500, 3400, 100}, } },

	{ "dldo5", 500, 3400, AXP8191_DLDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 5, 0,
	{ {500, 3400, 100}, } },
*/

	{ "eldo1", 500, 1500, AXP8191_ELDO1OUT_VOL, 0x3f, AXP8191_LDO_POWER_ON_OFF_CTL3, 6, 0,
	{ {500, 1500, 25}, } },
/*
	{ "eldo2", 500, 1500, AXP8191_ELDO2OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 7, 0,
	{ {500, 1500, 100}, } },

	{ "eldo3", 500, 1500, AXP8191_ELDO3OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 0, 0,
	{ {500, 1500, 100}, } },

	{ "eldo4", 500, 1500, AXP8191_ELDO4OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL3, 1, 0,
	{ {500, 1500, 100}, } },

	{ "eldo5", 500, 1500, AXP8191_ELDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL4, 2, 0,
	{ {500, 1500, 100}, } },

	{ "eldo5", 500, 1500, AXP8191_ELDO5OUT_VOL, 0x1f, AXP8191_LDO_POWER_ON_OFF_CTL4, 3, 0,
	{ {500, 1500, 100}, } },
*/
};

#define PMU_POWER_KEY_OFFSET 0x3

int axp8191_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_IRQ_STATUS2,
			  &reg_value)) {
		return -1;
	}

	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_IRQ_STATUS2,
				   reg_value)) {
			return -1;
		}
	}

	return (reg_value >> PMU_POWER_KEY_OFFSET) & 3;
}

int axp8191_get_power_source(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_POWER_ON_OFF_SOURCE_INDIVATION, &reg_value)) {
		return -1;
	}

	if (reg_value & (1 << 0)) {
		return AXP_BOOT_SOURCE_VBUS_USB;
	} else if (reg_value & (1 << 1)) {
		return AXP_BOOT_SOURCE_BUTTON;
	} else if (reg_value & (1 << 2)) {
		return AXP_BOOT_SOURCE_IRQ_LOW;
	}

	return -1;
}

static axp_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i    = 0;
	int size = ARRAY_SIZE(axp_ctrl_tbl);
	for (i = 0; i < size; i++) {
		if (!strncmp(name, axp_ctrl_tbl[i].name,
			     strlen(axp_ctrl_tbl[i].name))) {
			break;
		}
	}
	if (i >= size) {
		return NULL;
	}
	return (axp_ctrl_tbl + i);
}

static int pmu_set_vol(char *name, int set_vol, int onoff)
{
	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step		= 0;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if ((set_vol > 0) && (p_item->min_vol)) {
		if (set_vol < p_item->min_vol) {
			set_vol = p_item->min_vol;
		} else if (set_vol > p_item->max_vol) {
			set_vol = p_item->max_vol;
		}
		if (pmic_bus_read(AXP8191_RUNTIME_ADDR, p_item->cfg_reg_addr,
				  &reg_value)) {
			return -1;
		}

		reg_value &= ~p_item->cfg_reg_mask;

		for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
			if ((set_vol > p_item->axp_step_tbl[i].step_max_vol) &&
				(set_vol < p_item->axp_step_tbl[i+1].step_min_vol)) {
				set_vol = p_item->axp_step_tbl[i].step_max_vol;
			}
			if (p_item->axp_step_tbl[i].step_max_vol >= set_vol) {
				reg_value |= ((base_step + ((set_vol - p_item->axp_step_tbl[i].step_min_vol)/
					p_item->axp_step_tbl[i].step_val)) << p_item->reg_addr_offest);
				if (p_item->axp_step_tbl[i].regation) {
					u8 reg_value_temp = (~reg_value & p_item->cfg_reg_mask);
					reg_value &= ~p_item->cfg_reg_mask;
					reg_value |= reg_value_temp;
				}
				break;
			} else {
				base_step += ((p_item->axp_step_tbl[i].step_max_vol -
					p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
					p_item->axp_step_tbl[i].step_val);
			}
		}

		if (pmic_bus_write(AXP8191_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP8191_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

int axp8191_set_pll_voltage(int set_vol)
{
	if (pmu_set_vol("dcdc3", set_vol, 1)) {
		return -1;
	}
	if (pmu_set_vol("dcdc4", set_vol, 1)) {
		return -1;
	}
	return 0;
}

int axp8191_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc8", set_vol, 1);
}

/*
int axp8191_set_ddr4_2v5_voltage(int set_vol)
{
	return pmu_set_vol("aldo4", set_vol, 1);
}

int axp8191_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("dldo6", set_vol, 1);
}
*/

int axp8191_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc2", set_vol, onoff);
}

int axp8191_set_sys_voltage_ext(char *name, int set_vol, int onoff)
{
	return pmu_set_vol(name, set_vol, onoff);
}

static int axp8191_necessary_reg_enable(void)
{
	u8 pmu_value;
	u8 i;

	pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_DCDC_POWER_ON_OFF_CTL1, &pmu_value);
	pmu_value |= 0x08;
	pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_DCDC_POWER_ON_OFF_CTL1, pmu_value);

	/* set dcdc2-9 dvm */
	for (i = 0; i <= (AXP8191_DC9OUT_VOL - AXP8191_DC2OUT_VOL); i++) {
		pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_DC2OUT_VOL + i, &pmu_value);
		pmu_value |= 0x80;
		pmic_bus_write(AXP8191_RUNTIME_ADDR, AXP8191_DC2OUT_VOL + i, pmu_value);
	}

	return 0;
}

int axp8191_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP8191_DEVICE_ADDR, AXP8191_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP8191_RUNTIME_ADDR, AXP8191_CHIP_ID, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	if (pmu_type == 0x03) {
		/* pmu type AXP8191 */
		axp8191_necessary_reg_enable();
		printf("PMU: AXP8191\n");
		return AXP8191_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
