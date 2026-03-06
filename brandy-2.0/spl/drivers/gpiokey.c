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
 * (C) Copyright 2024
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * yangjianxiong <yangjianxiong@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/physical_key.h>
#include <arch/clock.h>
#include <arch/axp.h>
#include <arch/gpio.h>

normal_gpio_cfg key_gpio[4] = {
	//port| port_num | mul_sel | pull | drv_level | data | reserved[2]
#if defined(CONFIG_ARCH_SUN60IW2P1)
	{2, 0, 0xe, 0x1, 0x1, 0x1, {} }, /* PB0: vol- */
    {2, 1, 0xe, 0x1, 0x1, 0x1, {} }, /* PB1: vol+ */
#endif
};

__weak int sunxi_clock_init_key(void)
{
	return 0;
}

int sunxi_key_init(void)
{
#if defined(CONFIG_ARCH_SUN60IW2P1)
	u32 reg_val, pin;
	pin = (key_gpio[0].port -1) *32 +  key_gpio[0].port_num;

	/* check whether PB0 is uart */
	reg_val = sunxi_gpio_get_cfgpin(pin);
	if (reg_val == 0x3) {
		return -1;
	}
#endif

	boot_set_gpio(key_gpio, 2, 1);

	return 0;
}

int sunxi_key_read(void)
{
	int key = 0;
    int volume_down = -1;
	int volume_up = -1;

    volume_down = sunxi_gpio_get_one_pin_data(key_gpio[0].port, key_gpio[0].port_num);
	volume_up = sunxi_gpio_get_one_pin_data(key_gpio[1].port, key_gpio[1].port_num);

    if (volume_down == 0 || volume_up == 0) {
		printf("volume key pressed!\n");
		key = 1;
    }

	return key;
}

int check_update_gpiokey(u16 *key_input)
{
	int power_key_cnt = 0;

#if defined(CONFIG_ARCH_SUN60IW2P1)
	/* if pinmux is conflicted, return directly */
    if (sunxi_key_init()) {
		return 0;
	}
#else
	sunxi_key_init();
#endif

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

