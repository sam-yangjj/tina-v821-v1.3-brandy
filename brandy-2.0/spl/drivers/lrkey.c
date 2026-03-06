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
 * (C) Copyright 2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * zhouhuacai <zhouhuacai@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/physical_key.h>
#include <arch/clock.h>
#include <arch/axp.h>

__attribute__((section(".data")))
static uint32_t keyen_flag = 1;

__weak int sunxi_clock_init_key(void)
{
	return 0;
}

int sunxi_key_init(void)
{
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;
	uint reg_val = 0;

	sunxi_clock_init_key();

	reg_val = sunxi_key_base->ctrl;
	reg_val &= ~((7 << 1) | (0xffU << 24));
	reg_val |=  LRADC_HOLD_EN;
	reg_val |=  LRADC_EN;
	sunxi_key_base->ctrl = reg_val;

	/* disable all key irq */
	sunxi_key_base->intc = 0;
	sunxi_key_base->ints = 0x1f1f;

	return 0;
}

int sunxi_key_read(void)
{
	u32 ints;
	int key = -1;
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_LRADC_BASE;

	if (!keyen_flag) {
		return -1;
	}

	ints = sunxi_key_base->ints;
	/* clear the pending data */
	sunxi_key_base->ints |= (ints & 0x1f);

	/* if there is already data pending,
	 read it */
	mdelay(10);
	if (ints & ADC0_KEYDOWN_PENDING) {
		if (ints & ADC0_DATA_PENDING) {
			key = sunxi_key_base->data0 & 0x3f;
		}
	} else if (ints & ADC0_DATA_PENDING) {
		key = sunxi_key_base->data0 & 0x3f;
	}

	if (key > 0) {
		printf("key pressed value=0x%x\n", key);
	}

	return key;
}

int check_update_key(u16 *key_input)
{
	int power_key_cnt = 0;
	u32 time_tick = get_sys_ticks();
	*key_input = sunxi_key_read();
	while (sunxi_key_read() > 0) {
		if (probe_power_key() > 0)
			power_key_cnt++;
		if (power_key_cnt >= 3)
			return -1;
		if ((get_sys_ticks() - time_tick) > 3000)
			break;
	}
	if ((!power_key_cnt) && (get_power_source() != AXP_BOOT_SOURCE_BUTTON))
		*key_input = 0;
	return 0;
}

