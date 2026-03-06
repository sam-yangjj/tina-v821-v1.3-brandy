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
#include <arch/axp1530_reg.h>
#include <private_boot0.h>
#include <cache_align.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);

static axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdc1", 500, 3400, AXP1530_DC1OUT_VOL, 0x7f, AXP1530_OUTPUT_POWER_ON_OFF_CTL, 0, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1600, 3400, 100}, } },

	{ "dcdc2", 500, 1540, AXP1530_DC2OUT_VOL, 0x7f, AXP1530_OUTPUT_POWER_ON_OFF_CTL, 1, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc3", 500, 1840, AXP1530_DC3OUT_VOL, 0x7f, AXP1530_OUTPUT_POWER_ON_OFF_CTL, 2, 0,
	{ {500, 1200, 10}, {1220, 1840, 20}, } },

	{ "aldo1", 500, 3500, AXP1530_ALDO1OUT_VOL, 0x1f, AXP1530_OUTPUT_POWER_ON_OFF_CTL, 3, 0,
	{ {500, 3500, 100}, } },

	{ "dldo1", 500, 3500, AXP1530_DLDO1OUT_VOL, 0x1f, AXP1530_OUTPUT_POWER_ON_OFF_CTL, 4, 0,
	{ {500, 3500, 100}, } },

};


int axp1530_probe_power_key(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_IRQ_STATUS, &reg_value)) {
		return -1;
	}
	reg_value &= (0x03 << 4);
	if (reg_value) {
		if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_IRQ_STATUS,
				   reg_value)) {
			return -1;
		}
	}
	return (reg_value >> 4) & 3;

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
		if (pmic_bus_read(AXP1530_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP1530_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}


static int pmu_get_vol(char *name)
{

	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step1 = 0;
	u8 base_step2 = 0;
	int vol;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, p_item->ctrl_reg_addr,
		&reg_value)) {
		return -1;
	}
	if (!(reg_value & (0x01 << p_item->ctrl_bit_ofs))) {
		return 0;
	}

	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, p_item->cfg_reg_addr,
		&reg_value)) {
		return -1;
	}
	reg_value &= p_item->cfg_reg_mask;
	reg_value >>= p_item->reg_addr_offest;
	for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
		base_step1 += ((p_item->axp_step_tbl[i].step_max_vol -
			p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
			p_item->axp_step_tbl[i].step_val);
		if (reg_value < base_step1) {
			vol =  (reg_value - base_step2) * p_item->axp_step_tbl[i].step_val +
				p_item->axp_step_tbl[i].step_min_vol;
			return vol;
		}
		base_step2 += ((p_item->axp_step_tbl[i].step_max_vol -
			p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
			p_item->axp_step_tbl[i].step_val);
	}
	return -1;
}



int axp1530_set_ddr_voltage(int set_vol)
{
	int temp_vol, src_vol = pmu_get_vol("dcdc3");
	if (src_vol > set_vol) {
		for (temp_vol = src_vol; temp_vol >= set_vol; temp_vol -= 50) {
			pmu_set_vol("dcdc3", temp_vol, 1);
		}
	} else if (src_vol < set_vol) {
		for (temp_vol = src_vol; temp_vol <= set_vol; temp_vol += 50) {
			pmu_set_vol("dcdc3", temp_vol, 1);
		}
	}
	udelay(230);
	return 0;
}

int axp1530_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("aldo1", set_vol, 1);
}

int axp1530_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc2", set_vol, 1);
}

int axp1530_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc1", set_vol, onoff);
}

int axp1530_set_cpu_voltage_ext(int set_vol)
{
	int ret1 = 0, ret2 = 0;
	u8 val;

	pmic_bus_read(AXP1530_RUNTIME_ADDR, 0x13, &val);

	val |= 0x80;
	pmic_bus_write(AXP1530_RUNTIME_ADDR, 0x14, val);

	pmic_bus_write(AXP1530_RUNTIME_ADDR, 0x1D, 0x1E);
	pmic_bus_write(AXP1530_RUNTIME_ADDR, 0x22, 0x00);

	ret2 = pmu_set_vol("dcdc2", set_vol, 1);
	mdelay(1);

	ret1 = pmu_set_vol("dcdc1", set_vol, 1);
	udelay(500);

	pmic_bus_write(AXP1530_RUNTIME_ADDR, 0x22, 0x02);

	return (ret1 && ret2);
}

static int axp1530_necessary_reg_enable(void)
{
	__attribute__((unused)) u8 reg_value;
#ifdef CFG_AXP1530A_NECESSARY_REG_ENABLE
	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_WRITE_LOCK, &reg_value))
		return -1;
	reg_value |= 0x5;
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_WRITE_LOCK, reg_value))
		return -1;

	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_ERROR_MANAGEMENT, &reg_value))
		return -1;
	reg_value |= 0x8;
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_ERROR_MANAGEMENT, reg_value))
		return -1;

	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_DCDC_DVM_PWM_CTL, &reg_value))
		return -1;
	reg_value |= (0x1 << 5);
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_DCDC_DVM_PWM_CTL, reg_value))
		return -1;
#endif
	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_DC2OUT_VOL, &reg_value))
		return -1;
	reg_value |= (0x1 << 7);
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_DC2OUT_VOL, reg_value))
		return -1;

	/* Set over temperature shutdown functtion */
	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_POWER_DOMN_SEQUENCE, &reg_value))
		return -1;
	reg_value |= (0x1 << 1);
	if (pmic_bus_write(AXP1530_RUNTIME_ADDR, AXP1530_POWER_DOMN_SEQUENCE, reg_value))
		return -1;
	return 0;
}

int axp1530_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP1530_RUNTIME_ADDR, addr, val);
}

int axp1530_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP1530_RUNTIME_ADDR, addr, val);
}


int axp1530_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP1530_DEVICE_ADDR, AXP1530_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP1530_RUNTIME_ADDR, AXP1530_VERSION, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if (pmu_type == AXP1530_CHIP_ID || pmu_type == AXP313A_CHIP_ID || pmu_type == AXP313B_CHIP_ID || pmu_type == AXP323_CHIP_ID) {
		/* pmu type AXP1530 */
		axp1530_necessary_reg_enable();
		printf("PMU: AXP1530\n");
		return AXP1530_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}

