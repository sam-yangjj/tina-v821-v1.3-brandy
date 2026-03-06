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
#include <asm/io.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <asm/io.h>
#include <arch/watchdog.h>
#include <arch/physical_key.h>

#define CLOCK_DEBUG  0

#ifdef CFG_GPADC_KEY
int sunxi_gpadc_init(void);
#endif

typedef struct {
	u32 pll_en;
	u32 pll_ldo_en;
	u32 pll_lock_en;
	u32 pll_output_gate;
	u32 pll_n;
	u32 pll_m;
} ccu_pll_peri_cfg_info_t;

typedef struct {
	u32 pll_en;
	u32 pll_ldo_en;
	u32 pll_lock_en;
	u32 pll_output_gate;
	u32 pll_n;
	u32 pll_m;
	u32 pll_d;
} ccu_pll_general_cfg_info_t;

int pll_timeout(u32 time_cnt)
{
	u32 ret = 1;
	do {
		if (time_cnt == 0) {
			return ret;
		}

	} while (time_cnt--);
	return ret = 0;
}

typedef struct {
	u8 name[8];
	u32 addr;
	u32 value;
} sys_clock_info_t;

sys_clock_info_t sys_clock_info[] = {
	{
		.name  = "PLL_CPU",
		.addr  = CCMU_PLL_CPUX_CTRL_REG,
		.value = 0,
	},
	{
		.name  = "PLL_PERI",
		.addr  = CCMU_PLL_PERI0_CTRL_REG,
		.value = 0,
	},
	{
		.name  = "PLL_VE",
		.addr  = CCMU_PLL_VIDEO_CTRL_REG,
		.value = 0,
	},
	{
		.name  = "PLL_DDR",
		.addr  = CCMU_PLL_DDR0_CTRL_REG,
		.value = 0,
	},
	{
		.name  = "AHB",
		.addr  = CCMU_AHB_CLK_REG,
		.value = 0,
	},
	{
		.name  = "APB",
		.addr  = CCMU_APB_CLK_REG,
		.value = 0,
	},
	{
		.name  = "E90X",
		.addr  = CCMU_E90X_CLK_REG,
		.value = 0,
	},
	{
		.name  = "A27",
		.addr  = CCMU_A27_CLK_REG,
		.value = 0,
	},
};

#if CLOCK_DEBUG
void get_sys_clock_info(void)
{
	for (int i = 0; i < sizeof(sys_clock_info) / sizeof(sys_clock_info[0]);
	     i++) {
		sys_clock_info[i].value = readl(sys_clock_info[i].addr);
	}
}

void dump_sys_clock_info(void)
{
	for (int i = 0; i < sizeof(sys_clock_info) / sizeof(sys_clock_info[0]);
	     i++) {
		printf("%s:0x%x:0x%x\n", sys_clock_info[i].name,
		       sys_clock_info[i].addr, sys_clock_info[i].value);
	}
}
#endif

#define PLL_D_MASK   (PLL_CPU_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK)
#define PLL_M_MASK   (PLL_CPU_CTRL_REG_PLL_M_OFFSET)
#define PLL_N_MASK   (PLL_CPU_CTRL_REG_PLL_N_CLEAR_MASK)
#define PLL_D_OFFSET (PLL_CPU_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK)
#define PLL_N_OFFSET (PLL_CPU_CTRL_REG_PLL_N_OFFSET)

#define PLL_OUTPUT_GATE_MASK (PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_CLEAR_MASK)
#define PLL_OUTPUT_GATE_Enable                                                 \
	(PLL_CPU_CTRL_REG_PLL_EN_ENABLE << PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_OFFSET)
#define PLL_OUTPUT_GATE_Disable                                                \
	(PLL_CPU_CTRL_REG_PLL_EN_ENABLE << PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_OFFSET)
#define PLL_EN_MASK (PLL_CPU_CTRL_REG_PLL_EN_CLEAR_MASK)
#define PLL_Enable                                                             \
	(PLL_CPU_CTRL_REG_PLL_EN_ENABLE << PLL_CPU_CTRL_REG_PLL_EN_OFFSET)
#define PLL_Disable                                                            \
	(PLL_CPU_CTRL_REG_PLL_EN_DISABLE << PLL_CPU_CTRL_REG_PLL_EN_OFFSET)
#define PLL_LDO_MASK (PLL_CPU_CTRL_REG_PLL_LDO_EN_CLEAR_MASK)
#define PLL_LDO_Enable                                                         \
	(PLL_CPU_CTRL_REG_PLL_LDO_EN_ENABLE                                    \
	 << PLL_CPU_CTRL_REG_PLL_LDO_EN_OFFSET)
#define PLL_LDO_Disable                                                        \
	(PLL_CPU_CTRL_REG_PLL_LDO_EN_DISABLE                                   \
	 << PLL_CPU_CTRL_REG_PLL_LDO_EN_OFFSET)
#define PLL_LOCK_EN_MASK (PLL_CPU_CTRL_REG_LOCK_ENABLE_CLEAR_MASK)
#define PLL_LOCK_EN_Enable                                                        \
	(PLL_CPU_CTRL_REG_LOCK_ENABLE_ENABLE                                   \
	 << PLL_CPU_CTRL_REG_LOCK_ENABLE_OFFSET)
#define PLL_LOCK_EN_Disable                                                       \
	(PLL_CPU_CTRL_REG_LOCK_ENABLE_DISABLE                                  \
	 << PLL_CPU_CTRL_REG_LOCK_ENABLE_OFFSET)
#define PLL_LOCK_MASK (PLL_CPU_CTRL_REG_LOCK_CLEAR_MASK)
#define PLL_LOCK_TIME_MASK (PLL_CPU_CTRL_REG_PLL_LOCK_TIME_CLEAR_MASK)

#ifndef CFG_SUNXI_PMBOOT
__maybe_unused static void set_pll_general(u32 pll_addr, u32 en, u32 output_gate_en, u32 pll_d,
			    u32 pll_d_mask, u32 pll_d_off, u32 pll_n)
{
	u32 check_cnt = 0;
	u32 check_retry = 0;

	if (en) {
		setbits_le32(pll_addr, PLL_LDO_MASK);
		clrbits_le32(pll_addr, PLL_EN_MASK | PLL_OUTPUT_GATE_MASK | PLL_LOCK_EN_MASK);

		check_retry = 0;
		while (check_retry < 100) {
			if (!(readl(pll_addr) & (PLL_EN_MASK | PLL_OUTPUT_GATE_MASK | PLL_LOCK_EN_MASK))) {
				check_cnt++;
			}
			check_retry++;
			udelay(3);
		}
		if (readl(pll_addr) & (PLL_EN_MASK | PLL_OUTPUT_GATE_MASK | PLL_LOCK_EN_MASK)) {
			printf("pll open prepare fail\n");
		}

		if (pll_addr == CCMU_PLL_CPUX_CTRL_REG) {
			if (aw_get_hosc_freq() == HOSC_FREQ_40M) {
				/* hosc is 40M */
				clrsetbits_le32(pll_addr, PLL_LOCK_TIME_MASK, 0x03 << PLL_CPU_CTRL_REG_PLL_LOCK_TIME_OFFSET);
			} else {
				/* hosc is 24M */
				clrsetbits_le32(pll_addr, PLL_LOCK_TIME_MASK, 0x02 << PLL_CPU_CTRL_REG_PLL_LOCK_TIME_OFFSET);
			}
		}
		clrsetbits_le32(pll_addr, pll_d_mask, pll_d << pll_d_off);
		clrsetbits_le32(pll_addr, PLL_N_MASK, pll_n << PLL_N_OFFSET);
		setbits_le32(pll_addr, PLL_EN_MASK);
		setbits_le32(pll_addr, PLL_LOCK_EN_MASK);

		check_cnt = 0;
		check_retry = 0;
		while (check_cnt < 3 && check_retry < 100) {
			if (readl(pll_addr) & PLL_LOCK_MASK) {
				check_cnt++;
			}
			check_retry++;
			udelay(3);
		}
		if (check_cnt == 3) {
			udelay(20);
			setbits_le32(pll_addr, PLL_OUTPUT_GATE_MASK);
		} else {
			printf("pll open fail\n");
		}
	} else {
		clrbits_le32(pll_addr, PLL_LDO_MASK | PLL_EN_MASK | PLL_OUTPUT_GATE_MASK);
		clrbits_le32(pll_addr, PLL_LOCK_EN_MASK);
	}
}

__maybe_unused static void set_pll_e90x(void)
{
	/* Low freq --> High freq */
	/* clock 1024/3 = 512M */
	clrsetbits_le32(CCMU_E90X_CLK_REG, E907_CLK_REG_E907_CLK_DIV_CLEAR_MASK,
			CCMU_E90X_CLK_CPU_M_1
				<< E907_CLK_REG_E907_CLK_DIV_OFFSET);
	clrsetbits_le32(CCMU_E90X_CLK_REG,
			E907_CLK_REG_E907_CLK_SEL_CLEAR_MASK,
			E907_CLK_REG_E907_CLK_SEL_PERI_PLL_614M
					<< E907_CLK_REG_E907_CLK_SEL_OFFSET);

	return;
}

__maybe_unused static void set_pll_a27l2(void)
{
	/* Low freq --> High freq */
	/* clock 768/3 = 256M */
	clrsetbits_le32(
		CCMU_A27_CLK_REG, A27L2_CLK_REG_A27L2_CLK_DIV_CLEAR_MASK,
		CCMU_A27_CLK_CPU_M_1 << A27L2_CLK_REG_A27L2_CLK_DIV_OFFSET);
	clrsetbits_le32(CCMU_A27_CLK_REG,
			A27L2_CLK_REG_A27L2_CLK_SEL_CLEAR_MASK,
			A27L2_CLK_REG_A27L2_CLK_SEL_CPU_PLL
				<< A27L2_CLK_REG_A27L2_CLK_SEL_OFFSET);

	clrsetbits_le32(CCMU_A27_CLK_REG, A27L2_CLK_REG_A27L2_CLK_EN_CLEAR_MASK,
			A27L2_CLK_REG_A27L2_CLK_EN_CLOCK_IS_ON
				<< A27L2_CLK_REG_A27L2_CLK_EN_OFFSET);

	return;
}

static void set_pll_peri_ctrl0(u32 en, u32 output_gate_en, u32 pll_n,
			       u32 pll_m)
{
	ccu_pll_peri_cfg_info_t pll_peri_cfg;

	if (en == 1) {
		pll_peri_cfg.pll_en = PLL_PERI_CTRL0_REG_PLL_EN_ENABLE
				      << PLL_PERI_CTRL0_REG_PLL_EN_OFFSET;
		pll_peri_cfg.pll_ldo_en =
			PLL_PERI_CTRL0_REG_PLL_LDO_EN_ENABLE
			<< PLL_PERI_CTRL0_REG_PLL_LDO_EN_OFFSET;
		pll_peri_cfg.pll_lock_en =
			PLL_PERI_CTRL0_REG_LOCK_ENABLE_ENABLE
			<< PLL_PERI_CTRL0_REG_LOCK_ENABLE_OFFSET;
	} else {
		pll_peri_cfg.pll_en = PLL_PERI_CTRL0_REG_PLL_EN_DISABLE
				      << PLL_PERI_CTRL0_REG_PLL_EN_OFFSET;
		pll_peri_cfg.pll_ldo_en =
			PLL_PERI_CTRL0_REG_PLL_LDO_EN_DISABLE
			<< PLL_PERI_CTRL0_REG_PLL_LDO_EN_OFFSET;
		pll_peri_cfg.pll_lock_en =
			PLL_PERI_CTRL0_REG_LOCK_ENABLE_DISABLE
			<< PLL_PERI_CTRL0_REG_LOCK_ENABLE_OFFSET;
	}
	pll_peri_cfg.pll_output_gate =
		PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_DISABLE
		<< PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_OFFSET;
	pll_peri_cfg.pll_n =
		pll_n
		<< PLL_PERI_CTRL0_REG_PLL_N_OFFSET; //CCU_AON_PLL_N_192  default
	pll_peri_cfg.pll_m =
		pll_m
		<< PLL_PERI_CTRL0_REG_PLL_INPUT_DIV_OFFSET; //CCU_AON_PLL_PERI_M_5  default

	clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
			PLL_PERI_CTRL0_REG_PLL_INPUT_DIV_CLEAR_MASK,
			pll_peri_cfg.pll_m);
	clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
			PLL_PERI_CTRL0_REG_PLL_N_CLEAR_MASK,
			pll_peri_cfg.pll_n);

	clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
			PLL_PERI_CTRL0_REG_PLL_EN_CLEAR_MASK |
				PLL_PERI_CTRL0_REG_PLL_LDO_EN_CLEAR_MASK |
				PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
			pll_peri_cfg.pll_en|pll_peri_cfg.pll_ldo_en|
			pll_peri_cfg.pll_output_gate);
	clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
			PLL_PERI_CTRL0_REG_LOCK_ENABLE_CLEAR_MASK,
			pll_peri_cfg.pll_lock_en);

	while ((!(readl(CCMU_PLL_PERI_CTRL0_REG) &
		  PLL_PERI_CTRL0_REG_LOCK_CLEAR_MASK)) &
	       pll_timeout(0xffff))
		;

	if (output_gate_en == 1) {
		pll_peri_cfg.pll_output_gate =
			PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_ENABLE
			<< PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_OFFSET;
		clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
				PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
				pll_peri_cfg.pll_output_gate);
	}
}

static void set_pll_peri_ctrl1(void)
{
	u32 reg;
	reg = readl(CCMU_PLL_PERI_CTRL1_REG);
	// reg |= PLL_PERI_CTRL1_REG_PLL_PERI_CKO_192_EN_ENABLE
	//        << PLL_PERI_CTRL1_REG_PLL_PERI_CKO_192_EN_OFFSET;
	reg = 0xFFFF;
	writel(reg, CCMU_PLL_PERI_CTRL1_REG);
	return;
}

/* pll peri hosc*2N/M = 3072M  hardware *2 */
__maybe_unused static void set_pll_peri(void)
{
	// When efuse is burned, brom will initialize the peri clock in advance.
	if (readl(CCMU_PLL_PERI_CTRL0_REG) & PLL_PERI_CTRL0_REG_PLL_EN_CLEAR_MASK) {
		//pass
	} else {
		if (aw_get_hosc_freq() == HOSC_FREQ_40M) {
			set_pll_peri_ctrl0(
				PLL_PERI_CTRL0_REG_PLL_EN_ENABLE,
				PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_ENABLE,
				CCMU_AON_PLL_CPU_N_192, CCMU_AON_PLL_CPU_M_5);
		} else {
			set_pll_peri_ctrl0(
				PLL_PERI_CTRL0_REG_PLL_EN_ENABLE,
				PLL_PERI_CTRL0_REG_PLL_OUTPUT_GATE_ENABLE,
				CCMU_AON_PLL_CPU_N_192, CCMU_AON_PLL_CPU_M_3);
		}
	}
	set_pll_peri_ctrl1();
	return;
}

/* pll csi rate = 675M = hosc / 4 * N , N = N(INT) + N(FRAC) */
__maybe_unused static void set_pll_csi(void)
{
	u32 n, input_div, wave_bot;

	/* Set N、M */
	if (aw_get_hosc_freq() ==
	    HOSC_FREQ_40M) {
		n = CCMU_AON_PLL_CPU_N_67;
		wave_bot = 0xc0010000;
		input_div = PLL_CSI_CTRL_REG_PLL_INPUT_DIV_4;
	} else {
		n = CCMU_AON_PLL_CPU_N_56;
		wave_bot = 0xc0008000;
		input_div = PLL_CSI_CTRL_REG_PLL_INPUT_DIV_2;
	}
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
				PLL_CSI_CTRL_REG_PLL_FACTOR_N_CLEAR_MASK,
				n
				<< PLL_CSI_CTRL_REG_PLL_FACTOR_N_OFFSET);
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
			input_div
			<< PLL_CSI_CTRL_REG_PLL_INPUT_DIV_OFFSET);

	/* Enable PLL_EN PLL_LEO_EN */
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_PLL_EN_CLEAR_MASK,
			PLL_CSI_CTRL_REG_PLL_EN_ENABLE
			<< PLL_CSI_CTRL_REG_PLL_EN_OFFSET);
	clrsetbits_le32(CCMU_PLL_PERI_CTRL0_REG,
			PLL_CSI_CTRL_REG_PLL_LDO_EN_CLEAR_MASK,
			PLL_CSI_CTRL_REG_PLL_LDO_EN_ENABLE
			<< PLL_CSI_CTRL_REG_PLL_LDO_EN_OFFSET);

	/* Enable SDM */
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_PLL_SDM_EN_CLEAR_MASK,
			PLL_CSI_CTRL_REG_PLL_SDM_EN_ENABLE
			<< PLL_CSI_CTRL_REG_PLL_SDM_EN_OFFSET);

	/* Set WAVE_BOT and SPR_FREQ_MODE of PAT0 */
	clrsetbits_le32(CCMU_PLL_CSI_PAT0_REG,
			PLL_CSI_PAT0_CTRL_REG_WAVE_BOT_CLEAR_MASK,
			wave_bot
			<< PLL_CSI_PAT0_CTRL_REG_WAVE_BOT_OFFSET);

	/* Enable SIG_DELT_PAT_EN of PAT1 */
	clrsetbits_le32(CCMU_PLL_CSI_PAT1_REG,
			PLL_CSI_PAT1_CTRL_REG_SIG_DELT_PAT_EN_CLEAR_MASK,
			0x1
			<< PLL_CSI_PAT1_CTRL_REG_SIG_DELT_PAT_EN_OFFSET);

	/* Disable OUTPUT and Enable LOCK_ENABLE */
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
			PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_DISABLE
			<< PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_OFFSET);
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_LOCK_ENABLE_CLEAR_MASK,
			PLL_CSI_CTRL_REG_LOCK_ENABLE_ENABLE
			<< PLL_CSI_CTRL_REG_LOCK_ENABLE_OFFSET);

	/* Wait Lock_status */
	while ((!(readl(CCMU_PLL_CSI_CTRL_REG) & PLL_CSI_CTRL_REG_LOCK_CLEAR_MASK)) & pll_timeout(0xffff))
		;

	/* Enable Output */
	clrsetbits_le32(CCMU_PLL_CSI_CTRL_REG,
			PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
			PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_ENABLE
			<< PLL_CSI_CTRL_REG_PLL_OUTPUT_GATE_OFFSET);
}

/* pll video = 1200M = hosc * N */
__maybe_unused static void set_pll_video(void)
{
	/* Close Lock_en and Output_gate */
	clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
			PLL_VIDEO_CTRL_REG_LOCK_ENABLE_CLEAR_MASK,
			PLL_VIDEO_CTRL_REG_LOCK_ENABLE_DISABLE
			<< PLL_VIDEO_CTRL_REG_LOCK_ENABLE_OFFSET);
	clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
			PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
			PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_DISABLE
			<< PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_OFFSET);

	/* Set N、M */
	if (aw_get_hosc_freq() ==
	    HOSC_FREQ_40M) {
		clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
				PLL_VIDEO_CTRL_REG_PLL_N_CLEAR_MASK,
				CCMU_AON_PLL_CPU_N_30
				<< PLL_VIDEO_CTRL_REG_PLL_N_OFFSET);
	} else {
		clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
				PLL_VIDEO_CTRL_REG_PLL_N_CLEAR_MASK,
				CCMU_AON_PLL_CPU_N_50
				<< PLL_VIDEO_CTRL_REG_PLL_N_OFFSET);
	}
	clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
			PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
			PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_1
			<< PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_OFFSET);

	/* Enable Lock_en */
	clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
			PLL_VIDEO_CTRL_REG_LOCK_ENABLE_CLEAR_MASK,
			PLL_VIDEO_CTRL_REG_LOCK_ENABLE_ENABLE
			<< PLL_VIDEO_CTRL_REG_LOCK_ENABLE_OFFSET);

	/* Wait Lock_status */
	while ((!(readl(CCMU_PLL_VIDEO_CTRL_REG) & PLL_VIDEO_CTRL_REG_LOCK_CLEAR_MASK)) & pll_timeout(0xffff))
		;

	/* Enable Output */
	clrsetbits_le32(CCMU_PLL_VIDEO_CTRL_REG,
			PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_CLEAR_MASK,
			PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_ENABLE
			<< PLL_VIDEO_CTRL_REG_PLL_OUTPUT_GATE_OFFSET);
}

// ahb = 768/4 = 192M
__maybe_unused static void set_ahb(void)
{
	clrsetbits_le32(CCMU_AHB_CLK_REG,
				AHB_CLK_REG_AHB_CLK_DIV_CLEAR_MASK,
			CCMU_AON_PLL_CPU_M_4
				<< AHB_CLK_REG_AHB_CLK_DIV_OFFSET);
	udelay(2);
	clrsetbits_le32(CCMU_AHB_CLK_REG,
				AHB_CLK_REG_AHB_SEL_CLEAR_MASK,
			AHB_CLK_REG_AHB_SEL_PERI_768M
				<< AHB_CLK_REG_AHB_SEL_OFFSET);
	udelay(2);
	return;
}

// apb = 384/4 = 96M
__maybe_unused static void set_apb(void)
{
	clrsetbits_le32(CCMU_APB_CLK_REG,
				APB_CLK_REG_APB_CLK_DIV_CLEAR_MASK,
			CCMU_AON_PLL_CPU_M_4
				<< APB_CLK_REG_APB_CLK_DIV_OFFSET);
	udelay(2);
	clrsetbits_le32(CCMU_APB_CLK_REG,
				APB_CLK_REG_APB_SEL_CLEAR_MASK,
			APB_CLK_REG_APB_SEL_PERI_384M
				<< APB_CLK_REG_APB_SEL_OFFSET);
	udelay(2);
	return;
}

// 192M
static void set_apb_spec(void)
{
	clrsetbits_le32(CCMU_APB_SPEC_CLK_REG,
			APB_SPEC_CLK_REG_APB_SPEC_SEL_CLEAR_MASK |
				APB_SPEC_CLK_REG_APB_SPEC_CLK_DIV_CLEAR_MASK,
			APB_SPEC_CLK_REG_APB_SPEC_SEL_PERI_192M
				<< APB_SPEC_CLK_REG_APB_SPEC_SEL_OFFSET);
	return;
}
#endif

#ifdef CFG_GPADC_KEY
int sunxi_clock_init_gpadc(void)
{
	uint reg_val = 0;
	/* set source clk 24m */
	clrsetbits_le32(SUNXI_CCM_APP_BASE + GPADC_CLK_REG,
		GPADC_CLK_REG_GPADC_CLK_SEL_CLEAR_MASK,
		GPADC_CLK_REG_GPADC_CLK_SEL_CLK_24M <<
		GPADC_CLK_REG_GPADC_CLK_SEL_OFFSET);

	/* set clk en */
	clrsetbits_le32(SUNXI_CCM_APP_BASE + GPADC_CLK_REG,
		GPADC_CLK_REG_GPADC_CLK_EN_CLEAR_MASK,
		GPADC_CLK_REG_GPADC_CLK_EN_CLOCK_IS_ON <<
		GPADC_CLK_REG_GPADC_CLK_EN_OFFSET);

	/* reset */
	setbits_le32(SUNXI_CCM_APP_BASE + BUS_Reset1_REG, 0x3 << 0);

	/* enable KEYADC gating */
	setbits_le32(SUNXI_CCM_APP_BASE + BUS_CLK_GATING1_REG, 0x3 << 0);

	/* enable KEYADC BOOTSTRAP enable */
	reg_val = readl(GP_BOOTSTRAP_EN_REG);
	reg_val |= (1 << 0);
	writel(reg_val, GP_BOOTSTRAP_EN_REG);

	/* disable auto cali */
	reg_val = readl(GP_CTRL);
	reg_val &= ~(1 << 23);
	writel(reg_val, GP_CTRL);

	/* set default cali data */
	reg_val = 0x7FF;
	writel(reg_val, GP_CDATA);

	return 0;
}
#endif /* CFG_GPADC_KEY */

void sunxi_e907_clock_init(u32 addr);
void sunxi_e907_clock_reset(void);

void sunxi_board_pll_init(void)
{
#ifndef CFG_SUNXI_SIMPLEBOOT
#ifndef CFG_SUNXI_PMBOOT
	printf("set pll start\n");
#ifdef CFG_SUNXI_VF_2_1
// 1200MHz
#define PLL_CPU_CLK_40M_N CCMU_AON_PLL_CPU_N_60
#define PLL_CPU_CLK_24M_N CCMU_AON_PLL_CPU_N_50
#else
// 960MHz
#define PLL_CPU_CLK_40M_N CCMU_AON_PLL_CPU_N_48
#define PLL_CPU_CLK_24M_N CCMU_AON_PLL_CPU_N_40
#endif
	// set PLL_CPU=hosc*N/D
	// set PLL_VEDIO hosc*N/D
	if (aw_get_hosc_freq() == HOSC_FREQ_40M) {
		set_pll_general(CCMU_PLL_CPUX_CTRL_REG,
				PLL_CPU_CTRL_REG_PLL_EN_ENABLE,
				PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_ENABLE,
				CCMU_AON_PLL_CPU_D_2,
				PLL_CPU_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
				PLL_CPU_CTRL_REG_PLL_INPUT_DIV_OFFSET,
				PLL_CPU_CLK_40M_N);
		// When efuse is burned, brom will initialize the vedio clock in advance.
		if (!(readl(CCMU_PLL_VIDEO_CTRL_REG) &
		    PLL_CPU_CTRL_REG_PLL_EN_CLEAR_MASK)) {
			set_pll_general(CCMU_PLL_VIDEO_CTRL_REG, //1180M
					PLL_CPU_CTRL_REG_PLL_EN_ENABLE,
					PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_ENABLE,
					CCMU_AON_PLL_CPU_D_4,
					PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
					PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_OFFSET,
					CCMU_AON_PLL_CPU_N_118);
		}
	} else {
		set_pll_general(CCMU_PLL_CPUX_CTRL_REG,
				PLL_CPU_CTRL_REG_PLL_EN_ENABLE,
				PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_ENABLE,
				CCMU_AON_PLL_CPU_D_1,
				PLL_CPU_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
				PLL_CPU_CTRL_REG_PLL_INPUT_DIV_OFFSET,
				PLL_CPU_CLK_24M_N);
		// When efuse is burned, brom will initialize the vedio clock in advance.
		if (!(readl(CCMU_PLL_VIDEO_CTRL_REG) &
		    PLL_CPU_CTRL_REG_PLL_EN_CLEAR_MASK)) {
			set_pll_general(CCMU_PLL_VIDEO_CTRL_REG,
					PLL_CPU_CTRL_REG_PLL_EN_ENABLE,
					PLL_CPU_CTRL_REG_PLL_OUTPUT_GATE_ENABLE,
					CCMU_AON_PLL_CPU_D_2,
					PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_CLEAR_MASK,
					PLL_VIDEO_CTRL_REG_PLL_INPUT_DIV_OFFSET,
					CCMU_AON_PLL_CPU_N_99);
		}
	}
	set_pll_e90x();
	set_pll_a27l2();
	set_ahb();
	set_apb();
	set_pll_video();
	set_pll_csi();
#endif
#else
	printf("already init in simpleboot, skip\n");
#endif /* pll already init in simpleboot */

#ifdef CFG_GPADC_KEY
	sunxi_gpadc_init();
#endif
	printf("set pll end\n");

#ifdef CFG_AMP_BOOT0_BIN_PATH
	{
		sunxi_e907_clock_reset();
		printf("sunxi_e907_clock_init : 0x%lx\n", CFG_AMP_BOOT0_RUN_ADDR);
		sunxi_e907_clock_init(CFG_AMP_BOOT0_RUN_ADDR);
	}
#endif

#if CLOCK_DEBUG
	printf("-----BROM clock------\n");
	dump_sys_clock_info();
	get_sys_clock_info();
	printf("=====BOOT0 clock=====>\n");
	dump_sys_clock_info();
#endif
	return;
}

void sunxi_board_clock_reset(void)
{
	/* jump fel */
	writel(SUNXI_UPGRADE_FLAG_SET, SUNXI_UPGRADE_FLAG_ADDR);
	setbits_le32(SUNXI_PRCM_BASE + 0x1c, BIT(3)); /* enable WDT clk */
	writel(0x16aa0000, SUNXI_RTC_WDG_BASE + 0x18); /* disable WDT */
	writel(0x16aa0000 | BIT(0), SUNXI_RTC_WDG_BASE + 0x08); /* trigger WDT */
	return;
}

void sunxi_clock_init_uart(int port)
{
	u32 i, reg;
#if CLOCK_DEBUG
	get_sys_clock_info();
#endif

/*pm runs on dram*/
#ifndef CFG_SUNXI_PMBOOT
	set_pll_peri();
	set_apb_spec();
#endif
	/* reset */
	reg = readl(CCMU_BUS_Reset0_REG);
	reg |= (1 << (BUS_Reset0_REG_PRESETN_UART0_SW_OFFSET + port));
	writel(reg, CCMU_BUS_Reset0_REG);

	/* gate */
	reg = readl(CCMU_UART_BGR_REG);
	reg &= ~(1<< (BUS_CLK_GATING0_REG_UART0_PCLK_EN_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg |= (1 << (BUS_CLK_GATING0_REG_UART0_PCLK_EN_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);

	return;
}

void sunxi_e907_clock_init(u32 addr)
{
	u32 reg_val;

	/*DE-assert e907 rv_cfg_reset*/
	reg_val = readl(CCMU_BUS_Reset0_REG);
	reg_val |= CCMU_E907_CFG_RST;
	writel(reg_val, CCMU_BUS_Reset0_REG);

	/*DE-assert e907 rv_sys_apb_reset*/
	reg_val = readl(CCMU_BUS_Reset0_REG);
	reg_val |= CCMU_E907_SYS_APB_RST;
	writel(reg_val, CCMU_BUS_Reset0_REG);

	/*riscv_cfg Gating Clock ON */
	reg_val = readl(CCMU_BUS_CLK_GATING0_REG);
	reg_val |= CCMU_E907_CFG_CLK_GATING;
	writel(reg_val, CCMU_BUS_CLK_GATING0_REG);

	/*set e907 start addr */
	reg_val = addr;
	writel(reg_val, E907_STA_ADD_REG);

	/*DE-assert e907_rstn_sw */
	reg_val = 0xA5690001;
	writel(reg_val, CCMU_E907_RSTN_REG);
}

void sunxi_e907_clock_reset(void)
{
	u32 reg_val;

	/*riscv_cfg Gating Clock OFF */
	reg_val = readl(CCMU_BUS_CLK_GATING0_REG);
	reg_val &= ~(CCMU_E907_CFG_CLK_GATING);
	writel(reg_val, CCMU_BUS_CLK_GATING0_REG);

	/*assert e907 rv_cfg_reset*/
	reg_val = readl(CCMU_BUS_Reset0_REG);
	reg_val &= ~(CCMU_E907_CFG_RST);
	writel(reg_val, CCMU_BUS_Reset0_REG);

	/*assert e907 rv_sys_apb_reset*/
	reg_val = readl(CCMU_BUS_Reset0_REG);
	reg_val &= ~(CCMU_E907_SYS_APB_RST);
	writel(reg_val, CCMU_BUS_Reset0_REG);

	/*assert e907_rstn*/
	reg_val = 0xA5690000;
	writel(reg_val, CCMU_E907_RSTN_REG);
}
