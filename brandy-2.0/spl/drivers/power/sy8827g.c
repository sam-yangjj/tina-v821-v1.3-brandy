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
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/sy8827g_reg.h>
#include <private_boot0.h>

static int ext_set_vol(char *name, int set_vol, int onoff);

static ext_contrl_info ext_ctrl_tbl[] = {
	{ "dcdc0", 6000, 13875, SY8827G_VSEL0, 0x3f, SY8827G_VSEL0, 7, 0,
	{ {6000, 13875, 125}, } },

	{ "dcdc1", 6000, 13875, SY8827G_VSEL1, 0x3f, SY8827G_VSEL1, 7, 0,
	{ {6000, 13875, 125}, } },
};

static ext_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i    = 0;
	int size = ARRAY_SIZE(ext_ctrl_tbl);
	for (i = 0; i < size; i++) {
		if (!strncmp(name, ext_ctrl_tbl[i].name,
			     strlen(ext_ctrl_tbl[i].name))) {
			break;
		}
	}
	if (i >= size) {
		return NULL;
	}
	return (ext_ctrl_tbl + i);
}

static int ext_set_vol(char *name, int set_vol, int onoff)
{
	u8 reg_value, i;
	ext_contrl_info *p_item = NULL;
	u8 base_step		= 0;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}
	set_vol = set_vol * 10;

	if ((set_vol > 0) && (p_item->min_vol)) {
		if (set_vol < p_item->min_vol) {
			set_vol = p_item->min_vol;
		} else if (set_vol > p_item->max_vol) {
			set_vol = p_item->max_vol;
		}
		if (pmic_bus_read(SY8827G_RUNTIME_ADDR, p_item->cfg_reg_addr,
				  &reg_value)) {
			return -1;
		}

		reg_value &= ~p_item->cfg_reg_mask;

		for (i = 0; p_item->ext_step_tbl[i].step_max_vol != 0; i++) {
			if ((set_vol > p_item->ext_step_tbl[i].step_max_vol) &&
				(set_vol < p_item->ext_step_tbl[i+1].step_min_vol)) {
				set_vol = p_item->ext_step_tbl[i].step_max_vol;
			}
			if (p_item->ext_step_tbl[i].step_max_vol >= set_vol) {
				reg_value |= ((base_step + ((set_vol - p_item->ext_step_tbl[i].step_min_vol)/
					p_item->ext_step_tbl[i].step_val)) << p_item->reg_addr_offest);
				if (p_item->ext_step_tbl[i].regation) {
					u8 reg_value_temp = (~reg_value & p_item->cfg_reg_mask);
					reg_value &= ~p_item->cfg_reg_mask;
					reg_value |= reg_value_temp;
				}
				break;
			} else {
				base_step += ((p_item->ext_step_tbl[i].step_max_vol -
					p_item->ext_step_tbl[i].step_min_vol + p_item->ext_step_tbl[i].step_val) /
					p_item->ext_step_tbl[i].step_val);
			}
		}

		if (pmic_bus_write(SY8827G_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(SY8827G_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(SY8827G_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	pmic_bus_read(SY8827G_RUNTIME_ADDR, p_item->ctrl_reg_addr, &reg_value);
	return 0;

}

int sy8827g_set_pll_voltage(int set_vol)
{
	return ext_set_vol("dcdc0", set_vol, 1);
}


static int sy8827g_set_necessary_reg(void)
{
	return 0;
}

int sy8827g_ext_init(u8 power_mode)
{
	u8 reg_value;

	if (pmic_bus_init(SY8827G_DEVICE_ADDR, SY8827G_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(SY8827G_RUNTIME_ADDR, SY8827G_ID1, &reg_value)) {
		pmu_err("bus read error\n");
		return -1;
	}

	reg_value &= 0xE0;

	if (reg_value == 0x80) {
		/* ext type SY8827G */
		printf("EXT: SY8827G\n");
		sy8827g_set_necessary_reg();
		return SY8827G_CHIP_ID;
	}

	printf("unknow EXT:%x\n", reg_value);
	return -1;
}
