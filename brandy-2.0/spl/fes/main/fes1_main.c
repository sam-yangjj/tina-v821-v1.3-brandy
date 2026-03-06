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
#include <private_uboot.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/rtc.h>
#include <boot_param.h>

static void  note_dram_log(int dram_init_flag);


int main(void)
{
	int dram_size=0;
	int status;

	sunxi_fes_board_init_early();
	sunxi_serial_init(fes1_head.prvt_head.uart_port, (void *)fes1_head.prvt_head.uart_ctrl, 2);
	printf("fes begin commit:%s\n", fes1_head.hash);
	status = sunxi_board_init();
	if(status)
		return 0;
	sunxi_bootparam_load();
	printf("beign to init dram\n");
#ifdef FPGA_PLATFORM
	dram_size = sunxi_fpga_dram_init((void *)sunxi_bootparam_get_dram_buf());
#else
	dram_size = init_DRAM(0, (void *)sunxi_bootparam_get_dram_buf());
#endif
	if (rtc_probe_fel_flag()) {
		rtc_clear_fel_flag();
	}
	if (dram_size) {
		note_dram_log(1);
		printf("init dram ok\n");
	} else {
		note_dram_log(0);
		printf("init dram fail\n");
#ifdef CFG_ARCH_RISCV
		asm volatile("j .");
#else
		asm volatile("b .");
#endif
	}

	mdelay(10);

	return dram_size;
}

static void note_dram_log(int dram_init_flag)
{
	fes_aide_info_t *fes_aide =
		(fes_aide_info_t *)&fes1_head.fes_union_addr.fes1_res_addr;

	memset(fes_aide, 0, sizeof(fes_aide_info_t));
	fes_aide->dram_init_flag   = SYS_PARA_LOG;
	fes_aide->dram_update_flag = dram_init_flag;

	memcpy(fes_aide->dram_paras, fes1_head.prvt_head.dram_para,
	       SUNXI_DRAM_PARA_MAX * 4);
	memcpy((void *)DRAM_PARA_STORE_ADDR, fes1_head.prvt_head.dram_para,
	       SUNXI_DRAM_PARA_MAX * 4);

#ifdef CFG_SUNXI_BOOT_PARAM
#define BOOT_PARAM_FES_TRANSFER_UBOOT_ADDR                                     \
	(DRAM_PARA_STORE_ADDR + SUNXI_DRAM_PARA_MAX * 4)
	sunxi_bootparam_format_and_transfer((void *)BOOT_PARAM_FES_TRANSFER_UBOOT_ADDR);
#endif
}
