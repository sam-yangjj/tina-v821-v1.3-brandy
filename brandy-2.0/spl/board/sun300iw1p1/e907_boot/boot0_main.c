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

// CPUX_CFG
#define CPUX_WFI_MODE	   0x49100004
#define CPUX_START_ADD_REG 0x49100204

// CCU_APP
#define CCU_APP_CPUX_MT_CLK_REG 0x42001010
#define CCU_APP_CLK_REG		0x4200107C
#define CPUX_CFG_BUSCLKMASK	(0x3 << 8)
#define CPUX_CFG_BUSCLKDIV2	(0x1 << 8) // div = 2
#define CPUX_CFG_MSGBOX_CLK	(0x3 << 6) // cfg_clken and msgbox_clken
#define CCU_APP_BUS_RESET_REG	0x42001094
#define CPUX_CFG_MSGBOX_RSTN	(0x3 << 27) // cfg_rstn and msgbox_rstn
#define CPUX_CPU_RSTN		(0x1 << 26) // cpu_rstn
#define RV_WDG_RESET_REG	0x42001098
#define CPUX_WDG_RSTN		(0x1 << 2)

// CCU_AON
#define E907_CLK_REG 0x4A010584
#define CPUX_CLK_REG 0x4A010588
#define CLK_EN	     (0x1 << 31)

// POWERCTRL_AON
#define WAKUP_CTRL_REG 0x4A011064
#define CPUX_WUK_EN    (0x1 << 8)
#define LP_STATUS_REG  0x4A011068
#define CPUX_STATUS    (0x1 << 12)

#define BOOT0_2ND_CPU_BOOT_ADDR_OFFSET "0xa0"

#define readl(a)     (*(unsigned int *)(a))
#define writel(v, a) (*(unsigned int *)(a) = (unsigned int)(v))

const unsigned int CPUX_BOOT_ADDR[2] = { 0x12345678, 0x00000000 };

#define E907_BOOT_ADDR "0x82CC8000"

static __inline void irq_disable(void)
{
	unsigned int flags = 0;
	asm volatile("li %0, 0x8"
		     "\n"
		     "csrc mstatus, %0"
		     :
		     : "r"(flags));
}

static __inline void idcache_disable(void)
{
	unsigned int flags = 0;
	asm volatile("li %0, 0x103f"
		     "\n"
		     "csrc mhcr, %0"
		     :
		     : "r"(flags));
}

static __inline unsigned int get_cpux_addr(void)
{
	unsigned int addr = 0;
	asm volatile("auipc %0, 0x0"
		     "\n"
		     "addi %0, %0," BOOT0_2ND_CPU_BOOT_ADDR_OFFSET
		     : "=r"(addr));
	return addr;
}

#define LPMD_MODE_MASK	    "0xC"
#define LPMD_MODE_LILGHTSLP "0x4"

static __inline void e907_set_lightslp(void)
{
	unsigned int flags_0 = 0;
	unsigned int flags_1 = 0;
	asm volatile("li %0," LPMD_MODE_MASK "\n"
		     "csrrc %1, mexstatus, %0\n"
		     "li %0," LPMD_MODE_LILGHTSLP "\n"
		     "csrrs %1, mexstatus, %0\n"
		     :
		     : "r"(flags_0), "r"(flags_1));
}

int main(void)
{
	register unsigned int addr = get_cpux_addr();

	/*disable MIE IRQ*/
	irq_disable();

	/* turn on CPUX power */
	writel(readl(WAKUP_CTRL_REG) | CPUX_WUK_EN, WAKUP_CTRL_REG);

	/* enable CPUX clk, run HOSC */
	writel(CLK_EN, CPUX_CLK_REG);

	/* enable CPUX_MT_CLK, run HOSC */
	writel(CLK_EN, CCU_APP_CPUX_MT_CLK_REG);

	/* enable CPUX msgbus/rv_cfg clock and set AXI div */
	writel((readl(CCU_APP_CLK_REG) & (~(CPUX_CFG_BUSCLKMASK))) |
		       CPUX_CFG_BUSCLKDIV2 | CPUX_CFG_MSGBOX_CLK,
	       CCU_APP_CLK_REG);

	/*disable ICACHE*/
	idcache_disable();

	/* set E907 clk to HOSC for delay
	 * each instruct run about 24 cycles when icache disable
	 * 40M_HOSC as default */
	/* delay 1us for CPUX clk and AXI stable */
	writel(0, E907_CLK_REG);

	/* reseale CPUX msgbus/rv_cfg reset */
	writel(readl(CCU_APP_BUS_RESET_REG) | CPUX_CFG_MSGBOX_RSTN,
	       CCU_APP_BUS_RESET_REG);

	/* set CPUX start addr */
	writel(addr, CPUX_START_ADD_REG);

	/* clean WFI mode */
	writel(0, CPUX_WFI_MODE);

	/* keep CPUX_START_ADD_REG was set */
	while (readl(CPUX_START_ADD_REG) == 0x0) {
		asm volatile("nop");
	}

	/* reseale CPUX cpu reset */
	writel(readl(CCU_APP_BUS_RESET_REG) | CPUX_CPU_RSTN,
	       CCU_APP_BUS_RESET_REG);

	/* wfi in light sleep */
	e907_set_lightslp();
	asm volatile("WFI");

	/* should not run here */
	while (1)
		;
}
