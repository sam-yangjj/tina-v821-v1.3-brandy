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
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <common.h>
#include <asm/io.h>
#include <arch/uart.h>
#include <arch/gpio.h>
#include <arch/clock.h>

#define thr rbr
#define dll rbr
#define dlh ier
#define iir fcr


static serial_hw_t *serial_ctrl_base;


void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max)
{
	u32 uart_clk;
	void __iomem *uart0_base = sunxi_get_iobase(SUNXI_UART0_BASE);

#ifdef FPGA_PLATFORM
	normal_gpio_set_t fpga_uart_gpio[2] = {
	    {2, 9, 3, 1, 1, 0, {0} }, /* PB9: 3--RX */
	    {2, 8, 3, 1, 1, 0, {0} }, /* PB8: 3--TX */
	};
#endif

	sunxi_clock_init_uart(uart_port);
	/* gpio */
#ifdef FPGA_PLATFORM
	boot_set_gpio(fpga_uart_gpio, gpio_max, 1);
#else
	boot_set_gpio(gpio_cfg, gpio_max, 1);
#endif
	/* uart init */
	serial_ctrl_base = (serial_hw_t *)(uart0_base + uart_port * CCM_UART_ADDR_OFFSET);

	serial_ctrl_base->mcr = 0x3;
	uart_clk = (CCM_UART_CLK + 8 * UART_BAUD)/(16 * UART_BAUD);
	serial_ctrl_base->lcr |= 0x80;
	serial_ctrl_base->dlh = uart_clk>>8;
	serial_ctrl_base->dll = uart_clk&0xff;
	serial_ctrl_base->lcr &= ~0x80;
	serial_ctrl_base->lcr = ((PARITY&0x03)<<3) | ((STOP&0x01)<<2) | (DLEN&0x03);
	serial_ctrl_base->fcr = 0x7;

	return;
}

void sunxi_serial_putc (char c)
{
	while ((serial_ctrl_base->lsr & (1 << 6)) == 0)
		;
	serial_ctrl_base->thr = c;
}

char sunxi_serial_getc (void)
{
	while ((serial_ctrl_base->lsr & 1) == 0)
		;
	return serial_ctrl_base->rbr;
}

int sunxi_serial_tstc (void)
{
	return serial_ctrl_base->lsr & 1;
}
