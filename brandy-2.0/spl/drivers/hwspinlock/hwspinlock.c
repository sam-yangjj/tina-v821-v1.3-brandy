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
 * (C) Copyright 2023
 * wujiayi <wujiayi@allwinnertech.com>
 */
#include <asm/io.h>
#include <common.h>
#include <arch/clock.h>
#include <sunxi_hwspinlock.h>
#include "hwspinlock.h"

/* Possible values of SPINLOCK_LOCK_REG */
#define SPINLOCK_NOTTAKEN               (0)     /* free */
#define SPINLOCK_TAKEN                  (1)     /* locked */

void hwspinlock_init(void)
{
	u32 reg_val;
	/* clock gating */
	reg_val = readl(SUNXI_CCM_BASE + SPIN_LOCK_CLK_REG);
	reg_val |= (1 << BIT_SPIN_LOCK_GATING);
	writel(reg_val, SUNXI_CCM_BASE + SPIN_LOCK_CLK_REG);

	/* reset */
	reg_val = readl(SUNXI_CCM_BASE + SPIN_LOCK_RST_REG);
	reg_val |= (1 << BIT_SPIN_LOCK_RST);
	writel(reg_val, SUNXI_CCM_BASE + SPIN_LOCK_RST_REG);
}

int hwspinlock_check_taken(int num)
{
	return !!(readl(SPINLOCK_STATUS_REG) & (1 << num));
}

int hwspinlock_get(int num)
{
	unsigned long addr = SPINLOCK_LOCK_REG(num);
	int status;

	if (num > SPINLOCK_NUM)
		return HWSPINLOCK_EXCEED_MAX;

	status = readl(addr);

	if (status == SPINLOCK_NOTTAKEN)
		return HWSPINLOCK_OK;

	return HWSPINLOCK_ERR;
}

int hwspinlock_put(int num)
{
	unsigned long addr = SPINLOCK_LOCK_REG(num);

	if (num > SPINLOCK_NUM)
		return HWSPINLOCK_EXCEED_MAX;

	writel(SPINLOCK_NOTTAKEN, addr);

	return HWSPINLOCK_OK;
}

int hwspin_lock(int num)
{
	if (num > SPINLOCK_NUM)
		return HWSPINLOCK_ERR;

	while (1) {
		if (hwspinlock_get(num) == HWSPINLOCK_OK)
			break;
	}

	return HWSPINLOCK_OK;
}

void hwspin_unlock(int num)
{
	hwspinlock_put(num);
}
