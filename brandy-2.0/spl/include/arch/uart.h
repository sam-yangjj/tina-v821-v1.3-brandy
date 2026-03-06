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

#ifndef _UART_H_
#define _UART_H_
#include <arch/cpu.h>
#define CCM_UART_RST_OFFSET       (16)
#define CCM_UART_GATING_OFFSET    (0)

#ifdef CCM_UART_PLATFORM_ADDR_OFFSET
#define CCM_UART_ADDR_OFFSET      (CCM_UART_PLATFORM_ADDR_OFFSET)
#else
#define CCM_UART_ADDR_OFFSET      (0x400)
#endif /* CCM_UART_PLATFORM_ADDR_OFFSET*/


#ifdef CCM_UART_PLATFORM_CLK
#define CCM_UART_CLK              (CCM_UART_PLATFORM_CLK)
#else
#define CCM_UART_CLK              (24000000)
#endif /* CCM_UART_PLATFORM_CLK */

typedef struct serial_hw
{
	volatile unsigned int rbr;		/* 0 */
	volatile unsigned int ier;		/* 1 */
	volatile unsigned int fcr;		/* 2 */
	volatile unsigned int lcr;		/* 3 */
	volatile unsigned int mcr;		/* 4 */
	volatile unsigned int lsr;		/* 5 */
	volatile unsigned int msr;		/* 6 */
	volatile unsigned int sch;		/* 7 */
}serial_hw_t;

#define thr rbr
#define dll rbr
#define dlh ier
#define iir fcr


/* Baud rate for UART,Compute the divisor factor */
#ifdef CFG_UART_BAUD_1500000
#define   UART_BAUD    (1500000)
#else
#define   UART_BAUD    115200
#endif

/* UART Line Control Parameter */
/* Parity: 0,2 - no parity; 1 - odd parity; 3 - even parity */
#define   PARITY       0 
/* Number of Stop Bit: 0 - 1bit; 1 - 2(or 1.5)bits */
#define   STOP         0
/* Data Length: 0 - 5bits; 1 - 6bits; 2 - 7bits; 3 - 8bit */
#define   DLEN         3 


void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max);
void sunxi_serial_putc (char c);
char sunxi_serial_getc (void);
int sunxi_serial_tstc (void);


#endif    /*  #ifndef _UART_H_  */
