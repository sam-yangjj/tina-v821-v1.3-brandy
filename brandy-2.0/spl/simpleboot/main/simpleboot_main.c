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
 * (C) Copyright 2018
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <arch/clock.h>
#include <arch/rtc.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/gpio.h>
#include <config.h>
#include <boot_param.h>

static int boot0_clear_env(void);

extern char boot0_bin_start[];
extern char boot0_bin_end[];

static void print_commit_log(void)
{
	sunxi_serial_putc('H');
	sunxi_serial_putc('I');
	sunxi_serial_putc(' ');
	sunxi_serial_putc('S');
	sunxi_serial_putc('i');
	sunxi_serial_putc('m');
	sunxi_serial_putc('p');
	sunxi_serial_putc('l');
	sunxi_serial_putc('e');
	sunxi_set_printf_debug_mode(BT0_head.prvt_head.debug_mode, 0);
}

void main(void)
{
	int status;
	int dram_size;
	boot0_file_head_t *new = (boot0_file_head_t *)CFG_BOOT0_RUN_ADDR;

	record_time("brom");

	sunxi_board_init_early();
	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6);
	print_commit_log();

	record_time("print");

	status = sunxi_board_init();
	if (status)
		goto _BOOT_ERROR;
	record_time("board_init");

#ifdef FPGA_PLATFORM
	dram_size = sunxi_fpga_dram_init((void *)sunxi_bootparam_get_dram_buf());
#else
	dram_size = init_DRAM(0, (void *)sunxi_bootparam_get_dram_buf());
#endif
	record_time("init_dram");
	if (!dram_size) {
		printf("init dram fail\n");
		goto _BOOT_ERROR;
	} else {
		printf("dram size =%d\n", dram_size);
	}

void *memcpy_large(void *dst0, const void *src0, size_t len0);
	/* copy really boot bin */
	memcpy_large((void *)CFG_BOOT0_RUN_ADDR, boot0_bin_start,
		(u32)boot0_bin_end - (u32)boot0_bin_start);

	/* update BT0_head */
	memcpy(&new->prvt_head, &BT0_head.prvt_head, sizeof(*new) - sizeof(boot_file_head_t));
	new->dram_size = dram_size;
	record_time("copy_boot0");

	flush_dcache_range(CFG_BOOT0_RUN_ADDR, CFG_BOOT0_RUN_ADDR + \
			((u32)boot0_bin_end - (u32)boot0_bin_start));
	record_time("flush_cache");

#if defined(CFG_RECORD_BOOTTIME) && defined(CFG_BOOTTIME_RTC_IDX)
	rtc_write_data(CFG_BOOTTIME_RTC_IDX, (unsigned long)boottime);
#endif
	boot0_jmp(CFG_BOOT0_RUN_ADDR);

	printf("Jump to second boot failed! Stay in the loop...\n");
	while (1)
		;

_BOOT_ERROR:
	printf("Failed preparing the second boot\n");
	boot0_clear_env();
	boot0_jmp(FEL_BASE);
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mdelay(10);

	return 0;
}
