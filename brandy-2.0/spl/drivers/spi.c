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
 * sunxi SPI driver for uboot.
 *
 * Copyright (C) 2018
 * 2013.5.7  Mintow <duanmintao@allwinnertech.com>
 * 2018.11.7 wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <arch/gpio.h>
#include <arch/clock.h>
#include <arch/spinor.h>
#include <arch/dma.h>
#include "spi-sunxi.h"
#include <private_boot0.h>

#ifdef CFG_SPI_USE_DMA
static sunxi_dma_set *spi_rx_dma;
static uint spi_rx_dma_hd;
__attribute__((section(".data")))
sunxi_dma_set g_spi_rx_dma;
#endif

#define CONFIG_SF_DEFAULT_BUS               0
#define	SUNXI_SPI_MAX_TIMEOUT	1000000
#define	SUNXI_SPI_PORT_OFFSET	0x1000
#define SUNXI_SPI_DEFAULT_CLK  (50000000)

/* For debug */
#define SPI_DEBUG 0

#if SPI_DEBUG
#define SPI_EXIT()		printf("%s()%d - %s\n", __func__, __LINE__, "Exit")
#define SPI_ENTER()		printf("%s()%d - %s\n", __func__, __LINE__, "Enter ...")
#define SPI_DBG(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SPI_INF(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SPI_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)

#else
#define SPI_EXIT()
#define SPI_ENTER()
#define SPI_DBG(fmt, arg...)
#define SPI_INF(fmt, arg...)
#define SPI_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#endif
#define SUNXI_SPI_OK   0
#define SUNXI_SPI_FAIL -1

extern void sunxi_dma_reg_func(void *p);
static int spi_clk = SUNXI_SPI_DEFAULT_CLK;
extern boot_spinor_info_t *spinor_info;

#if SPI_DEBUG
int snprintf(char *buf, size_t size, const char *fmt, ...);
static void spi_print_info(void __iomem *base_addr)
{
	char buf[1024] = {0};
	snprintf(buf, sizeof(buf)-1,
			"base_addr = 0x%x, the SPI control register:\n"
			"[VER] 0x%02x = 0x%08x, [GCR] 0x%02x = 0x%08x, [TCR] 0x%02x = 0x%08x\n"
			"[ICR] 0x%02x = 0x%08x, [ISR] 0x%02x = 0x%08x, [FCR] 0x%02x = 0x%08x\n"
			"[FSR] 0x%02x = 0x%08x, [WCR] 0x%02x = 0x%08x, [CCR] 0x%02x = 0x%08x\n"
			"[DCR] 0x%02x = 0x%08x, [BCR] 0x%02x = 0x%08x, [TCR] 0x%02x = 0x%08x\n"
			"[BCC] 0x%02x = 0x%08x, [DMA] 0x%02x = 0x%08x",
			base_addr,
			SPI_VER_REG, readl(base_addr + SPI_VER_REG),
			SPI_GC_REG, readl(base_addr + SPI_GC_REG),
			SPI_TC_REG, readl(base_addr + SPI_TC_REG),
			SPI_INT_CTL_REG, readl(base_addr + SPI_INT_CTL_REG),
			SPI_INT_STA_REG, readl(base_addr + SPI_INT_STA_REG),

			SPI_FIFO_CTL_REG, readl(base_addr + SPI_FIFO_CTL_REG),
			SPI_FIFO_STA_REG, readl(base_addr + SPI_FIFO_STA_REG),
			SPI_WAIT_CNT_REG, readl(base_addr + SPI_WAIT_CNT_REG),
			SPI_CLK_CTL_REG, readl(base_addr + SPI_CLK_CTL_REG),
			SPI_SDC_REG, readl(base_addr + SPI_SDC_REG),

			SPI_BURST_CNT_REG, readl(base_addr + SPI_BURST_CNT_REG),
			SPI_TRANSMIT_CNT_REG, readl(base_addr + SPI_TRANSMIT_CNT_REG),
			SPI_BCC_REG, readl(base_addr + SPI_BCC_REG),
			SPI_DMA_CTL_REG, readl(base_addr + SPI_DMA_CTL_REG));
			printf("%s\n", buf);
}
#endif


static u32 clock_get_pll6(void)
{
#if defined(CONFIG_ARCH_SUN55IW6) || defined(CONFIG_ARCH_SUN300IW1)
	return	SPI_CLK_PLL_PERI0;
#else
	return 600000000;
#endif
}

static u32 clock_source_selection(void)
{
#if defined(CONFIG_ARCH_SUN55IW6)
	return 2;
#elif defined(FPGA_PLATFORM)
	return 0;
#else
	return 1;
#endif
}

/* config chip select */
static s32 spi_set_cs(u32 chipselect, void __iomem *base_addr)
{
	int ret;
	u32 reg_val = readl(base_addr + SPI_TC_REG);

	if (chipselect < 4) {
		reg_val &= ~SPI_TC_SS_MASK;/* SS-chip select, clear two bits */
		reg_val |= chipselect << SPI_TC_SS_BIT_POS;/* set chip select */
		writel(reg_val, base_addr + SPI_TC_REG);
		ret = SUNXI_SPI_OK;
	} else {
		SPI_ERR("Chip Select set fail! cs = %d\n", chipselect);
		ret = SUNXI_SPI_FAIL;
	}

	return ret;
}

static void spi_set_sdm(void __iomem *base_addr, unsigned int smod)
{
	unsigned int rval = readl(base_addr + SPI_TC_REG) & (~SPI_TC_SDM);

	if (smod)
		rval |= SPI_TC_SDM;
	writel(rval, base_addr + SPI_TC_REG);
}

static void spi_set_sdc(void __iomem *base_addr, unsigned int sample)
{
	unsigned int rval = readl(base_addr + SPI_TC_REG) & (~SPI_TC_SDC);

	if (sample)
		rval |= SPI_TC_SDC;
	writel(rval, base_addr + SPI_TC_REG);
}

static void spi_set_sdc1(void __iomem *base_addr, unsigned int sample)
{
	unsigned int rval = readl(base_addr + SPI_TC_REG) & (~SPI_TC_SDC1);

	if (sample)
		rval |= SPI_TC_SDC1;
	writel(rval, base_addr + SPI_TC_REG);
}

void spi_set_sample_mode(void __iomem *base_addr, unsigned int mode)
{
	unsigned int sample_mode[7] = {
		DELAY_NORMAL_SAMPLE, DELAY_0_5_CYCLE_SAMPLE,
		DELAY_1_CYCLE_SAMPLE, DELAY_1_5_CYCLE_SAMPLE,
		DELAY_2_CYCLE_SAMPLE, DELAY_2_5_CYCLE_SAMPLE,
		DELAY_3_CYCLE_SAMPLE
	};
	spi_set_sdm(base_addr, (sample_mode[mode] >> 8) & 0xf);
	spi_set_sdc(base_addr, (sample_mode[mode] >> 4) & 0xf);
	spi_set_sdc1(base_addr, sample_mode[mode] & 0xf);
}

void spi_set_sample_delay(void __iomem  *base_addr,
		unsigned int sample_delay)
{
	unsigned int rval = readl(base_addr + SPI_SDC_REG)&(~(0x3f << 0));

	rval |= sample_delay;
	writel(rval, base_addr + SPI_SDC_REG);
	mdelay(1);
}

void spi_samp_dl_sw_status(void __iomem  *base_addr, unsigned int status)
{
	unsigned int rval = readl(base_addr + SPI_SDC_REG);

	if (status)
		rval |= SPI_SAMP_DL_SW_EN;
	else
		rval &= ~SPI_SAMP_DL_SW_EN;

	writel(rval, base_addr +  SPI_SDC_REG);
}

void spi_samp_mode(void __iomem *base_addr, unsigned int status)
{
	unsigned int rval = readl(base_addr + SPI_GC_REG);

	if (status)
		rval |= SPI_SAMP_MODE_EN;
	else
		rval &= ~SPI_SAMP_MODE_EN;

	writel(rval, base_addr + SPI_GC_REG);
}

static int sunxi_spi_clk_init(u32 bus, u32 mod_clk)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	u32 mclk_base = (((phys_addr_t)&ccm->spi0_clk_cfg) & 0xffffffff) + bus * 0x4;
	u32 rval;
	__maybe_unused u32 source_clk = 0;
	__maybe_unused u32 m, n, div, factor_m;

	void __iomem *spi_base = sunxi_get_iobase(mclk_base);
	/* SCLK = src/M/N */
	/* N: 00:1 01:2 10:4 11:8 */
	/* M: factor_m + 1 */
#ifdef FPGA_PLATFORM
	n = 0;
	m = 1;
	div = 1;
	factor_m = m - 1;
	rval = (1U << 31) | ((clock_source_selection()) << 24) | (n << 8) | factor_m;
	source_clk = 24000000;
#else
	source_clk = clock_get_pll6();/*pll6, fix me*/
	SPI_INF("source_clk: %d Hz\n", source_clk);

	div = (source_clk + mod_clk - 1) / mod_clk;
	div = div == 0 ? 1 : div;
	if (div > 128) {
		m = 1;
		n = 0;
		return -1;
	} else if (div > 64) {
		n = 3;
		m = div >> 3;
	} else if (div > 32) {
		n = 2;
		m = div >> 2;
	} else if (div > 16) {
		n = 1;
		m = div >> 1;
	} else {
		n = 0;
		m = div;
	}

	factor_m = m - 1;
#if defined(CONFIG_ARCH_SUN8IW11) || defined(CONFIG_ARCH_SUN300IW1)
	rval = (1U << 31) | ((clock_source_selection()) << 24) | (n << 16) | factor_m;
#else
	rval = (1U << 31) | ((clock_source_selection()) << 24) | (n << 8) | factor_m;
#endif
	/**sclk_freq = source_clk / (1 << n) / m; */
#endif
	SPI_INF("source_clk : %d Hz, div =%d, n= %d, m=%d\n", source_clk, div, n, m);
	writel(rval, spi_base);

#if defined(CONFIG_ARCH_SUN8IW11) || defined(CONFIG_ARCH_SUN300IW1)
	/* spi reset */
	writel(readl(&ccm->ahb_reset0_cfg) & ~(1 << GATING_RESET_SHIFT), &ccm->ahb_reset0_cfg);
	writel(readl(&ccm->ahb_reset0_cfg) | (1 << GATING_RESET_SHIFT), &ccm->ahb_reset0_cfg);

	/* spi gating */
	writel(readl(&ccm->ahb_gate0) | (1 << GATING_RESET_SHIFT), &ccm->ahb_gate0);
#else
	/* spi reset */
	setbits_le32(&ccm->spi_gate_reset, (0<<RESET_SHIFT));
	setbits_le32(&ccm->spi_gate_reset, (1<<RESET_SHIFT));

	/* spi gating */
	setbits_le32(&ccm->spi_gate_reset, (1<<GATING_SHIFT));
#endif

	return 0;
}

static int sunxi_get_spic_clk(int bus)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	u32 mclk_base = (((phys_addr_t)&ccm->spi0_clk_cfg) & 0xffffffff) + bus * 0x4;
	u32 reg_val = 0;
	u32 src = 0, clk = 0, sclk_freq = 0;
	u32 n, m;
	void __iomem *spi_base = sunxi_get_iobase(mclk_base);

	reg_val = readl(spi_base);
	src = (reg_val >> 24)&0x7;
	n = (reg_val >> 8)&0x3;
	m = ((reg_val >> 0)&0xf) + 1;

	switch (src) {
	case 0:
		clk = 24000000;
		break;
	case 1:
	case 2:
		clk = clock_get_pll6();
		break;
	default:
		clk = 0;
		break;
	}
	sclk_freq = clk / (1 << n) / m;
	SPI_INF("sclk_freq= %d Hz,reg_val: %x , n=%d, m=%d\n", sclk_freq, reg_val, n, m);
	return sclk_freq;

}

/* config spi */
static void spi_config_tc(u32 master, u32 config, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_TC_REG);
	uint sclk_freq = 0;

	sclk_freq = sunxi_get_spic_clk(0);

	if (config || !spinor_info || spinor_info->sample_delay ==
			SAMP_MODE_DL_DEFAULT) {
		if (sclk_freq >= 60000000) {
			reg_val &= ~(SPI_TC_SDC | SPI_TC_SDM);
			reg_val |= SPI_TC_SDC;
		} else if (sclk_freq <= 24000000) {
			reg_val &= ~(SPI_TC_SDC | SPI_TC_SDM);
			reg_val |= SPI_TC_SDM;
		} else
			reg_val &= ~(SPI_TC_SDC | SPI_TC_SDM);
	} else {
		spi_set_sample_mode(base_addr, spinor_info->sample_mode);
		spi_set_sample_delay(base_addr, spinor_info->sample_delay);
		reg_val = readl(base_addr + SPI_TC_REG);
	}

	reg_val |= SPI_TC_DHB | SPI_TC_SS_LEVEL | SPI_TC_SPOL;
	writel(reg_val, base_addr + SPI_TC_REG);

#if 0
	/*1. POL */
	if (config & SPI_POL_ACTIVE_)
		reg_val |= SPI_TC_POL;/*default POL = 1 */
	else
		reg_val &= ~SPI_TC_POL;

	/*2. PHA */
	if (config & SPI_PHA_ACTIVE_)
		reg_val |= SPI_TC_PHA;/*default PHA = 1 */
	else
		reg_val &= ~SPI_TC_PHA;

	/*3. SSPOL,chip select signal polarity */
	if (config & SPI_CS_HIGH_ACTIVE_)
		reg_val &= ~SPI_TC_SPOL;
	else
		reg_val |= SPI_TC_SPOL; /*default SSPOL = 1,Low level effect */

	/*4. LMTF--LSB/MSB transfer first select */
	if (config & SPI_LSB_FIRST_ACTIVE_)
		reg_val |= SPI_TC_FBS;
	else
		reg_val &= ~SPI_TC_FBS;/*default LMTF =0, MSB first */

	/*master mode: set DDB,DHB,SMC,SSCTL*/
	if (master == 1) {
		/*5. dummy burst type */
		if (config & SPI_DUMMY_ONE_ACTIVE_)
			reg_val |= SPI_TC_DDB;
		else
			reg_val &= ~SPI_TC_DDB;/*default DDB =0, ZERO */

		/*6.discard hash burst-DHB */
		if (config & SPI_RECEIVE_ALL_ACTIVE_)
			reg_val &= ~SPI_TC_DHB;
		else
			reg_val |= SPI_TC_DHB;/*default DHB =1, discard unused burst */

		/*7. set SMC = 1 , SSCTL = 0 ,TPE = 1 */
		reg_val &= ~SPI_TC_SSCTL;
	} else {
		/* tips for slave mode config */
		SPI_INF("slave mode configurate control register.\n");
	}
#endif

}

/* set spi clock */
static void spi_set_clk(u32 spi_clk, u32 ahb_clk, void __iomem *base_addr, u32 cdr)
{
	u32 reg_val = 0;
	u32 div_clk = 0;

	SPI_DBG("set spi clock %d, mclk %d\n", spi_clk, ahb_clk);
	reg_val = readl(base_addr + SPI_CLK_CTL_REG);

	/* CDR2 */
	if (cdr) {
		div_clk = ahb_clk / (spi_clk * 2) - 1;
		reg_val &= ~SPI_CLK_CTL_CDR2;
		reg_val |= (div_clk | SPI_CLK_CTL_DRS);
		SPI_DBG("CDR2 - n = %d\n", div_clk);
	} else { /* CDR1 */
		while (ahb_clk > spi_clk) {
			div_clk++;
			ahb_clk >>= 1;
		}
		reg_val &= ~(SPI_CLK_CTL_CDR1 | SPI_CLK_CTL_DRS);
		reg_val |= (div_clk << 8);
		SPI_DBG("CDR1 - n = %d\n", div_clk);
	}

	writel(reg_val, base_addr + SPI_CLK_CTL_REG);
}

/* start spi transfer */
static void spi_start_xfer(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_TC_REG);

	reg_val |= SPI_TC_XCH;
	writel(reg_val, base_addr + SPI_TC_REG);
}

/* enable spi bus */
static void spi_enable_bus(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_GC_REG);

	reg_val |= SPI_GC_EN;
	writel(reg_val, base_addr + SPI_GC_REG);
}

/* disbale spi bus */
static void spi_disable_bus(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_GC_REG);

	reg_val &= ~SPI_GC_EN;
	writel(reg_val, base_addr + SPI_GC_REG);
}

/* set master mode */
static void spi_set_master(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_GC_REG);

	reg_val |= SPI_GC_MODE;
	writel(reg_val, base_addr + SPI_GC_REG);
}

/* enable transmit pause */
static void spi_enable_tp(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_GC_REG);

	reg_val |= SPI_GC_TP_EN;
	writel(reg_val, base_addr + SPI_GC_REG);
}

/* soft reset spi controller */
static void spi_soft_reset(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_GC_REG);

	reg_val |= SPI_GC_SRST;
	writel(reg_val, base_addr + SPI_GC_REG);
}

#if 0
/* enable irq type */
static void spi_enable_irq(u32 bitmap, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_INT_CTL_REG);

	bitmap &= SPI_INTEN_MASK;
	reg_val |= bitmap;
	writel(reg_val, base_addr + SPI_INT_CTL_REG);
}
#endif

/* disable irq type */
static void spi_disable_irq(u32 bitmap, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_INT_CTL_REG);

	bitmap &= SPI_INTEN_MASK;
	reg_val &= ~bitmap;
	writel(reg_val, base_addr + SPI_INT_CTL_REG);
}

/* query irq pending */
static u32 spi_qry_irq_pending(void __iomem *base_addr)
{
	return (SPI_INT_STA_MASK & readl(base_addr + SPI_INT_STA_REG));
}

/* clear irq pending */
static void spi_clr_irq_pending(u32 pending_bit, void __iomem *base_addr)
{
	pending_bit &= SPI_INT_STA_MASK;
	writel(pending_bit, base_addr + SPI_INT_STA_REG);
}

/* query txfifo bytes */
static u32 spi_query_txfifo(void __iomem *base_addr)
{
	u32 reg_val = (SPI_FIFO_STA_TX_CNT & readl(base_addr + SPI_FIFO_STA_REG));

	reg_val >>= SPI_TXCNT_BIT_POS;
	return reg_val;
}

/* query rxfifo bytes */
static u32 spi_query_rxfifo(void __iomem *base_addr)
{
	u32 reg_val = (SPI_FIFO_STA_RX_CNT & readl(base_addr + SPI_FIFO_STA_REG));

	reg_val >>= SPI_RXCNT_BIT_POS;
	return reg_val;
}

/* reset fifo */
static void spi_reset_fifo(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_FIFO_CTL_REG);

	reg_val |= (SPI_FIFO_CTL_RX_RST|SPI_FIFO_CTL_TX_RST);
	/* Set the trigger level of RxFIFO/TxFIFO. */
	reg_val &= ~(SPI_FIFO_CTL_RX_LEVEL|SPI_FIFO_CTL_TX_LEVEL);
	reg_val |= (0x20<<16) | 0x20;
	writel(reg_val, base_addr + SPI_FIFO_CTL_REG);
}

/* set transfer total length BC, transfer length TC and single transmit length STC */
static void spi_set_bc_tc_stc(u32 tx_len, u32 rx_len, u32 stc_len, u32 dummy_cnt, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPI_BURST_CNT_REG);

	reg_val &= ~SPI_BC_CNT_MASK;
	reg_val |= (SPI_BC_CNT_MASK & (tx_len + rx_len + dummy_cnt));
	writel(reg_val, base_addr + SPI_BURST_CNT_REG);

	reg_val = readl(base_addr + SPI_TRANSMIT_CNT_REG);
	reg_val &= ~SPI_TC_CNT_MASK;
	reg_val |= (SPI_TC_CNT_MASK & tx_len);
	writel(reg_val, base_addr + SPI_TRANSMIT_CNT_REG);

	reg_val = readl(base_addr + SPI_BCC_REG);
	reg_val &= ~SPI_BCC_STC_MASK;
	reg_val |= (SPI_BCC_STC_MASK & stc_len);
	reg_val &= ~(0xf << 24);
	reg_val |= (dummy_cnt << 24);
	writel(reg_val, base_addr + SPI_BCC_REG);
}

/* set ss control */
static void spi_ss_owner(void __iomem *base_addr, u32 on_off)
{
	u32 reg_val = readl(base_addr + SPI_TC_REG);

	on_off &= 0x1;
	if (on_off)
		reg_val |= SPI_TC_SS_OWNER;
	else
		reg_val &= ~SPI_TC_SS_OWNER;
	writel(reg_val, base_addr + SPI_TC_REG);
}

/* set ss level */
static void spi_ss_level(void __iomem *base_addr, u32 hi_lo)
{
	u32 reg_val = readl(base_addr + SPI_TC_REG);

	hi_lo &= 0x1;
	if (hi_lo)
		reg_val |= SPI_TC_SS_LEVEL;
	else
		reg_val &= ~SPI_TC_SS_LEVEL;
	writel(reg_val, base_addr + SPI_TC_REG);
}


#if 1
static void spi_disable_dual(void __iomem  *base_addr)
{
	u32 reg_val = readl(base_addr+SPI_BCC_REG);
	reg_val &= ~SPI_BCC_DUAL_MODE;
	writel(reg_val, base_addr + SPI_BCC_REG);
}

static void spi_enable_dual(void __iomem  *base_addr)
{
	u32 reg_val = readl(base_addr+SPI_BCC_REG);
	reg_val &= ~SPI_BCC_QUAD_MODE;
	reg_val |= SPI_BCC_DUAL_MODE;
	writel(reg_val, base_addr + SPI_BCC_REG);
}

static void spi_disable_quad(void __iomem  *base_addr)
{
	u32 reg_val = readl(base_addr+SPI_BCC_REG);

	reg_val &= ~SPI_BCC_QUAD_MODE;
	writel(reg_val, base_addr + SPI_BCC_REG);
}

static void spi_enable_quad(void __iomem  *base_addr)
{
	u32 reg_val = readl(base_addr+SPI_BCC_REG);

	reg_val |= SPI_BCC_QUAD_MODE;
	writel(reg_val, base_addr + SPI_BCC_REG);
}


static int sunxi_spi_mode_check(void __iomem  *base_addr, u32 tcnt, u32 rcnt, u8 cmd)
{
	/* single mode transmit counter*/
	unsigned int stc = 0;
	if (SPINOR_OP_READ_1_1_4 == cmd || SPINOR_OP_READ4_1_1_4 == cmd) {
		/*tcnt is cmd len, use single mode to transmit*/
		/*rcnt is  the len of recv data, use quad mode to transmit*/
		stc = tcnt;
		spi_enable_quad(base_addr);
		spi_set_bc_tc_stc(tcnt, rcnt, stc, 0, base_addr);
	} else if (SPINOR_OP_READ_1_1_2 == cmd || SPINOR_OP_READ4_1_1_2 == cmd) {
		/*tcnt is cmd len, use single mode to transmit*/
		/*rcnt is  the len of recv data, use dual mode to transmit*/
		stc = tcnt;
		spi_enable_dual(base_addr);
		spi_set_bc_tc_stc(tcnt, rcnt, stc, 0, base_addr);
	} else {
		/*tcnt is the len of cmd, rcnt is the len of recv data. use single mode to transmit*/
		stc = tcnt+rcnt;
		spi_disable_dual(base_addr);
		spi_disable_quad(base_addr);
		spi_set_bc_tc_stc(tcnt, rcnt, stc, 0, base_addr);
	}

	return 0;
}

#endif

#define SUNXI_GPIO_POSITION(port, port_num) (32 * (port - 1) + port_num)

static int sunxi_spi_gpio_init(void)
{
#ifndef CFG_SUNXI_SBOOT
	int i = 0;
#if defined(CONFIG_ARCH_SUN8IW11)
	int pin_num = 4;
#else
	int pin_num = 6;
#endif
	if (BT0_head.prvt_head.storage_gpio[i].port) {
		boot_set_gpio(
		(normal_gpio_cfg *)BT0_head.prvt_head.storage_gpio, pin_num, 1);
	} else {
#endif
#if defined(CONFIG_ARCH_SUN50IW3)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(5), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_pull(SUNXI_GPC(5), 1);
#elif defined(CONFIG_ARCH_SUN8IW11)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUNXI_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(23), SUNXI_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUNXI_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(1), SUNXI_GPC_SPI0); /*spi0_miso*/

	sunxi_gpio_set_pull(SUNXI_GPC(23), 1);
#elif defined(CONFIG_ARCH_SUN8IW16)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(1), SUN50I_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(10), SUN50I_GPC_SPI0); /*spi0_cs1*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(13), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(14), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(1), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(10), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(13), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(14), 1);
#elif defined(CONFIG_ARCH_SUN8IW18) || defined(CONFIG_ARCH_SUN50IW9)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN50I_GPC_SPI0); /*spi0_cs1*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(7), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN8IW19) || defined(CONFIG_ARCH_SUN8IW21)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(1), SUN50I_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(5), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(1), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(4), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(5), 1);
#elif defined(CONFIG_ARCH_SUN50IW11)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(1), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(5), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(0), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(2), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(5), 1);
#elif defined(CONFIG_ARCH_SUN50IW12)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN8IW20) || defined(CONFIG_ARCH_SUN20IW1P1)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN20I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN20I_GPC_SPI0); /*spi0_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN20I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(5), SUN20I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(6), SUN20I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN20I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(6), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(7), 1);
#elif defined(CONFIG_ARCH_SUN55IW3)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN55IW5) || defined(CONFIG_ARCH_SUN55IW6) || defined(CONFIG_ARCH_SUN50IW15)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN50I_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN300IW1)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(6), SUNXI_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUNXI_GPC_SPI0); /*spi0_HOLD*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(8), SUNXI_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(9), SUNXI_GPC_SPI0); /*spi0_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(10), SUNXI_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(11), SUNXI_GPC_SPI0); /*spi0_miso*/

	sunxi_gpio_set_pull(SUNXI_GPC(6), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(7), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(10), 1);
#else
	#error "spi pinctrl not available for this architecture"
#endif
#ifndef CFG_SUNXI_SBOOT
	}
#endif
	return 0;
}

static int sunxi_spi_clk_exit(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
#if defined(CONFIG_ARCH_SUN8IW11) || defined(CONFIG_ARCH_SUN300IW1)
	/* clr spi gating */
	writel(readl(&ccm->ahb_gate0) & ~(1 << GATING_RESET_SHIFT), &ccm->ahb_gate0);

	/* clr spi reset */
	writel(readl(&ccm->ahb_reset0_cfg) & ~(1 << GATING_RESET_SHIFT), &ccm->ahb_reset0_cfg);
#else
	/* spi gating */
	clrbits_le32(&ccm->spi_gate_reset, 1<<GATING_SHIFT);

	/* spi reset */
	clrbits_le32(&ccm->spi_gate_reset, 1<<RESET_SHIFT);
#endif

	return 0;
}


static int sunxi_spi_cpu_writel(void *base_addr, const unsigned char *buf, unsigned int len)
{
#ifndef CONFIG_DMA_ENGINE
	unsigned char time;
#endif
	unsigned int tx_len = len;	/* number of bytes receieved */
	unsigned char *tx_buf = (unsigned char *)buf;
	unsigned int poll_time = 0x7ffffff;

#if SPI_DEBUG
	printf("spi tx: %d bytes\n", len);
	/*sunxi_dump(tx_buf, len);*/
#endif
	for (; tx_len > 0; --tx_len) {
		writeb(*tx_buf++, base_addr + SPI_TXDATA_REG);
		if (spi_query_txfifo(base_addr) >= MAX_FIFU)
			for (time = 2; 0 < time; --time)
				;
	}

	while (spi_query_txfifo(base_addr) && (--poll_time > 0))
		;
	if (poll_time <= 0) {
		SPI_ERR("cpu transfer data time out!\n");
		return -1;
	}

	return 0;
}

static int sunxi_spi_cpu_readl(void *base_addr, unsigned char *buf, unsigned int len)
{
	unsigned int rx_len = len;	/* number of bytes sent */
	unsigned char *rx_buf = buf;
	unsigned int poll_time = 0x7ffffff;

	while (rx_len && poll_time ) {
	/* rxFIFO counter */
		if (spi_query_rxfifo(base_addr) && (--poll_time > 0)) {
			*rx_buf++ =  readb(base_addr + SPI_RXDATA_REG);
			--rx_len;
		}
	}
	if (poll_time <= 0) {
		SPI_ERR("cpu receive data time out!\n");
		return -1;
	}
#if SPI_DEBUG
	printf("spi rx: %d bytes\n" , len);
	/*sunxi_dump(buf, len);*/
#endif
	return 0;
}

int spi_claim_bus(void *base_addr)
{
	uint sclk_freq = 0;

	SPI_ENTER();

	sclk_freq = sunxi_get_spic_clk(0);
	if(!sclk_freq)
		return -1;

	spi_soft_reset(base_addr);

	/* 1. enable the spi module */
	spi_enable_bus(base_addr);
	/* 2. set the default chip select */
	spi_set_cs(0, base_addr);

	/* 3. master: set spi module clock;
	 * 4. set the default frequency	10MHz
	 */
	spi_set_master(base_addr);
	spi_set_clk(spi_clk, sclk_freq, base_addr, 0);
	/* 5. master : set POL,PHA,SSOPL,LMTF,DDB,DHB; default: SSCTL=0,SMC=1,TBW=0.
	 * 6. set bit width-default: 8 bits
	 */
	spi_config_tc(1, 1, base_addr);
	spi_ss_level(base_addr, 1);
	spi_enable_tp(base_addr);
	/* 7. spi controller sends ss signal automatically*/
	spi_ss_owner(base_addr, 0);
	/* 8. reset fifo */
	spi_reset_fifo(base_addr);

	return 0;
}

void spi_release_bus(void* base_addr)
{
	SPI_ENTER();
	/* disable the spi controller */
	spi_disable_bus(base_addr);

	/* disable module clock */
	sunxi_spi_clk_exit();

	/*release gpio*/
}

#ifdef CFG_SPI_USE_DMA
static int spi_dma_recv_start(void *spi_addr, uint spi_no, unchar *pbuf, uint byte_cnt)
{
	flush_dcache_range((unsigned long)pbuf, (unsigned long)pbuf + byte_cnt);
	sunxi_dma_start(spi_rx_dma_hd, (phys_addr_t)spi_addr + SPI_RXDATA_REG, (phys_addr_t)pbuf, byte_cnt);
	return 0;
}
static int spi_wait_dma_recv_over(uint spi_no)
{
	return sunxi_dma_querystatus(spi_rx_dma_hd);
}

static int spi_dma_cfg(u32 spi_no)
{
	sunxi_dma_init();

	spi_rx_dma = &g_spi_rx_dma;
	spi_rx_dma_hd = sunxi_dma_request(DMAC_DMATYPE_NORMAL);

	if (spi_rx_dma_hd == 0) {
		printf("spi request dma failed\n");

		return -1;
	}
	/* config spi rx dma */
	spi_rx_dma->loop_mode = 0;
	spi_rx_dma->wait_cyc  = 0x8;
	/* spi_rx_dma->data_block_size = 1 * DMAC_CFG_SRC_DATA_WIDTH_8BIT/8;*/
	spi_rx_dma->data_block_size = 1 * 32 / 8;

	spi_rx_dma->channal_cfg.src_drq_type	 = DMAC_CFG_TYPE_SPI0;	/* SPI0 */
	spi_rx_dma->channal_cfg.src_addr_mode	 = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;
	spi_rx_dma->channal_cfg.src_burst_length = DMAC_CFG_SRC_8_BURST;
	spi_rx_dma->channal_cfg.src_data_width	 = DMAC_CFG_SRC_DATA_WIDTH_32BIT;

	spi_rx_dma->channal_cfg.dst_drq_type	 = DMAC_CFG_TYPE_DRAM;	/* DRAM */
	spi_rx_dma->channal_cfg.dst_addr_mode	 = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE;
	spi_rx_dma->channal_cfg.dst_burst_length = DMAC_CFG_DEST_8_BURST;
	spi_rx_dma->channal_cfg.dst_data_width	 = DMAC_CFG_DEST_DATA_WIDTH_32BIT;

	return 0;
}

static int sunxi_spi_dma_readl(void *base_addr, unsigned char *buf, unsigned int len)
{
	unsigned char *rx_buf = buf;
	unsigned int rcnt = len;
	ulong ctime = 0;
	int ret = 0;

/*#ifdef SUNXI_DMA_SECURITY
	if((get_boot_work_mode() != WORK_MODE_BOOT)
		&& (sunxi_get_securemode() !=  SUNXI_NORMAL_MODE))
		return sunxi_spi_cpu_readl(sspi, buf, len);
#endif*/
	memset((void *)rx_buf, 0, len);
	writel((readl(base_addr + SPI_FIFO_CTL_REG) | SPI_FIFO_CTL_RX_DRQEN), base_addr + SPI_FIFO_CTL_REG);
	spi_dma_recv_start(base_addr, 0, rx_buf, rcnt);

	/* wait DMA finish */
	ctime = timer_get_us();
	while (1) {
		if (spi_wait_dma_recv_over(0) == 0) {
			ret = 0;
			invalidate_dcache_range((unsigned long)rx_buf, (unsigned long)rx_buf + rcnt);
			break;
		}
		if (timer_get_us() - ctime > 0x4C4B40) {
			printf("rx wait_dma_recv_over fail\n");
			ret = -1;
			break;
		}
	}
	return ret;
}

static int sunxi_spi_dma_disable(void *base_addr)
{
	u32 fcr;

	sunxi_dma_reg_func(NULL);
	/* disable dma req */
	fcr = readl(base_addr + SPI_FIFO_CTL_REG);
	fcr &= ~(SPI_FIFO_CTL_TX_DRQEN | SPI_FIFO_CTL_RX_DRQEN);
	writel(fcr, base_addr + SPI_FIFO_CTL_REG);

	return 0;
}

int spi_dma_init(void)
{
	if (spi_dma_cfg(CONFIG_SF_DEFAULT_BUS)) {
		printf("spi dma cfg error!\n");
		return -1;
	}
	sunxi_dma_install_int(spi_rx_dma_hd, NULL);

	sunxi_dma_enable_int(spi_rx_dma_hd);

	sunxi_dma_setting(spi_rx_dma_hd, (void *)spi_rx_dma);

	return 0;
}
#endif

int set_spi_clk(void)
{
	/* clock */
	if (spinor_info->frequency) {
		spi_clk = spinor_info->frequency;
		if (sunxi_spi_clk_init(0, spi_clk))
			return -1;
		/* spi hardware init*/
		if (spi_claim_bus((void *)SUNXI_SPI0_BASE))
			return -1;
		printf("set spi freq:%d\n", spinor_info->frequency);

		spi_config_tc(1, 0, (void *)SUNXI_SPI0_BASE);
	}
	return 0;
}

int spi_init(void)
{
	SPI_ENTER();
#ifdef CFG_SPI_USE_DMA
	if (spi_dma_init())
		return -1;
#endif
	/* gpio */
	sunxi_spi_gpio_init();
	/* clock */
	if (sunxi_spi_clk_init(0, SUNXI_SPI_DEFAULT_CLK))
		return -1;
	/* spi hardware init*/
	if(spi_claim_bus((void*)SUNXI_SPI0_BASE))
		return -1;
	return 0;
}

void spi_exit(void)
{
	spi_release_bus((void*)SUNXI_SPI0_BASE);
}

int spi_xfer( unsigned int tx_len, const void *dout, unsigned int rx_len,  void *din)
{
	int timeout = 0xfffff;
	void __iomem *base_addr = (void*)SUNXI_SPI0_BASE;

	unsigned int tcnt = 0, rcnt = 0;
	u8 cmd = *(u8*)dout;

	SPI_ENTER();

	/* No data */
	if (!din && !dout)
		return 0;

	rcnt = rx_len;
	tcnt = tx_len;

	spi_disable_irq(0xffffffff, base_addr);
	spi_clr_irq_pending(0xffffffff, base_addr);
	/*spi_set_bc_tc_stc(tcnt, rcnt, stc, 0, base_addr);*/
	sunxi_spi_mode_check(base_addr, tcnt, rcnt, cmd);
	//spi_config_tc(1, 0, base_addr);
	spi_ss_level(base_addr, 1);
	spi_start_xfer(base_addr);

	if (tcnt) {
		if (sunxi_spi_cpu_writel(base_addr, dout, tcnt))
			return -1;
	}

	/* recv data */
	if (rcnt) {
#ifdef CFG_SPI_USE_DMA
		if (rcnt <= 64) {
			if (sunxi_spi_cpu_readl(base_addr, din, rcnt))
				return -1;
		} else {
			if (sunxi_spi_dma_readl(base_addr, din, rcnt))
				return -1;
		}
#else
		if (sunxi_spi_cpu_readl(base_addr, din, rcnt))
			return -1;
#endif
	}

	/* check int status error */
	if (spi_qry_irq_pending(base_addr) & SPI_INT_STA_ERR) {
		printf("int stauts error");
		return -1;
	}

	/* check tx/rx finish */
	timeout = 0xfffff;
	/* wait transfer complete */
	while (!(spi_qry_irq_pending(base_addr)&SPI_INT_STA_TC)) {
		timeout--;
		if (!timeout) {
			printf("SPI_ISR time_out \n");
			return -1;
		}
	}

#ifdef CFG_SPI_USE_DMA
	sunxi_spi_dma_disable(base_addr);
#endif

	/* check SPI_EXCHANGE when SPI_MBC is 0 */
	if (readl(base_addr + SPI_BURST_CNT_REG) == 0) {
		if (readl(base_addr + SPI_TC_REG) & SPI_TC_XCH) {
			printf("XCH Control Error!!\n");
			return -1;
		}
	} else {
		printf("SPI_MBC Error!\n");
		return -1;
	}
	spi_clr_irq_pending(0xffffffff, base_addr);
	return 0;
}
