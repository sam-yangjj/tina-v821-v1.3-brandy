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
 * (C) Copyright 2017-2020
 *Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#include <common.h>
#include <private_boot0.h>
#include <asm/io.h>
#include <arch/gpio.h>
#include <arch/clock.h>


int SPINAND_Print(const char *str, ...);

#define get_wvalue(addr)	(*((volatile unsigned long  *)(addr)))
#define put_wvalue(addr, v)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

__u32 SPINAND_GetIOBaseAddr(void)
{
	return SPIC0_BASE_ADDR;
}

__u32 Getpll6Clk(void)
{
	return SPI_CLK_PLL_PERI0;
}

int SPINAND_ClkRequest(__u32 nand_index)
{
#if defined(CONFIG_ARCH_SUN300IW1)
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	u8 reset_shift = SPI_GATING_RESET_SHIFT;
	u8 gating_shift = SPI_GATING_RESET_SHIFT;
	/* spi reset */
	writel(readl(&ccm->ahb_reset0_cfg) & ~(1 << reset_shift), &ccm->ahb_reset0_cfg);
	writel(readl(&ccm->ahb_reset0_cfg) | (1 << reset_shift), &ccm->ahb_reset0_cfg);

	/* spi gating */
	writel(readl(&ccm->ahb_gate0) | (1 << gating_shift), &ccm->ahb_gate0);
#else
	__u32 cfg;
	/*reset*/
	cfg = readl(CCMU_BASE_ADDR + SPI_BGR_REG);
	cfg &= (~(0x1<<16));
	writel(cfg, CCMU_BASE_ADDR + SPI_BGR_REG);

	cfg = readl(CCMU_BASE_ADDR + SPI_BGR_REG);
	cfg |= (0x1<<16);
	writel(cfg, CCMU_BASE_ADDR + SPI_BGR_REG);

	/*open ahb*/
	cfg = readl(CCMU_BASE_ADDR + SPI_BGR_REG);
	cfg |= (0x1<<0);
	writel(cfg, CCMU_BASE_ADDR + SPI_BGR_REG);
#endif
	return 0;
}

void SPINAND_ClkRelease(__u32 nand_index)
{
    return ;
}

int SPINAND_SetClk(__u32 nand_index, __u32 nand_clock)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	unsigned long sclk0_reg_adr = (unsigned long)&ccm->spi0_clk_cfg;
	u32 reg_val;
	u32 sclk0_src_sel, sclk0, sclk0_src;
	u32 sclk0_pre_ratio_n, sclk0_src_t, sclk0_ratio_m;

	/*close dclk and cclk*/
	if (nand_clock == 0) {
		reg_val = readl(sclk0_reg_adr);
		reg_val &= (~(0x1U << 31));
		writel(reg_val, sclk0_reg_adr);
		return 0;
	}

#ifdef FPGA_PLATFORM
	sclk0_src_sel = 0;
	sclk0 = 24;
	sclk0_src = 24;
#elif defined(CONFIG_ARCH_SUN55IW6)
	sclk0_src_sel = 2;
	sclk0 = nand_clock;
	sclk0_src = Getpll6Clk()/1000000;
#else
	sclk0_src_sel = 1;
	sclk0 = nand_clock;
	sclk0_src = Getpll6Clk()/1000000;
#endif

	/* sclk0: 2*dclk*/
	/*sclk0_pre_ratio_n*/
	sclk0_pre_ratio_n = 3;
	if (sclk0_src > 4 * 16 * sclk0)
		sclk0_pre_ratio_n = 3;
	else if (sclk0_src > 2 * 16 * sclk0)
		sclk0_pre_ratio_n = 2;
	else if (sclk0_src > 1 * 16 * sclk0)
		sclk0_pre_ratio_n = 1;
	else
		sclk0_pre_ratio_n = 0;

	sclk0_src_t = sclk0_src >> sclk0_pre_ratio_n;

	/*sclk0_ratio_m*/
	sclk0_ratio_m = (sclk0_src_t / (sclk0)) - 1;
	if (sclk0_src_t % (sclk0))
		sclk0_ratio_m += 1;

	/*close clock*/
	reg_val = readl(sclk0_reg_adr);
	reg_val &= (~(0x1U << 31));
	writel(reg_val, sclk0_reg_adr);

	/*configure*/
	/*sclk0 <--> 2*dclk*/
	reg_val = readl(sclk0_reg_adr);
	/*clock source select*/
	reg_val &= (~(0x7 << 24));
	reg_val |= (sclk0_src_sel & 0x7) << 24;
	/*clock pre-divide ratio(N)*/
	reg_val &= (~(0x3 << 8));
	reg_val |= (sclk0_pre_ratio_n & 0x3) << 8;
	/*clock divide ratio(M)*/
	reg_val &= ~(0xf << 0);
	reg_val |= (sclk0_ratio_m & 0xf) << 0;
	writel(reg_val, sclk0_reg_adr);

	/* open clock*/
	reg_val = readl(sclk0_reg_adr);
	reg_val |= 0x1U << 31;
	writel(reg_val, sclk0_reg_adr);

	return 0;

}

int SPINAND_GetClk(__u32 nand_index)
{
	__u32 pll6_clk;
	__u32 cfg;
	__u32 nand_max_clock;
	__u32 m, n;

	/*set nand clock*/
	pll6_clk = Getpll6Clk();

	/*set nand clock gate on*/
	cfg = readl(CCMU_BASE_ADDR + 0x0940);
	m = ((cfg)&0xf) + 1;
	n = ((cfg >> 8) & 0x3);
	nand_max_clock = pll6_clk / ((1 << n) * m);
	/*printf("(NAND_CLK_BASE_ADDR + 0x0940): 0x%x\n", *(volatile __u32
	 * *)(CCMU_BASE_ADDR + 0x0940));*/

	return nand_max_clock;
}

void SPINAND_PIORequest(__u32 nand_index)
{
#ifdef FPGA_PLATFORM
	sunxi_gpio_set_cfgpin(SUNXI_GPF(24), SUNXI_FPGA_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPF(25), SUNXI_FPGA_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPF(26), SUNXI_FPGA_SPI0); /*spi0_hold*/
	sunxi_gpio_set_cfgpin(SUNXI_GPF(29), SUNXI_FPGA_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPF(30), SUNXI_FPGA_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPF(31), SUNXI_FPGA_SPI0); /*spi0_sclk*/

	sunxi_gpio_set_pull(SUNXI_GPF(25), 1);
	sunxi_gpio_set_pull(SUNXI_GPF(26), 1);
	sunxi_gpio_set_pull(SUNXI_GPF(30), 1);
#elif defined(CONFIG_ARCH_SUN55IW3)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN50I_GPC_SPI0); /*spi0_sclk*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(15), SUN50I_GPC_SPI0); /*spi0_wp*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(16), SUN50I_GPC_SPI0); /*spi0_hold*/

	sunxi_gpio_set_pull(SUNXI_GPC(3), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(15), 1);
	sunxi_gpio_set_pull(SUNXI_GPC(16), 1);
#elif defined(CONFIG_ARCH_SUN55IW5) || defined(CONFIG_ARCH_SUN55IW6) || defined(CONFIG_ARCH_SUN50IW15)
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), SUN50I_GPC_SPI0); /*spi0_mosi*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(3), SUN50I_GPC_SPI0); /*spi0_cs0*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(4), SUN50I_GPC_SPI0); /*spi0_miso*/
	sunxi_gpio_set_cfgpin(SUNXI_GPC(12), SUN50I_GPC_SPI0); /*spi0_sclk*/
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
	#error "spinand pinctrl not available for this architecture"
#endif
}

void SPINAND_PIORelease(__u32 nand_index)
{
	return;
}


/*
********************************************************************
*                                             OSAL_malloc
*˵��������һ����ٵ�malloc������Ŀ��ֻ���ṩ����һ��������������벻ͨ��
*�������ṩ�κεĺ�������
*
********************************************************************
*/
void *SPINAND_Malloc(unsigned int Size)
{
	return (void *)CONFIG_SYS_DRAM_BASE;
}

