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
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/gpio.h>
#include <arch/rtc.h>
#include <arch/efuse.h>
#include "sboot_flash.h"
#include "sboot_toc.h"
#include <boot_param.h>

sbrom_toc0_config_t *toc0_config = (sbrom_toc0_config_t *)(CONFIG_TOC0_CFG_ADDR);
phys_addr_t mmc_config_addr = (phys_addr_t)(((sbrom_toc0_config_t *)CONFIG_TOC0_CFG_ADDR)->storage_data + 160);

static int sboot_clear_env(void);

static void print_commit_log(void)
{
	raw_printf("......\n\n\n");
	printf("HELLO! SBOOT is starting!\n");
	printf("sboot commit : %s \n",sboot_head.hash);
	sunxi_set_printf_debug_mode(toc0_config->debug_mode, 0);
}

void sboot_main(void)
{
	toc0_private_head_t *toc0 = (toc0_private_head_t *)CONFIG_TOC0_HEAD_BASE;
	uint dram_size;
	u16 pmu_type = 0, key_input = 0; /* TODO: set real value */
	int  ret;

	sunxi_board_init_early();
	sunxi_serial_init(toc0_config->uart_port, toc0_config->uart_ctrl, 2);
	print_commit_log();

	ret = sunxi_board_init();
	if (ret)
		goto _BOOT_ERROR;

	if (rtc_probe_fel_flag()) {
		rtc_clear_fel_flag();
		goto _BOOT_ERROR;
#ifdef CFG_SUNXI_PHY_KEY
#ifdef CFG_LRADC_KEY
	} else if (check_update_key(&key_input)) {
		goto _BOOT_ERROR;
#elif CFG_GPIO_KEY
	} else if (check_update_gpiokey(&key_input)) {
		goto _BOOT_ERROR;
#endif
#endif
	}

	if (toc0_config->enable_jtag) {
		printf("enable_jtag\n");
		boot_set_gpio((normal_gpio_cfg *)toc0_config->jtag_gpio, 5, 1);
	}
#if CFG_SUNXI_JTAG_DISABLE
	else {
		sid_disable_jtag();
	}
#endif

	char uart_input_value = get_uart_input();

	if (uart_input_value == '2') {
		sunxi_set_printf_debug_mode(3, 0);
		printf("detected user input 2\n");
		goto _BOOT_ERROR;
	} else if (uart_input_value == 'd') {
		sunxi_set_printf_debug_mode(8, 1);
		printf("detected user input d\n");
	} else if (uart_input_value == 'q') {
		printf("detected user input q\n");
		sunxi_set_printf_debug_mode(0, 1);
	}
	sunxi_bootparam_load();
#ifdef FPGA_PLATFORM
	dram_size = sunxi_fpga_dram_init((void *)sunxi_bootparam_get_dram_buf());
#else
	dram_size = init_DRAM(0, (void *)sunxi_bootparam_get_dram_buf());
#endif
	if (!dram_size) {
		printf("init dram fail\n");
		goto _BOOT_ERROR;
	} else {
		if (toc0_config->dram_size > 0)
			dram_size = toc0_config->dram_size;
		printf("dram size =%d\n", dram_size);
	}

	mmu_enable(dram_size);
	malloc_init(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE);

	boot_info("secure:flash map:boot_param:%d,uboot_start_sector:%d,uboot_start_bak_sector:%d\n",
				sboot_head.flash_map.boot_param,
				sboot_head.flash_map.uboot_start_sector,
				sboot_head.flash_map.uboot_bak_start_sector);

	ret = sunxi_board_late_init();
	if (ret)
		goto _BOOT_ERROR;

	if (toc0->platform[0] & 0xf0)
		printf("read toc0 from emmc backup\n");

	ret = sunxi_flash_init(toc0->platform[0] & 0x0f);
	if (ret)
		goto _BOOT_ERROR;

	ret = toc1_init();
	if (ret)
		goto _BOOT_ERROR;

	ret = toc1_verify_and_run(dram_size, pmu_type, uart_input_value, key_input);
	if (ret)
		goto _BOOT_ERROR;

_BOOT_ERROR:
	sboot_clear_env();
	boot0_jmp(SECURE_FEL_BASE);
}

static int sboot_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mmu_disable();
	mdelay(10);
	return 0;
}
