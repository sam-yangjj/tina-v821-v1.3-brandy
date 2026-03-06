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
 * Copyright (C) 2019 Allwinner Tech
 * frank <frank@allwinnertech.com>
 */

#include <arch/watchdog.h>

static const int wdt_timeout_map[] = {
		[1]  = 0x1, /* 1s  */
		[2]  = 0x2, /* 2s  */
		[3]  = 0x3, /* 3s  */
		[4]  = 0x4, /* 4s  */
		[5]  = 0x5, /* 5s  */
		[6]  = 0x6, /* 6s  */
		[8]  = 0x7, /* 8s  */
		[10] = 0x8, /* 10s */
		[12] = 0x9, /* 12s */
		[14] = 0xA, /* 14s */
		[16] = 0xB, /* 16s */
};

#if defined(CONFIG_ARCH_SUN8IW21) || defined(CONFIG_ARCH_SUN300IW1P1)
void wdt_stop(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;

	writel(wtmode | KEY_FIELD, &wdt->mode);
}

void wdt_start(unsigned int timeout)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wdt_stop();

	if (wdt_timeout_map[timeout] == 0)
		timeout++;

	wtmode = wdt_timeout_map[timeout] << 4 | WDT_MODE_EN | KEY_FIELD;

	writel(WDT_CFG_RESET, &wdt->cfg);
	writel(wtmode, &wdt->mode);
	writel(WDT_CTRL_RELOAD, &wdt->ctl);
}
#elif !defined(CONFIG_MACH_SUN8IW11)
void wdt_stop(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;

	writel(wtmode, &wdt->mode);
}

void wdt_start(unsigned int timeout)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wdt_stop();

	if (wdt_timeout_map[timeout] == 0)
		timeout++;

	wtmode = wdt_timeout_map[timeout] << 4 | WDT_MODE_EN;

	writel(WDT_CFG_RESET, &wdt->cfg);
	writel(wtmode, &wdt->mode);
}
#else
void wdt_stop(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;

	writel(wtmode, &wdt->mode);
}

void wdt_start(unsigned int timeout)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)SUNXI_WDOG_BASE;
	unsigned int wtmode;

	wdt_stop();

	if (wdt_timeout_map[timeout] == 0)
		timeout++;

	wtmode =
		wdt_timeout_map[timeout] << 3 | WDT_MODE_EN | WDT_MODE_RESET_EN;

	writel(wtmode, &wdt->mode);
}
#endif
