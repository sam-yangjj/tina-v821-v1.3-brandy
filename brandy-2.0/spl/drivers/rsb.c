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
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on allwinner u-boot sources rsb code which is:
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * lixiang <lixiang@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <arch/gpio.h>
#include <arch/rsb.h>
#include <arch/rsb.h>
#include <asm/io.h>

static int rsb_set_device_mode(void);

static void rsb_cfg_io(void)
{
#if defined(CONFIG_ARCH_SUN50IW3) || defined(CONFIG_ARCH_SUN8IW16) \
	|| defined(CONFIG_ARCH_SUN8IW19) || defined(CONFIG_ARCH_SUN50IW9) \
	|| defined(CONFIG_ARCH_SUN50IW10) || defined(CONFIG_ARCH_SUN8IW15)
	sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN8I_GPL_R_RSB);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN8I_GPL_R_RSB);
	sunxi_gpio_set_pull(SUNXI_GPL(0), 1);
	sunxi_gpio_set_pull(SUNXI_GPL(1), 1);
	sunxi_gpio_set_drv(SUNXI_GPL(0), 2);
	sunxi_gpio_set_drv(SUNXI_GPL(1), 2);
#elif defined CONFIG_MACH_SUN9I
	sunxi_gpio_set_cfgpin(SUNXI_GPN(0), SUN9I_GPN_R_RSB);
	sunxi_gpio_set_cfgpin(SUNXI_GPN(1), SUN9I_GPN_R_RSB);
	sunxi_gpio_set_pull(SUNXI_GPN(0), 1);
	sunxi_gpio_set_pull(SUNXI_GPN(1), 1);
	sunxi_gpio_set_drv(SUNXI_GPN(0), 2);
	sunxi_gpio_set_drv(SUNXI_GPN(1), 2);
#else
#error unsupported MACH_SUNXI
#endif
}

static void rsb_set_clk(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	u32 div = 0;
	u32 cd_odly = 0;

	/* Source is Hosc24M, set RSB clk to 3Mhz */
	div = 24000000 / 3000000 / 2 - 1;
	cd_odly = div >> 1;
	if (!cd_odly)
		cd_odly = 1;

	writel((cd_odly << 8) | div, &rsb->ccr);
}


static void prcm_rsb_enable(void)
{
	u32 reg_addr;
	/* R_RSB Bus Gating Reset Reg */
	reg_addr = SUNXI_RPRCM_BASE + 0x1bc;

	/* rsb reset */
	setbits_le32(reg_addr, 1<<16);

	/* rsb gating */
	setbits_le32(reg_addr, 1<<0);

}

int rsb_init(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	/* Enable RSB and PIO clk, and de-assert their resets */
	prcm_rsb_enable();

	/* Setup external pins */
	rsb_cfg_io();

	writel(RSB_CTRL_SOFT_RST, &rsb->ctrl);
	rsb_set_clk();

	return rsb_set_device_mode();
}

static int rsb_await_trans(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	unsigned long tmo = timer_get_us() + 1000000;
	u32 stat;
	int ret;

	while (1) {
		stat = readl(&rsb->stat);
		if (stat & RSB_STAT_LBSY_INT) {
			ret = -EBUSY;
			break;
		}
		if (stat & RSB_STAT_TERR_INT) {
			ret = -EIO;
			break;
		}
		if (stat & RSB_STAT_TOVER_INT) {
			ret = 0;
			break;
		}
		if (timer_get_us() > tmo) {
			ret = -ETIME;
			break;
		}
	}
	writel(stat, &rsb->stat); /* Clear status bits */

	return ret;
}

static int rsb_set_device_mode(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	unsigned long tmo = timer_get_us() + 1000000;

	writel(RSB_DMCR_DEVICE_MODE_START | RSB_DMCR_DEVICE_MODE_DATA,
	       &rsb->dmcr);

	while (readl(&rsb->dmcr) & RSB_DMCR_DEVICE_MODE_START) {
		if (timer_get_us() > tmo)
			return -ETIME;
	}

	return rsb_await_trans();
}

static int rsb_do_trans(void)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	setbits_le32(&rsb->ctrl, RSB_CTRL_START_TRANS);
	return rsb_await_trans();
}

int rsb_set_device_address(u16 device_addr, u16 runtime_addr)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_addr) |
	       RSB_DEVADDR_DEVICE_ADDR(device_addr), &rsb->devaddr);
	writel(RSB_CMD_SET_RTSADDR, &rsb->cmd);

	return rsb_do_trans();
}

int rsb_write(const u16 runtime_device_addr, const u8 reg_addr, u8 data)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_device_addr), &rsb->devaddr);
	writel(reg_addr, &rsb->addr);
	writel(data, &rsb->data);
	writel(RSB_CMD_BYTE_WRITE, &rsb->cmd);

	return rsb_do_trans();
}

int rsb_read(const u16 runtime_device_addr, const u8 reg_addr, u8 *data)
{
	struct sunxi_rsb_reg * const rsb =
		(struct sunxi_rsb_reg *)SUNXI_RSB_BASE;
	int ret;

	writel(RSB_DEVADDR_RUNTIME_ADDR(runtime_device_addr), &rsb->devaddr);
	writel(reg_addr, &rsb->addr);
	writel(RSB_CMD_BYTE_READ, &rsb->cmd);

	ret = rsb_do_trans();
	if (ret)
		return ret;

	*data = readl(&rsb->data) & 0xff;

	return 0;
}
