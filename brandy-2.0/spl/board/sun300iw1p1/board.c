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
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/csr.h>
#include <linux/compiler.h>
#include <private_boot0.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/rtc.h>
#include <config.h>
#include <part_efi.h>
#include <sunxi-chips.h>
#include <arch/sun300iw1p1/cpu_autogen.h>
#include <arch/efuse.h>
#include <arch/sunxi_ebp.h>
#include <arch/sunxi_ecc.h>
#include <arch/physical_key.h>

int sunxi_fes_board_init_early(void)
{
	u32 reg_val = 0;
	sunxi_board_init_early();
	/* check fel upgrade flag status, if set clear it */
	reg_val = readl(SUNXI_UPGRADE_FLAG_ADDR);
	if (reg_val) {
		writel(SUNXI_UPGRADE_FLAG_CLEAR, SUNXI_UPGRADE_FLAG_ADDR);
		while (readl(SUNXI_UPGRADE_FLAG_ADDR))
			;
	}
	return 0;
}

int sunxi_pmboot_board_init_early(void)
{
	sunxi_board_init_early();
	return 0;
}

static int sunxi_hosc_detect(void)
{
	/* hosc dectect enable bit in efuse is bit 613 */
	u32 hosc_det_en = (readl(SID_EFUSE + 76) >> 5) & 0x1;
	u32 val = readl(CCMU_HOSC_FREQ_DET_REG);

	if ((hosc_det_en != 0) ||
	    (val & HOSC_FREQ_DET_HOSC_FREQ_READY_CLEAR_MASK) != 0) {
		val = (readl(CCMU_PLL_FUNC_CFG_REG) &
		       PLL_FUNC_CFG_REG_DCXO_ST_CLEAR_MASK) >>
		      PLL_FUNC_CFG_REG_DCXO_ST_OFFSET;
		if (val == PLL_FUNC_CFG_REG_DCXO_ST_SYSTEM_USES_DCXO_40MHZ)
			return HOSC_FREQ_40M;
		else
			return HOSC_FREQ_24M;
	}

	writel(val & (~HOSC_FREQ_DET_HOSC_CLEAR_MASK), CCMU_HOSC_FREQ_DET_REG);
	writel(val | HOSC_FREQ_DET_HOSC_ENABLE_DETECT, CCMU_HOSC_FREQ_DET_REG);

	do {
		val = readl(CCMU_HOSC_FREQ_DET_REG);
	} while (!(val & HOSC_FREQ_DET_HOSC_FREQ_READY_CLEAR_MASK));

	val = (val & HOSC_FREQ_DET_HOSC_FREQ_DET_CLEAR_MASK) >>
	      HOSC_FREQ_DET_HOSC_FREQ_DET_OFFSET;

	if (val > HOSC_24M_COUNTER_MIN && val < HOSC_24M_COUNTER_MAX)
		return HOSC_FREQ_24M;
	else if (val > HOSC_40M_COUNTER_MIN && val < HOSC_40M_COUNTER_MAX)
		return HOSC_FREQ_40M;

	/* freq detect abnormal val, default user 40M */
	return HOSC_FREQ_40M;
}

#define BIT(nr)						(1UL << (nr))
#define CSR_MCACHE_CTL					0x7ca
#define CSR_MMISC_CTL					0x7d0

/* MMISC control register */
#define V5_MMISC_CTL_NON_BLOCKING_OFFSET		8
#define V5_MMISC_CTL_NON_BLOCKING_EN			BIT(V5_MMISC_CTL_NON_BLOCKING_OFFSET)

/* MCACHE control register */
#define V5_MCACHE_CTL_IC_EN_OFFSET			0
#define V5_MCACHE_CTL_DC_EN_OFFSET			1
#define V5_MCACHE_CTL_CCTL_SUEN_OFFSET			8
#define V5_MCACHE_CTL_L1I_PREFETCH_OFFSET		9
#define V5_MCACHE_CTL_L1D_PREFETCH_OFFSET		10
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_1		13
#define V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1		15

#define V5_MCACHE_CTL_IC_EN				BIT(V5_MCACHE_CTL_IC_EN_OFFSET)
#define V5_MCACHE_CTL_CCTL_SUEN				(1UL << V5_MCACHE_CTL_CCTL_SUEN_OFFSET)
#define V5_MCACHE_CTL_L1I_PREFETCH_EN			BIT(V5_MCACHE_CTL_L1I_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_L1D_PREFETCH_EN			BIT(V5_MCACHE_CTL_L1D_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_DC_WAROUND_1_EN			BIT(V5_MCACHE_CTL_DC_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_L2C_WAROUND_1_EN			BIT(V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1)

static void andes_harts_early_init(void)
{
	unsigned long long mmisc_ctl_val = csr_read(CSR_MMISC_CTL);
	unsigned long long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);


	mcache_ctl_val |= (V5_MCACHE_CTL_IC_EN | V5_MCACHE_CTL_CCTL_SUEN | \
			  V5_MCACHE_CTL_L1I_PREFETCH_EN | V5_MCACHE_CTL_L1D_PREFETCH_EN | \
			  V5_MCACHE_CTL_DC_WAROUND_1_EN | V5_MCACHE_CTL_L2C_WAROUND_1_EN);

	csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

	if (!(mmisc_ctl_val & V5_MMISC_CTL_NON_BLOCKING_EN))
		mmisc_ctl_val |= V5_MMISC_CTL_NON_BLOCKING_EN;
	csr_write(CSR_MMISC_CTL, mmisc_ctl_val);
}

/* backup efuse information(0x0~0xFF) to ddr */
#ifdef SUNXI_EFUSE_RAM_BASE_IN_DDR
#ifndef CFG_SUNXI_PMBOOT
__maybe_unused static void sunxi_efuse_backup_to_ddr(void)
{
#if defined(CFG_SUNXI_EFUSE_SW_EBP) || defined(CFG_SUNXI_EFUSE_SW_ECC_HAMMING)
	int ret = 0;
#endif
	uint i;
	void __iomem *efuse_map_sram_base = sunxi_get_iobase(SID_EFUSE);
	void __iomem *efuse_map_ddr_base = sunxi_get_iobase(SUNXI_EFUSE_RAM_BASE_IN_DDR);
	u32 efuse_data[EFUSE_MAP_SIZE] = {0};

	/* TODO: clarify why memcpy does not take effect */
	for (i = 0; i < EFUSE_MAP_SIZE; i += 4) {
		efuse_data[i/4] = readl(efuse_map_sram_base + i);
	}
#ifdef CFG_SUNXI_EFUSE_SW_ECC_HAMMING
	ret = sunxi_ecc_correct(efuse_data, EFUSE_MAP_SIZE);
	if (ret)
		printf("ECC Decode Error\n");
#endif
#ifdef CFG_SUNXI_EFUSE_SW_EBP
	ret = sunxi_ebp_correct(efuse_data, EFUSE_MAP_SIZE);
	if (ret)
		printf("EBP Decode Error\n");
#endif

	/* Write corrected data back to DDR */
	for (i = 0; i < EFUSE_MAP_SIZE; i += 4) {
		writel(efuse_data[i/4], efuse_map_ddr_base + i);
	}
}
#endif
#endif

/* startup source && wakeup source */
#define RTC_SOURCE_INDEX            (5)
#define RTC_SOFTWARE_FLAG           (7)
/*
 * use the prcm_priv5(0x4A000214) to record the reset source and wakeup source
 * reset source:
 * bit31 -- rtcwdg
 * bit29 -- detect
 * bit28 -- poweron
 * wakeup source:
 * bit11 -- wlan
 * bit10 -- alarm1
 * bit9  -- alarm0
 * bit8  -- wakeup timer
 * bit7  -- wakeup io7
 * bit6  -- wakeup io6
 * bit5  -- wakeup io5
 * bit4  -- wakeup io4
 * bit3  -- wakeup io3
 * bit2  -- wakeup io2
 * bit1  -- wakeup io1
 * bit0  -- wakeup io0
 */
#define WAKEUP_IO_STA_REGISTER      (0x4A000058)
#define WAKEUP_TIMER_VAL_REGISTER   (0x4A000404)
#define WAKEUP_TIMER_PENDING_MASK   (1 << 31)
#define ALARM0_IRQST_REGISTER       (0x4A000C30)
#define ALARM1_IRQST_REGISTER       (0x4a000c4c)
#define ALARM_IRQ_PENDING_MASK      (1 << 0)
#define WLAN_IRQ_STATUS_REG         (0x7AA800A0)
#define WLAN_IRQ_PENDING_MASK       (1 << 1)
#define RESET_SOURCE_REGISTER       (0x4A0001D4)
#define RESET_SOURCE_MASK           (0x0000000F)
#define WAKEUP_IO_NUM               (8)

#define RTC_WAKESRC_MASK_WUPIO_0    (0x00000001)
#define RTC_WAKESRC_MASK_WUPIO_1    (0x00000002)
#define RTC_WAKESRC_MASK_WUPIO_2    (0x00000004)
#define RTC_WAKESRC_MASK_WUPIO_3    (0x00000008)
#define RTC_WAKESRC_MASK_WUPIO_4    (0x00000010)
#define RTC_WAKESRC_MASK_WUPIO_5    (0x00000020)
#define RTC_WAKESRC_MASK_WUPIO_6    (0x00000040)
#define RTC_WAKESRC_MASK_WUPIO_7    (0x00000080)
#define RTC_WAKESRC_MASK_WUPTIMER   (0x00000100)
#define RTC_WAKESRC_MASK_ALARM_0    (0x00000200)
#define RTC_WAKESRC_MASK_ALARM_1    (0x00000400)
#define RTC_WAKESRC_MASK_WLAN       (0x00000800)
#define RTC_WAKESRC_MASK_ALL        (0x00000FFF)
#define RTC_RSTSRC_SHIFT            (28)

#ifndef CFG_SUNXI_PMBOOT
static void update_hardware_source(void)
{
	uint32_t src = 0;
	uint32_t val;
	uint32_t i;

	/* get hardware reset source */
	src = readl(RESET_SOURCE_REGISTER) & RESET_SOURCE_MASK;
	src <<= RTC_RSTSRC_SHIFT;

	/* get hardware wakeup source */
	/* check wakeup io */
	val = readl(WAKEUP_IO_STA_REGISTER);
	for (i = 0; i <= WAKEUP_IO_NUM; i++) {
		if (val & (1 << i)) {
			src |= (1 << i);
		}
	}
	/* check wakeup timer */
	if (readl(WAKEUP_TIMER_VAL_REGISTER) & WAKEUP_TIMER_PENDING_MASK) {
		src |= RTC_WAKESRC_MASK_WUPTIMER;
	}
	/* check rtc alarm */
	if (readl(ALARM0_IRQST_REGISTER) & ALARM_IRQ_PENDING_MASK) {
		src |= RTC_WAKESRC_MASK_ALARM_0;
	}
	if (readl(ALARM1_IRQST_REGISTER) & ALARM_IRQ_PENDING_MASK) {
		src |= RTC_WAKESRC_MASK_ALARM_1;
	}
	/* check wlan */
	if (readl(WLAN_IRQ_STATUS_REG) & WLAN_IRQ_PENDING_MASK) {
		src |= RTC_WAKESRC_MASK_WLAN;
	}

	rtc_set_data(RTC_SOURCE_INDEX, src);
}
#endif

static void show_hardware_source(void)
{
	const uint32_t src = rtc_read_data(RTC_SOURCE_INDEX);
	const uint32_t reset_src = src >> RTC_RSTSRC_SHIFT;
	const uint32_t wake_src = src & RTC_WAKESRC_MASK_ALL;
	uint32_t i;
	int has_print = 0;

	printf("Hardware Reset and Wakeup Sources:\n");

	/* Print reset sources */
	printf("Reset Sources: ");
	if (0xb == reset_src) {
		raw_printf("cold_boot ");
		has_print = 1;
	} else {
		if (reset_src & (1 << 3)) {
			if (0x2 == rtc_read_data(RTC_SOFTWARE_FLAG))
				raw_printf("user_reboot ");
			else
				raw_printf("rtc_wdg_rst ");
			has_print = 1;
		}
		if (reset_src & (1 << 1)) {
			raw_printf("det_rst ");
			has_print = 1;
		}
		if (reset_src & (1 << 0)) {
			raw_printf("pwron_rst ");
			has_print = 1;
		}
	}

	if (!has_print) {
		raw_printf("none");
	}
	raw_printf("\n");

	/* Print wakeup sources */
	printf("Wakeup Sources: ");
	has_print = 0;

	/* Check wakeup IO */
	for (i = 0; i <= WAKEUP_IO_NUM; i++) {
		if (wake_src & (1 << i)) {
			raw_printf("WUP_IO%d ", i);
			has_print = 1;
		}
	}

	/* Check wakeup timer */
	if (wake_src & RTC_WAKESRC_MASK_WUPTIMER) {
		raw_printf("WUP_TIMER ");
		has_print = 1;
	}

	/* Check RTC alarms */
	if (wake_src & RTC_WAKESRC_MASK_ALARM_0) {
		raw_printf("ALARM0 ");
		has_print = 1;
	}
	if (wake_src & RTC_WAKESRC_MASK_ALARM_1) {
		raw_printf("ALARM1 ");
		has_print = 1;
	}

	/* Check WLAN */
	if (wake_src & RTC_WAKESRC_MASK_WLAN) {
		raw_printf("WLAN ");
		has_print = 1;
	}

	if (!has_print) {
		raw_printf("none");
	}
	raw_printf("\n");
}

int sunxi_board_init_early(void)
{
#ifndef CFG_SUNXI_PMBOOT
	update_hardware_source();
#endif

	andes_harts_early_init();

	/* detect hosc */
	if (sunxi_hosc_detect() == HOSC_FREQ_24M) {
		aw_set_hosc_freq(HOSC_FREQ_24M);

		writel(readl(CCMU_FUNC_CFG_REG) |
			       PLL_FUNC_CFG_REG_DCXO_ST_CLEAR_MASK,
		       CCMU_FUNC_CFG_REG);
	} else {
		aw_set_hosc_freq(HOSC_FREQ_40M);

		writel(readl(CCMU_FUNC_CFG_REG) &
		       (~PLL_FUNC_CFG_REG_DCXO_ST_CLEAR_MASK),
			CCMU_FUNC_CFG_REG);
	}

	/* Enable PLMT_CLOCK */
	if (!(readl(CCMU_A27L2_MTCLK_REG) & CCMU_A27L2_MTCLK_EN))
		writel(CCMU_A27L2_MTCLK_EN | A27L2_MT_CLK_SEL_HOSC,
			CCMU_A27L2_MTCLK_REG);

	return 0;
}

__maybe_unused static int sunxi_gpadc_ldo_enable(void)
{
	u32 reg_val = 0;

	reg_val = readl(SUNXI_CCM_APP_BASE  + BUS_CLK_GATING1_REG);
	reg_val |= BUS_CLK_GATING1_REG_THS_PCLK_GATING_CLEAR_MASK;
	writel(reg_val, SUNXI_CCM_APP_BASE + BUS_CLK_GATING1_REG);

	reg_val = readl(SUNXI_CCM_APP_BASE + BUS_Reset1_REG);
	reg_val |= BUS_Reset1_REG_THS_RSTN_SW_CLEAR_MASK;
	writel(reg_val, SUNXI_CCM_APP_BASE + BUS_Reset1_REG);

	reg_val = readl(SUNXI_THS_BASE + 0x4);
	reg_val |= (1 << 16);
	writel(reg_val, SUNXI_THS_BASE + 0x4);

	return 0;
}

#ifndef FPGA_PLATFORM
void rtc_set_vccio_det_spare(void)
{
	u32 val = 0;

	/* set detection threshold to 2.9V */
	val = readl(SUNXI_PMU_RTC_BASE + VCC33_DET_CTRL_REG);
	val &= ~(VCCIO_THRESHOLD_MASK << 8);
	val |= (VCCIO_THRESHOLD_VOLTAGE_2_9);
	writel(val, SUNXI_PMU_RTC_BASE + VCC33_DET_CTRL_REG);

	/* enable vccio debonce */
	val = readl(SUNXI_PRCM_BASE + POR_RESET_CTRL_REG);
	val &= ~VCC33_DET_RSTN_ENABLE;
	writel(val, SUNXI_PRCM_BASE + POR_RESET_CTRL_REG);
}

static void fastboot_info_clear(void)
{
	u32 val = 0;

	val = readl(SUNXI_PRCM_BASE + 0x258);
	if (((val & 0xf0000000) >> 28) != 0x3)
		return;
	val &= 0xf0ffffff;
	writel(val, SUNXI_PRCM_BASE + 0x258);
}

static void sunxi_rtc_init(void)
{
	/* If the SYSRTC interrupt flag is set, clear the flag. */
	u32 val = 0;

	val = readl(SUNXI_SYSRTC_BASE + ALARM0_IRQ_STA_REG);
	if (val & ALARM0_IRQ_PEND)
		writel(ALARM0_IRQ_PEND, SUNXI_SYSRTC_BASE + ALARM0_IRQ_STA_REG);
}

typedef struct {
	int low;   // 区间下限
	int high;  // 区间上限
	int value; // code_b对应值
} RES_RANG;

// 定义“PHY外(die内)独立式校准RES”电阻校准区间和对应的查表值(用于DDRPHY)
RES_RANG phy_out_range[] = {
	{1870, 1890, 6},
	{1891, 1911, 6},
	{1912, 1932, 6},
	{1933, 1953, 6},
	{1954, 1974, 7},
	{1975, 1995, 7},
	{1996, 2016, 7},
	{2017, 2037, 8},
	{2038, 2058, 8},
	{2059, 2079, 8},
	{2080, 2100, 9},
	{2101, 2121, 9},
	{2122, 2142, 9},
	{2143, 2163, 10},
	{2164, 2184, 10},
	{2185, 2205, 10},
};

uint32_t DDR_PHY_Resctrl_Test(void)
{
#define	RESCAL_CTRL_REG		0x160
#define	TSENSOR_EN			0x4

	uint32_t input_val = 0, result_val = 0;

	/* disable the ADC function */
	clrbits_le32(GP_CTRL, 0x1 << 16);

	/* set the acquiring time of ADC\the ADC sample frequency divider */
	writel((0x164 << 0) | (0x1DF << 16), GP_SR_CON);

	/* set the single conversion mode */
	clrbits_le32(GP_CTRL, 0x3 << 18);

	/* enable the analog input channel */
	setbits_le32(GP_CS_EN, 0x1 << 5);

	/* set the gpadc built in LDO enable signal wait for 10us to complete the voltage setup */
	setbits_le32(SUNXI_THS_BASE + TSENSOR_EN, 0x1 << 16);
	udelay(10);

	/* enable RES calibration\ select DDR_PHY RES calibration channel */
	setbits_le32(SUNXI_SYSCTRL_BASE + RESCAL_CTRL_REG, (0x1 << 1) | (0x3 << 2));

	/* enable the ADC function */
	setbits_le32(GP_CTRL, 0x1 << 16);

	while (!readl(GP_CH0_DATA + 4 * 5))
		;
	input_val = readl(GP_CH0_DATA + 4 * 5);

	/* disable the gpadc built in LDO */
	clrbits_le32(SUNXI_THS_BASE + TSENSOR_EN, 0x1 << 16);

	// 遍历区间数组，查找匹配的区间
	for (int i = 0; i < sizeof(phy_out_range)/sizeof(phy_out_range[0]); ++i) {
		if (input_val >= phy_out_range[i].low && input_val <= phy_out_range[i].high) {
			result_val = phy_out_range[i].value;
			break;
		}
	}

	if (result_val)
		printf("DBG:[%s]-line(%d) input_val = %d =>> result_val = %d\n", __func__, __LINE__, input_val, result_val);
	else
		printf("DBG:[%s]-line(%d) input_val = %d =>> fail to match code_b\n", __func__, __LINE__, input_val);

	return result_val;
}

void set_resistor_calibration(void)
{
#define	DDR_RES_CTRL_REG	0x164
#define	USB_RES_CTRL_REG	0x178
#define	CSI_RES_CTRL_REG	0x17c
#define	KEY_FIELD		0x1931

	unsigned int res_trim = 0, res240_trim = 0, usb_trim = 0;

	/* ddr resistor calibratiom */
	res240_trim = (readl(SID_EFUSE + EFUSE_FT_ZONE_LOW) & RES240_TRIM_MASK) >> RES240_TRIM_POS;
	if (res240_trim)
		writel(res240_trim | (res240_trim << 8) | (KEY_FIELD << 16),
				SUNXI_SYSCTRL_BASE + DDR_RES_CTRL_REG);
	else {
		res240_trim = DDR_PHY_Resctrl_Test();
		if (res240_trim)
			writel(res240_trim | (res240_trim << 8) | (KEY_FIELD << 16),
					SUNXI_SYSCTRL_BASE + DDR_RES_CTRL_REG);
	}

	/* CSI & USB resistor calibratiom */
	res_trim = (readl(SID_EFUSE + EFUSE_FT_ZONE_LOW) & RES_TRIM_MASK) >> RES_TRIM_POS;
	if (res_trim) {
		writel(res_trim | (KEY_FIELD << 16),
				SUNXI_SYSCTRL_BASE + CSI_RES_CTRL_REG);

		usb_trim = (readl(SID_EFUSE + EFUSE_FT_ZONE_HIGH) & USB_TRIM_MASK) >> USB_TRIM_POS;
		if (!(usb_trim & 0x1))
			writel(res_trim | (KEY_FIELD << 16),
					SUNXI_SYSCTRL_BASE + USB_RES_CTRL_REG);
	}

}

void pmu_aon_set_en2_wait_time(void)
{
#define PMU_AON_EN2_WAIT_TIME_US    100
#define PMU_AON_SETTIME_REG1        0x4
#define PMU_AON_EN2_SETTIME_MASK    (0xfff << 16)
#define PMU_AON_EN2_SETTIME_OFFSET  16

	u32 value;
	/* set wait 100us after PMC_EN2 enable */
	value = readl(SUNXI_PMU_AON_BASE + PMU_AON_SETTIME_REG1);
	value &= ~PMU_AON_EN2_SETTIME_MASK;
	value |= PMU_AON_EN2_WAIT_TIME_US << PMU_AON_EN2_SETTIME_OFFSET;
	writel(value, SUNXI_PMU_AON_BASE + PMU_AON_SETTIME_REG1);
}

#define SUNXI_RTC_GPIO_PL_CFG0             0x42000540
#define SUNXI_RTC_GPIO_PL_DAT              0x42000550
#define SUNXI_RTC_IO_HOLD_CTRL             0x4a00005c

int sunxi_board_init(void)
{
	u32 value = 0;

	if (sunxi_chip_alter_version() == SUNXI_CHIP_ALTER_VERSION_V821B) {
		/* set LDO_RTC_0V9, in hibernation is 0.795V, not in hibernation is 0.705V */
		value = 0x0d07;
		writel(value, SUNXI_PMU_RTC_BASE + 0x08);
#ifndef CFG_V821B_CHIP
		// Check chip and firmware
		for (value = 0; value < 20; value++)
			pr_emerg("ERROR: The detected chip model is V821B, Please do not use V821 firmware\n");
		asm volatile("j .");
#endif
	} else {
		/* set LDO_RTC_0V9, both in hibernation and not in hibernation is 0.795V */
		value = 0x0d0d;
		writel(value, SUNXI_PMU_RTC_BASE + 0x08);
#ifndef CFG_CHIP_V821
		// Check chip and firmware
		for (value = 0; value < 20; value++)
			pr_emerg("ERROR: The detected chip model is V821, Please do not use V821B firmware\n");
		asm volatile("j .");
#endif
	}

	/* enbale rtc vccio detected */
	rtc_set_vccio_det_spare();

	sunxi_board_pll_init();

	pmu_aon_set_en2_wait_time();

	fastboot_info_clear();

	sunxi_gpadc_ldo_enable();

	sunxi_rtc_init();
#ifdef CFG_SUNXI_POWER
	axp_init(0);
	/*
	 * When waking up, don't change the voltage, frequency,
	 * cpu is easy to hang. We restore it in atf.
	 */

	set_pll_voltage(CONFIG_SUNXI_CORE_VOL);
	set_sys_voltage(CONFIG_SUNXI_SYS_VOL);
#endif

	show_hardware_source();

#ifdef CFG_ENABLE_LDO28
	u32 reg_data;

	reg_data = readl(SUNXI_PMU_RTC_BASE + 0x10);
	reg_data |= BIT(0);
	writel(reg_data, SUNXI_PMU_RTC_BASE + 0x10);
	udelay(120);
#endif
	if (sunxi_chip_alter_version() == SUNXI_CHIP_ALTER_VERSION_V821B)
		set_resistor_calibration();

	/* TODO: there is a risk of data errors
	 * in reading the sid sram after ultra-standby,
	 * and this function is before called dram_init
	 * so can't read from sid ddr base.
	 */
#ifndef CFG_SUNXI_PMBOOT
#ifdef CFG_SUNXI_PL_1V8
	/* set pl to 1.8v when used battery*/
	value = readl(SUNXI_PMU_RTC_BASE + 0x00);
	value &= ~(0x1);
	writel(value, SUNXI_PMU_RTC_BASE + 0x00);
	udelay(313);
#else
	value = readl(SUNXI_PMU_RTC_BASE + 0x00);
	value |= 0x1;
	writel(value, SUNXI_PMU_RTC_BASE + 0x00);
	udelay(313);
#endif

#define EFUSE_BROM              (0x4c)
	volatile unsigned long efuse;
	int time_out = 0;

	efuse = (readl(SID_EFUSE + EFUSE_BROM) & 0x60000000) >> 29;
	if (efuse) {
		/* efuse = 0x3: have battery */
		/* efuse > 0:    error no, used battery function */
#ifdef CFG_PMC_EN2_FOR_PWRCTRL
		value = readl(SUNXI_PRCM_BASE + 0x70);
		/* set pl2 pmc_en2, no pull */
		value &= ~(0x3 << 26);
		value &= ~(0x3 << 20);
		writel(value, SUNXI_PRCM_BASE + 0x70);
#endif
		value = readl(SUNXI_PWR_REQ_BASE + 0x00);
		value |= 0x4;
		writel(value, SUNXI_PWR_REQ_BASE + 0x00);

		while (!(readl(SUNXI_PWR_REQ_BASE + 0x08) & (0x1 << 2))) {
			time_out++;
			udelay(1);
			if (time_out > 20 * 1000) {
				printf("set dcdc1_5 time out\n");
				/* set jtag by pinctrl */
				while (1)
					;
			}
		}
#ifdef CFG_PMC_PL6_FOR_PWRCTRL
		/* default v821-perf2-b board using PL6 HIGH control DRAM Power */
		value = readl(SUNXI_RTC_IO_HOLD_CTRL);
		value &= ~(0x1 << 6);
		writel(value, SUNXI_RTC_IO_HOLD_CTRL); //clear gpio L6 io hold
		udelay(125); //delay 125us for hardware handle

		value = readl(SUNXI_RTC_GPIO_PL_DAT);
		value |= (0x1 << 6);
		writel(value, SUNXI_RTC_GPIO_PL_DAT); // set gpio L6 output 1

		value = readl(SUNXI_RTC_GPIO_PL_CFG0);
		value &= (~(0xf << 24));
		value |= (0x1 << 24);
		writel(value, SUNXI_RTC_GPIO_PL_CFG0); // set gpio L6 output
#endif
#ifdef CFG_PMC_PL7_FOR_PWRCTRL
		/* v821-ver board using PL7 LOW control DRAM Power */
		value = readl(SUNXI_RTC_IO_HOLD_CTRL);
		value &= ~(0x1 << 7);
		writel(value, SUNXI_RTC_IO_HOLD_CTRL); //clear gpio L7 io hold
		udelay(125); //delay 125us for hardware handle

		value = readl(SUNXI_RTC_GPIO_PL_DAT);
		value &= (~(0x1 << 7));
		writel(value, SUNXI_RTC_GPIO_PL_DAT); // set gpio L7 output 0

		value = readl(SUNXI_RTC_GPIO_PL_CFG0);
		value &= (~(0xf << 28));
		value |= (0x1 << 28);
		writel(value, SUNXI_RTC_GPIO_PL_CFG0); // set gpio L7 output
#endif
		mdelay(1);
	}

#endif

	if (sunxi_chip_alter_version() != SUNXI_CHIP_ALTER_VERSION_V821B) {
#ifdef	CFG_SUNXI_SPINOR
		/* Clean Warmboot Flag
		* After successfully booting with 60M, record 60M/1M=60 in bit[27:24]
		* of GPRCM 0x258; However, the 4bit can only record the lower four
		* digits of 60 (0x111100), so it is recorded as 12M, so if the reset boot,
		* it will use 12M for boot
		*/
		if (((readl(SUNXI_PRCM_BASE + 0x258) >> 28) & 0x3) == 0x3)
			writel(readl(SUNXI_PRCM_BASE + 0x258) & ~(0xf << 24),
					SUNXI_PRCM_BASE + 0x258);
#endif
	}

	printf("board init ok: use hosc %dM\n", aw_get_hosc_freq());
	return 0;
}
#else
int sunxi_board_init(void)
{
	printf("FPGA board init ok\n");
	/*
	 * @TODO:
	 * For temporary use only
	 * later bitfile resets will not reinitialize DDR
	 * then can delete this delay operation
	 */
	mdelay(5000);
	return 0;
}

#ifdef CFG_SUNXI_FPGA_BTIFILE_INIT_DRAM
int sunxi_fpga_bitfile_init_dram(void)
{
	return 128;
}
#endif

#endif

uint8_t sunxi_board_late_init(void)
{
#ifdef CFG_SUNXI_POWER
	if (get_pmu_exist() <= 0) {
#ifdef CFG_SUNXI_TWI
		 i2c_exit();
#endif
	}
#endif
#ifdef CFG_SUNXI_KEY_PROVISION
	sunxi_key_provision();
#endif

#ifdef SUNXI_EFUSE_RAM_BASE_IN_DDR
#ifndef CFG_SUNXI_PMBOOT
	sunxi_efuse_backup_to_ddr();
#endif
#endif

#ifndef CFG_SUNXI_PMBOOT
#ifdef CFG_EARLY_BOOT_RISCV
#ifdef CFG_SUNXI_GPT
	if (init_gpt()) {
		printf("init GPT fail\n");
		return -1;
	}
	if (fw_env_open() == -1) {
		printf("init env fail\n");
		return -1;
	}
#endif
	early_fastboot_boot_riscv(NULL);
#endif
#endif
	return 0;
}

int sunxi_board_exit(void)
{
	return 0;
}

#if defined(CFG_AMP_DELEG_SPIF_IRQ) && !defined(CFG_SUNXI_SPIF)
#error "amp deleg spif irq only support spif storage"
#endif

#ifdef CFG_RISCV_E907
void sunxi_e907_clock_init(u32 addr);
void sunxi_e907_clock_reset(void);

void boot_e907(phys_addr_t base, unsigned long fdt_addr)
{
	u32 run_addr = 0;

	(void)fdt_addr;

/* ISP needs, after boot0 runs successfully,
 * you need to set the flag on rtc, and clear
 * it in advance to prevent the small core from
 * reading in advance
 */

#if CFG_BOOT0_LOAD_ISPPARM
/* Specific to fastboot*/
#ifdef CFG_SUNXI_SDMMC
	int (*flash_read)(uint, uint, void *);
	flash_read = mmc_bread_ext;
#elif CFG_SUNXI_SPINOR
	int (*flash_read)(uint, uint, void *);
	flash_read = spinor_read;
#elif CFG_SUNXI_SPINAND
extern int spinand_read_kernel(uint sector_num, uint N, void *buffer);
	int (*flash_read)(uint, uint, void *);
	flash_read = spinand_read_kernel;
#endif /*sdmmc*/
	if (load_isp_param(flash_read) != 0) {
		printf("read isp param failed!!!\n");
	}
	extern void update_isp_param(void);
	update_isp_param();
#endif /* CFG_BOOT0_LOAD_ISPPARM */

#if defined(CFG_BOOT0_WIRTE_RTC_TO_ISP) && !defined(CFG_SUNXI_SIMPLEBOOT)
	/* it will be set in opensbi */
	rtc_clear_isp_flag();
#elif defined(CFG_BOOT0_WIRTE_RTC_TO_ISP) && defined(CFG_SUNXI_SIMPLEBOOT)
	/* sram already release */
	rtc_set_isp_flag();
#endif

#ifdef CFG_AMP_DELEG_SPIF_IRQ
	{
		u32 val = rtc_read_data(CFG_DELEG_SPIF_IRQ_RTC_IDX);

		val |= (1 << CFG_DELEG_SPIF_IRQ_RTC_BIT);
#ifndef CFG_SUNXI_PMBOOT
		/* reboot or cold boot */
		val &= ~(1 << CFG_SPIF_IRQ_RDY_RTC_BIT);
#endif
		rtc_set_data(CFG_DELEG_SPIF_IRQ_RTC_IDX, val);
		rtc_clear_data(CFG_SPIF_RECORD_RTC_IDX);
	}
#endif

#ifndef CFG_AMP_BOOT0_RUN_ADDR
	/* assert e907 */
	sunxi_e907_clock_reset();
#endif
	/* get boot address */
	run_addr = elf_get_entry_addr(base);
	/* load elf to target address */
	load_elf_image(base);
	flush_dcache_all();
	/* de-assert e907 */
	sunxi_e907_clock_init(run_addr);
}
#endif

int load_rtos_partition(char *name, phys_addr_t load_base)
{
#if defined(CFG_SUNXI_SPINAND) && defined(CFG_NAND_CPUS_LOAD)
extern int spinand_read_kernel(uint sector_num, uint N, void *buffer);
#if defined(CFG_NAND_RISCV_START) && defined(CFG_NAND_RISCV_SIZE)
	uint start_sector = CFG_NAND_RISCV_START / 512;
	uint sector_num = CFG_NAND_RISCV_SIZE / 512;
#else
	uint start_sector = 0;
	uint sector_num = 0;
#endif

	u8  *rtos_image_base = (u8 *)load_base;
	flash_init();

	if (spinand_read_kernel(start_sector, sector_num, (void *)rtos_image_base)) {
		printf("read %s raw partition error\n", name);
		return -1;
	}
#else /* CFG_SUNXI_SPINAND && CFG_NAND_CPUS_LOAD */
#ifdef CFG_SUNXI_GPT
	uint start_sector;
	uint sector_num;
	u8  *rtos_image_base = (u8 *)load_base;

	if (init_gpt()) {
		printf("init GPT fail\n");
		return -1;
	}
	if (get_part_info_by_name(name, &start_sector, &sector_num)) {
		return -1;
	}
#ifdef CFG_SUNXI_ENV
	if (spl_flash_read(start_sector, sector_num, (void *)rtos_image_base)) {
		printf("read %s partition error\n", name);
		return -1;
	}
#endif
#endif
#endif /* CFG_SUNXI_SPINAND && CFG_NAND_CPUS_LOAD */
	return 0;
}

#ifdef CFG_SUNXI_SET_EFUSE_POWER
void sboot_set_efuse_voltage(int status)
{
	uint reg_val;

	if (status) {
		value = readl(SUNXI_PWR_REQ_BASE + 0x00);
		value &= ~(0x8);
		writel(value, SUNXI_PWR_REQ_BASE + 0x00);
		while (readl(SUNXI_PWR_REQ_BASE + 0x08) & 0x08)
			;

		value = readl(SUNXI_PMU_AON_BASE + 0x84);
		value |= 0x1;
		writel(value, SUNXI_PMU_AON_BASE + 0x84);
		value = readl(SUNXI_PWR_REQ_BASE + 0x00);
		value |= 0x8;
		writel(value, SUNXI_PWR_REQ_BASE + 0x00);

		udelay(200);
		while (!(readl(SUNXI_PWR_REQ_BASE + 0x08) & 0x08))
			;

	} else {
		value = readl(SUNXI_PWR_REQ_BASE + 0x00);
		value &= ~(0x8);
		writel(value, SUNXI_PWR_REQ_BASE + 0x00);
		while (readl(SUNXI_PWR_REQ_BASE + 0x08) & 0x08)
			;

		value = readl(SUNXI_PMU_AON_BASE + 0x84);
		value |= 0x1;
		writel(value, SUNXI_PMU_AON_BASE + 0x84);
		value = readl(SUNXI_PWR_REQ_BASE + 0x00);
		value |= 0x8;
		writel(value, SUNXI_PWR_REQ_BASE + 0x00);

		udelay(200);
		while (!(readl(SUNXI_PWR_REQ_BASE + 0x08) & 0x08))
			;

		value = readl(SUNXI_PMU_AON_BASE + 0x84);
		value &= ~(0x1);
		writel(value, SUNXI_PMU_AON_BASE + 0x84);
		value = readl(SUNXI_RPRCM_BASE + 0x88);
		value |= 0x01;
		writel(value, SUNXI_RPRCM_BASE + 0x88);

	}
}
#endif

