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
 * (C) Copyright 2022-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * lujianliang <lujianliang@allwinnertech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <arch/clock.h>
#include <arch/gpio.h>
#include <arch/spinor.h>
#include "spif-sunxi.h"
#include <private_boot0.h>
#include <cache_align.h>
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
#include <sunxi_hwspinlock.h>
#endif

#define ROUND_UP(a, b) (((a) + (b)-1) & ~((b)-1))
#define SUNXI_SPIF_DEFAULT_CLK  (30000000)

/* For debug */
#define SPIF_DEBUG 0

#if SPIF_DEBUG
#define SPIF_EXIT()		printf("%s()%d - %s\n", __func__, __LINE__, "Exit")
#define SPIF_ENTER()		printf("%s()%d - %s\n", __func__, __LINE__, "Enter ...")
#define SPIF_DBG(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SPIF_INF(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SPIF_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)

#else
#define SPIF_EXIT()
#define SPIF_ENTER()
#define SPIF_DBG(fmt, arg...)
#define SPIF_INF(fmt, arg...)
#define SPIF_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#endif
#define SUNXI_SPIF_OK   0
#define SUNXI_SPIF_FAIL -1

extern boot_spinor_info_t *spinor_info;
__attribute__((section(".data"))) __aligned(64)
struct sunxi_spif_slave g_sspif;

struct sunxi_spif_slave *get_sspif(void)
{
	return &g_sspif;
}

#if SPIF_DEBUG
int spif_debug_flag;
int snprintf(char *buf, size_t size, const char *fmt, ...);
static void spif_print_info(void __iomem *base_addr)
{
	char buf[1024] = {0};
	snprintf(buf, sizeof(buf)-1,
			"base_addr = 0x%x, the SPIF control register:\n"
			"[VER] 0x%02x = 0x%08x, [GC]  0x%02x = 0x%08x, [GCA] 0x%02x = 0x%08x\n"
			"[TCR] 0x%02x = 0x%08x, [TDS] 0x%02x = 0x%08x, [INT] 0x%02x = 0x%08x\n"
			"[STA] 0x%02x = 0x%08x, [CSD] 0x%02x = 0x%08x, [PHC] 0x%02x = 0x%08x\n"
			"[TCF] 0x%02x = 0x%08x, [TCS] 0x%02x = 0x%08x, [TNM] 0x%02x = 0x%08x\n"
			"[PSR] 0x%02x = 0x%08x, [PSA] 0x%02x = 0x%08x, [PEA] 0x%02x = 0x%08x\n"
			"[PMA] 0x%02x = 0x%08x, [DMA] 0x%02x = 0x%08x, [DSC] 0x%02x = 0x%08x\n"
			"[DFT] 0x%02x = 0x%08x, [CFT] 0x%02x = 0x%08x, [CFS] 0x%02x = 0x%08x\n"
			"[BAT] 0x%02x = 0x%08x, [BAC] 0x%02x = 0x%08x, [TB]  0x%02x = 0x%08x\n"
			"[RB]  0x%02x = 0x%08x\n",
			base_addr,
			SPIF_VER_REG, readl(base_addr + SPIF_VER_REG),
			SPIF_GC_REG, readl(base_addr + SPIF_GC_REG),
			SPIF_GCA_REG, readl(base_addr + SPIF_GCA_REG),

			SPIF_TC_REG, readl(base_addr + SPIF_TC_REG),
			SPIF_TDS_REG, readl(base_addr + SPIF_TDS_REG),
			SPIF_INT_EN_REG, readl(base_addr + SPIF_INT_EN_REG),

			SPIF_INT_STA_REG, readl(base_addr + SPIF_INT_STA_REG),
			SPIF_CSD_REG, readl(base_addr + SPIF_CSD_REG),
			SPIF_PHC_REG, readl(base_addr + SPIF_PHC_REG),

			SPIF_TCF_REG, readl(base_addr + SPIF_TCF_REG),
			SPIF_TCS_REG, readl(base_addr + SPIF_TCS_REG),
			SPIF_TNM_REG, readl(base_addr + SPIF_TNM_REG),

			SPIF_PS_REG, readl(base_addr + SPIF_PS_REG),
			SPIF_PSA_REG, readl(base_addr + SPIF_PSA_REG),
			SPIF_PEA_REG, readl(base_addr + SPIF_PEA_REG),

			SPIF_PMA_REG, readl(base_addr + SPIF_PMA_REG),
			SPIF_DMA_CTL_REG, readl(base_addr + SPIF_DMA_CTL_REG),
			SPIF_DSC_REG, readl(base_addr + SPIF_DSC_REG),

			SPIF_DFT_REG, readl(base_addr + SPIF_DFT_REG),
			SPIF_CFT_REG, readl(base_addr + SPIF_CFT_REG),
			SPIF_CFS_REG, readl(base_addr + SPIF_CFS_REG),

			SPIF_BAT_REG, readl(base_addr + SPIF_BAT_REG),
			SPIF_BAC_REG, readl(base_addr + SPIF_BAC_REG),
			SPIF_TB_REG, readl(base_addr + SPIF_TB_REG),

			SPIF_RB_REG, readl(base_addr + SPIF_RB_REG));
			printf("%s\n\n", buf);
}

void spif_print_descriptor(struct spif_descriptor_op *spif_op)
{
	char buf[512] = {0};
	snprintf(buf, sizeof(buf)-1,
			"hburst_rw_flag        : 0x%x\n"
			"block_data_len        : 0x%x\n"
			"data_addr             : 0x%x\n"
			"next_des_addr         : 0x%x\n"
			"trans_phase	       : 0x%x\n"
			"flash_addr	       : 0x%x\n"
			"cmd_mode_buswidth     : 0x%x\n"
			"addr_dummy_data_count : 0x%x\n",
			spif_op->hburst_rw_flag,
			spif_op->block_data_len,
			spif_op->data_addr,
			spif_op->next_des_addr,
			spif_op->trans_phase,
			spif_op->flash_addr,
			spif_op->cmd_mode_buswidth,
			spif_op->addr_dummy_data_count);
			printf("%s", buf);
	printf("spif_op addr [%x]...\n\n", (u32)spif_op);
}

#endif

static s32 sunxi_spif_gpio_request(void)
{
/*#ifndef CFG_SUNXI_SBOOT
	int i = 0;
	int pin_num = 6;

	if (BT0_head.prvt_head.storage_gpio[i].port) {
		boot_set_gpio(
		(normal_gpio_cfg *)BT0_head.prvt_head.storage_gpio, pin_num, 1);
	} else {
#endif*/
#if defined(CONFIG_ARCH_SUN8IW21)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), SUN8I_GPC_SPIF); /*spif_sclk */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(1), SUN8I_GPC_SPIF); /*spif_cs0*/

	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN8I_GPC_SPIF); /*spif_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN8I_GPC_SPIF); /*spif_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN8I_GPC_SPIF); /*spif_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(5), SUN8I_GPC_SPIF); /*spif_hold*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(6), SUN8I_GPC_SPIF); /*spif_io4*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN8I_GPC_SPIF); /*spif_io5*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(8), SUN8I_GPC_SPIF); /*spif_io6*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(9), SUN8I_GPC_SPIF); /*spif_io7*/

	sunxi_gpio_set_pull(SUNXI_GPC(1), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(4), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(5), 1);
#elif defined(CONFIG_ARCH_SUN55IW3)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN55I_GPC_SPIF); /*spif_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN55I_GPC_SPIF); /*spif_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN55I_GPC_SPIF); /*spif_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN55I_GPC_SPIF); /*spif_dqs*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(8), SUN55I_GPC_SPIF); /*spif_io7*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(9), SUN55I_GPC_SPIF); /*spif_io6*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(10), SUN55I_GPC_SPIF); /*spif_io5*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(11), SUN55I_GPC_SPIF); /*spif_io4*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN55I_GPC_SPIF); /*spif_clk*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN55I_GPC_SPIF); /*spif_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN55I_GPC_SPIF); /*spif_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN55IW5) || defined(CONFIG_ARCH_SUN50IW15)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN55I_GPC_SPIF); /*spif_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN55I_GPC_SPIF); /*spif_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN55I_GPC_SPIF); /*spif_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN55I_GPC_SPIF); /*spif_dqs*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(8), SUN55I_GPC_SPIF); /*spif_io7*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(9), SUN55I_GPC_SPIF); /*spif_io6*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(10), SUN55I_GPC_SPIF); /*spif_io5*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(11), SUN55I_GPC_SPIF); /*spif_io4*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN55I_GPC_SPIF); /*spif_clk*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN55I_GPC_SPIF); /*spif_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN55I_GPC_SPIF); /*spif_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN300IW1)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(6), SUN8I_GPC_SPIF); /*spif_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(7), SUN8I_GPC_SPIF); /*spif_hold*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(8), SUN8I_GPC_SPIF); /*spif_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(9), SUN8I_GPC_SPIF); /*spif_clk*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(10), SUN8I_GPC_SPIF); /*spif_cs*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(11), SUN8I_GPC_SPIF); /*spif_miso*/

	sunxi_gpio_set_pull(SUNXI_GPC(6), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(7), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(10), 1);
#else
	#error "spi pinctrl not available for this architecture"
#endif
/*#ifndef CFG_SUNXI_SBOOT
	}
#endif*/
	return 0;
}

u32 sunxi_spif_get_version(void)
{
	return readl(SUNXI_SPIF_BASE + SPIF_VER_REG);
}

static void spif_big_little_endian(bool endian, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (endian == LSB_FIRST)
		reg_val |= (SPIF_GC_RX_CFG_FBS | SPIF_GC_TX_CFG_FBS);
	else
		reg_val &= ~(SPIF_GC_RX_CFG_FBS | SPIF_GC_TX_CFG_FBS);
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static void spif_clean_mode_en(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	reg_val &= ~(SPIF_GC_NMODE_EN | SPIF_GC_PMODE_EN);
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static void spif_wp_en(bool enable, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (enable)
		reg_val |= SPIF_GC_WP_EN;
	else
		reg_val &= ~SPIF_GC_WP_EN;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static void spif_hold_en(bool enable, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (enable)
		reg_val |= SPIF_GC_HOLD_EN;
	else
		reg_val &= ~SPIF_GC_HOLD_EN;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static void spif_set_cs_pol(bool pol, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (pol)
		reg_val |= SPIF_GC_CS_POL;
	else
		reg_val &= ~SPIF_GC_CS_POL;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

/* spif config chip select */
static s32 spif_set_cs(u32 chipselect, void __iomem *base_addr)
{
	int ret;
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (chipselect < 4) {
		reg_val &= ~SPIF_GC_SS_MASK;/* SS-chip select, clear two bits */
		reg_val |= chipselect << SPIF_GC_SS_BIT_POS;/* set chip select */
		reg_val |= SPIF_GC_CS_POL;/* active low polarity */
		writel(reg_val, base_addr + SPIF_GC_REG);
		ret = SUNXI_SPIF_OK;
	} else {
		SPIF_ERR("Chip Select set fail! cs = %d\n", chipselect);
		ret = SUNXI_SPIF_FAIL;
	}

	return ret;
}

static void spif_set_mode(u32 spi_mode, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	reg_val &= ~SPIF_MASK;
	reg_val |= spi_mode;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

void spif_samp_dl_sw_rx_status(void __iomem  *base_addr, unsigned int status)
{
	unsigned int rval = readl(base_addr + SPIF_TC_REG);

	if (status)
		rval |= SPIF_ANALOG_DL_SW_RX_EN;
	else
		rval &= ~SPIF_ANALOG_DL_SW_RX_EN;

	writel(rval, base_addr +SPIF_TC_REG);
}

void spif_samp_mode(void __iomem  *base_addr, unsigned int status)
{
	unsigned int rval = readl(base_addr + SPIF_TC_REG);

	if (status)
		rval |= SPIF_DIGITAL_ANALOG_EN;
	else
		rval &= ~SPIF_DIGITAL_ANALOG_EN;

	writel(rval, base_addr + SPIF_TC_REG);
}

void spif_set_sample_mode(void __iomem *base_addr, unsigned int mode)
{
	unsigned int rval = readl(base_addr + SPIF_TC_REG);

	rval &= (~SPIF_DIGITAL_DELAY_MASK);
	rval |= mode << SPIF_DIGITAL_DELAY;
	writel(rval, base_addr + SPIF_TC_REG);
}

void spif_set_sample_delay(void __iomem  *base_addr, unsigned int sample_delay)
{
	unsigned int rval = readl(base_addr + SPIF_TC_REG);

	rval &= (~SPIF_ANALOG_DELAY_MASK);
	rval |= sample_delay << SPIF_ANALOG_DELAY;
	writel(rval, base_addr + SPIF_TC_REG);
	mdelay(1);
}

static void spif_config_tc(void __iomem  *base_addr)
{
	if (spinor_info->sample_mode != SAMP_MODE_DL_DEFAULT) {
		spif_samp_mode(base_addr, 1);
		spif_samp_dl_sw_rx_status(base_addr, 1);
		spif_set_sample_mode(base_addr, spinor_info->sample_mode);
		spif_set_sample_delay(base_addr, spinor_info->sample_delay);
	}
}
/*
static void spif_set_dqs(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_DFT_REG);

	reg_val |= SPIF_DFT_DQS;
	writel(reg_val, base_addr + SPIF_DFT_REG);
}

static void spif_set_cdc(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_CFT_REG);

	reg_val = SPIF_CFT_CDC;
	writel(reg_val, base_addr + SPIF_CFT_REG);
}
*/
static void spif_set_csd(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_CSD_REG);

	reg_val |= SPIF_CSD_DEF;
	writel(reg_val, base_addr + SPIF_CSD_REG);
}

/* soft reset spif controller */
static void spif_soft_reset_fifo(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GCA_REG);

	reg_val |= SPIF_GCA_SRST;
	writel(reg_val, base_addr + SPIF_GCA_REG);
}

static void spif_reset_fifo(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GCA_REG);

	reg_val |= SPIF_FIFO_SRST;
	writel(reg_val, base_addr + SPIF_GCA_REG);
}

static void spif_set_trans_mode(u8 mode, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (mode)
		reg_val |= SPIF_GC_CFG_MODE;
	else
		reg_val &= ~SPIF_GC_CFG_MODE;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

/* set first descriptor start addr */
static void spif_set_des_start_addr(struct spif_descriptor_op *spif_op,
				void __iomem *base_addr)
{
#ifdef CONFIG_ARCH_SUN8IW21
	writel((u32)spif_op, base_addr + SPIF_DSC_REG);
#else
	/* addr word alignment */
	writel((u32)spif_op >> 2, base_addr + SPIF_DSC_REG);
#endif
}

/* set descriptor len */
static void spif_set_des_len(int len, void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_DMA_CTL_REG);

	reg_val |= len;
	writel(reg_val, base_addr + SPIF_DMA_CTL_REG);
}

/* DMA start Signal */
static void spif_dma_start_signal(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_DMA_CTL_REG);

	reg_val |= CFG_DMA_START;
	writel(reg_val, base_addr + SPIF_DMA_CTL_REG);
}

static void spif_trans_type_enable(u32 type_phase, void __iomem *base_addr)
{
	writel(type_phase, base_addr + SPIF_PHC_REG);
}

static void spif_set_flash_addr(u32 flash_addr, void __iomem *base_addr)
{
	writel(flash_addr, base_addr + SPIF_TCF_REG);
}

static void spif_set_buswidth(u32 cmd_mode_buswidth, void __iomem *base_addr)
{
	writel(cmd_mode_buswidth, base_addr + SPIF_TCS_REG);
}

static void spif_set_data_count(u32 addr_dummy_data_count, void __iomem *base_addr)
{
	writel(addr_dummy_data_count, base_addr + SPIF_TNM_REG);
}

static void spif_cpu_start_transfer(void __iomem *base_addr)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	reg_val |= SPIF_GC_NMODE_EN;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static void spif_set_output_clk(void __iomem *base_addr, u32 status)
{
	u32 reg_val = readl(base_addr + SPIF_TC_REG);

	if (status)
		reg_val |= SPIF_CLK_SCKOUT_SRC_SEL;
	else
		reg_val &= ~SPIF_CLK_SCKOUT_SRC_SEL;
	writel(reg_val, base_addr + SPIF_TC_REG);
}

static void spif_set_dtr(void __iomem *base_addr, u32 status)
{
	u32 reg_val = readl(base_addr + SPIF_GC_REG);

	if (status)
		reg_val |= SPIF_GC_DTR_EN;
	else
		reg_val &= ~SPIF_GC_DTR_EN;
	writel(reg_val, base_addr + SPIF_GC_REG);
}

static int sunxi_spif_clk_init(u32 bus, u32 mod_clk)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	unsigned long mclk_base =
		(unsigned long)&ccm->spif_clk_cfg + bus * 0x4;
	u32 source_clk = 0;
	u32 rval = 0;
	u32 m, n, div;
	static u32 first_init;

#if defined(CONFIG_ARCH_SUN300IW1)
	u8 reset_shift = SPIF_GATING_RESET_SHIFT;
	u8 gating_shift = SPIF_GATING_RESET_SHIFT;
	if (first_init) {
		/* close gating */
		writel(readl(&ccm->ahb_gate0) & ~(1 << gating_shift), &ccm->ahb_gate0);
	}
#else
	if (first_init) {
		/* close gating */
		setbits_le32(&ccm->spi_gate_reset, (0 << SPIF_GATING_SHIFT));
	}
#endif


	/* SCLK = src/M/N */
	/* N: 00:1 01:2 10:4 11:8 */
#ifdef FPGA_PLATFORM
	n = 0;
	m = 1;
	rval = CCM_SPIF_CTRL_ENABLE | CCM_SPIF_CTRL_N(n) | CCM_SPIF_CTRL_M(m);;
	source_clk = 24000000;
#else

	source_clk = GET_SPIF_CLK_SOURECS(CCM_SPIF_CTRL_PERI);
	SPIF_INF("source_clk: %d Hz, mod_clk: %d Hz\n", source_clk, mod_clk);

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

	rval = CCM_SPIF_CTRL_ENABLE | CCM_SPIF_CTRL_PERI |
			CCM_SPIF_CTRL_N(n) | CCM_SPIF_CTRL_M(m);

#endif
	writel(rval, (volatile void __iomem *)mclk_base);

#if defined(CONFIG_ARCH_SUN300IW1)
	if (!first_init) {
		/* spi reset */
		writel(readl(&ccm->ahb_reset0_cfg) & ~(1 << reset_shift), &ccm->ahb_reset0_cfg);
		writel(readl(&ccm->ahb_reset0_cfg) | (1 << reset_shift), &ccm->ahb_reset0_cfg);
		first_init = 1;
	}

	/* spi gating */
	writel(readl(&ccm->ahb_gate0) | (1 << gating_shift), &ccm->ahb_gate0);
#else
	if (!first_init) {
		/* spif reset */
		setbits_le32(&ccm->spi_gate_reset, (0 << SPIF_RESET_SHIFT));
		setbits_le32(&ccm->spi_gate_reset, (1 << SPIF_RESET_SHIFT));
		first_init = 1;
	}

	/* spif gating */
	setbits_le32(&ccm->spi_gate_reset, (1 << SPIF_GATING_SHIFT));
#endif

	SPIF_INF("src: %d Hz, spic:%d Hz,  n=%d, m=%d\n",
			source_clk, source_clk/ (1 << n) / m, n, m);

	return 0;
}
/*
static int sunxi_get_spif_clk(int bus)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	unsigned long mclk_base =
		(unsigned long)&ccm->spif_clk_cfg + bus * 0x4;
	u32 reg_val = 0;
	u32 src = 0, clk = 0, sclk_freq = 0;
	u32 n, m;

	reg_val = readl((volatile void __iomem *)mclk_base);
	src = (reg_val >> 24) & 0x7;
	n = (reg_val >> 8) & 0x3;
	m = ((reg_val >> 0) & 0xf) + 1;

	switch(src) {
		case 0:
			clk = 24000000;
			break;
		case 1:
			clk = GET_SPIF_CLK_SOURECS(CCM_SPIF_CTRL_PERI400M);
			break;
		case 2:
			clk = GET_SPIF_CLK_SOURECS(CCM_SPIF_CTRL_PERI300M);
			break;
		default:
			clk = 0;
			break;
	}
	sclk_freq = clk / (1 << n) / m;
	SPIF_INF("sclk_freq= %d Hz,reg_val: %x , n=%d, m=%d\n",
			sclk_freq, reg_val, n, m);
	return sclk_freq;

}
*/
static int sunxi_spif_clk_exit(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

#if defined(CONFIG_ARCH_SUN300IW1)
	u8 reset_shift = SPIF_GATING_RESET_SHIFT;
	u8 gating_shift = SPIF_GATING_RESET_SHIFT;
	/* spi gating */
	writel(readl(&ccm->ahb_gate0) & ~(1 << gating_shift), &ccm->ahb_gate0);

	/* spi reset */
	writel(readl(&ccm->ahb_reset0_cfg) & ~(1 << reset_shift), &ccm->ahb_reset0_cfg);
#else
	/* spif gating */
	clrbits_le32(&ccm->spi_gate_reset, 1 << SPIF_GATING_SHIFT);

	/* spif reset */
	clrbits_le32(&ccm->spi_gate_reset, 1 << SPIF_RESET_SHIFT);
#endif
	return 0;
}

int set_spif_clk(void)
{
	/* clock */
	if (spinor_info->frequency) {
		if (sunxi_spif_clk_init(0, spinor_info->frequency))
			return -1;
		printf("set spif freq:%d\n", spinor_info->frequency);
	}
	return 0;
}

static void spif_dtr_enable(struct spif_descriptor_op *spif_op)
{
	struct sunxi_spif_slave *sspif = get_sspif();
	void __iomem *base_addr = (void *)SUNXI_SPIF_BASE;
	unsigned int bus = 0;
	unsigned int clk = sspif->max_hz;
	unsigned int dtr_double_clk = clk * 2;
	static int double_clk_flag;

	if (!sspif->rx_dtr_en && !sspif->tx_dtr_en)
		return;

	if ((spif_op->cmd_mode_buswidth >> SPIF_ADDR_TRANS_POS) & 0x3) {
		if ((spif_op->trans_phase & SPIF_RX_TRANS_EN) &&
				sspif->rx_dtr_en) {
			spif_set_output_clk(base_addr, 1);
			spif_set_dtr(base_addr, 1);
			if (!double_clk_flag) {
				sunxi_spif_clk_init(bus, dtr_double_clk);
				double_clk_flag = 1;
			}
		} else if (spif_op->trans_phase & SPIF_TX_TRANS_EN &&
				sspif->tx_dtr_en) {
			spif_set_output_clk(base_addr, 1);
			spif_set_dtr(base_addr, 1);
			if (!dtr_double_clk) {
				sunxi_spif_clk_init(bus, dtr_double_clk);
				double_clk_flag = 1;
			}
		}
	} else {
		spif_set_output_clk(base_addr, 0);
		spif_set_dtr(base_addr, 0);
		if (double_clk_flag) {
			sunxi_spif_clk_init(bus, clk);
			double_clk_flag = 0;
		}
	}
}

static void sunxi_spif_set_irq(void __iomem *base_addr, u32 bitmap, int set)
{
	uint32_t reg_val = readl(base_addr + SPIF_INT_EN_REG);

	if (set)
		reg_val |= bitmap;
	else
		reg_val &= ~bitmap;
	writel(reg_val, base_addr + SPIF_INT_EN_REG);
}

int spif_claim_bus(void *base_addr)
{
	SPIF_ENTER();

	/* 1. reset all tie logic & fifo */
	spif_soft_reset_fifo(base_addr);
	spif_clean_mode_en(base_addr);

	/* 2. interface first transmit bit select */
	spif_big_little_endian(MSB_FIRST, base_addr);

	/* 3. disable wp & hold */
	spif_wp_en(0, base_addr);
	spif_hold_en(0, base_addr);

	/* 4. disable DTR */
	spif_set_output_clk(base_addr, 0);
	spif_set_dtr(base_addr, 0);

	/* 5. set the default chip select */
	spif_set_cs(0, base_addr);
	spif_set_cs_pol(1, base_addr);

	/* 6. set spi CPOL and CPHA */
	spif_set_mode(SPIF_MODE0, base_addr);

	/* 7. set reg defauld count */
	//spif_set_dqs(base_addr);
	//spif_set_cdc(base_addr);
	spif_set_csd(base_addr);

	return 0;
}

int spif_init(void)
{
	SPIF_ENTER();
	/* gpio */
	sunxi_spif_gpio_request();

	/* clock */
	if (sunxi_spif_clk_init(0, SUNXI_SPIF_DEFAULT_CLK))
		return -1;

	if (spif_claim_bus((void *)SUNXI_SPIF_BASE))
		return -1;
	return 0;
}

void spif_exit(void)
{
	/* disable module clock */
	sunxi_spif_clk_exit();
}

static void spif_ctr_recover(void)
{
	spif_exit();
	spif_init();
	spif_config_tc((void *)SUNXI_SPIF_BASE);
}

int spif_xfer(struct spif_descriptor_op *spif_op, unsigned int data_len, int async)
{
	int timeout = 0xfffffff;
	void __iomem *base_addr = (void *)SUNXI_SPIF_BASE;
	uint desc_count = ((data_len + SPIF_MAX_TRANS_NUM - 1) / SPIF_MAX_TRANS_NUM) + 1;
	uint desc_size = desc_count * sizeof(struct spif_descriptor_op);
#ifdef CONFIG_ARCH_SUN8IW21
	unsigned int data_addr = (u32)spif_op->data_addr;
#else
	unsigned int data_addr = (u32)spif_op->data_addr << 2;
#endif
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
	hwspin_lock(CFG_SPIF_HWLOCK_IDX);
#endif
	spif_reset_fifo(base_addr);
	spif_dtr_enable(spif_op);
	if ((spif_op->block_data_len & DMA_DATA_LEN) == 0) {
		spif_set_trans_mode(SPIF_GC_CPU_MODE, base_addr);

		spif_trans_type_enable(spif_op->trans_phase, base_addr);

		spif_set_flash_addr(spif_op->flash_addr, base_addr);

		spif_set_buswidth(spif_op->cmd_mode_buswidth, base_addr);

		spif_set_data_count(spif_op->addr_dummy_data_count, base_addr);

		spif_cpu_start_transfer(base_addr);

		while ((readl(base_addr + SPIF_GC_REG) & SPIF_GC_NMODE_EN)) {
			timeout--;
			if (!timeout) {
				spif_ctr_recover();
				printf("SPIF DMA transfer time_out\n");
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
				hwspin_unlock(CFG_SPIF_HWLOCK_IDX);
#endif
				return -1;
			}
		}
#if SPIF_DEBUG
		if (spif_debug_flag)
			spif_print_info(base_addr);
#endif
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
		hwspin_unlock(CFG_SPIF_HWLOCK_IDX);
#endif
	} else {
#if 0 //CFG_SPI_USE_DMA
		spif_set_trans_mode(SPIF_GC_CPU_MODE, base_addr);

		spif_trans_type_enable(spif_op->trans_phase, base_addr);

		spif_set_flash_addr(spif_op->flash_addr, base_addr);

		spif_set_buswidth(spif_op->cmd_mode_buswidth, base_addr);

		spif_set_data_count(spif_op->addr_dummy_data_count, base_addr);
#else
		spif_set_trans_mode(SPIF_GC_DMA_MODE, base_addr);

#endif
		/* flush data addr */
		flush_dcache_range(data_addr, data_addr +
				ROUND_UP(data_len, CONFIG_SYS_CACHELINE_SIZE));
		flush_dcache_range((u32)spif_op, (u32)spif_op +
				ROUND_UP(desc_size, CONFIG_SYS_CACHELINE_SIZE));

		spif_set_des_start_addr(spif_op, base_addr);

		spif_set_des_len(DMA_DESCRIPTOR_LEN, base_addr);

#if SPIF_DEBUG
		if (spif_debug_flag) {
			spif_print_descriptor(spif_op);
			spif_print_info(base_addr);
		}
#endif

		if (async) {
			sunxi_spif_set_irq(base_addr, SPIF_DMA_TRANS_DONE_EN | SPIF_INT_STA_ERR_EN, 1);
		} else {
			sunxi_spif_set_irq(base_addr, SPIF_DMA_TRANS_DONE_EN | SPIF_INT_STA_ERR_EN, 0);
		}
		spif_dma_start_signal(base_addr);

		/*
		 *  The SPIF move data through DMA, and DMA and CPU modes
		 *  differ only between actively configuring registers and
		 *  configuring registers through the DMA descriptor
		 */
#if 0 //CFG_SPI_USE_DMA
		spif_cpu_start_transfer(base_addr);
#endif

		if (!async) {
			/* waiting DMA finish */
			while (!(readl(base_addr + SPIF_INT_STA_REG) &
					SPIF_DMA_TRANS_DONE_EN)) {
				timeout--;
				if (!timeout) {
					spif_ctr_recover();
					printf("SPIF DMA transfer time_out\n");
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
					hwspin_unlock(CFG_SPIF_HWLOCK_IDX);
#endif
					return -1;
				}
			}
			invalidate_dcache_range(data_addr, data_addr +
					ROUND_UP(data_len, CONFIG_SYS_CACHELINE_SIZE));
			writel(SPIF_DMA_TRANS_DONE_EN, base_addr + SPIF_INT_STA_REG);
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
			hwspin_unlock(CFG_SPIF_HWLOCK_IDX);
#endif
		} else {
			invalidate_dcache_range(data_addr, data_addr +
					ROUND_UP(data_len, CONFIG_SYS_CACHELINE_SIZE));
		}
	}

	return 0;
}

void spif_unlock(void)
{
#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT)
	hwspin_unlock(CFG_SPIF_HWLOCK_IDX);
#endif
}

