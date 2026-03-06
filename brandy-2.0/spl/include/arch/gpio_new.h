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
#ifndef _SUNXI_GPIO_NEW_H
#define _SUNXI_GPIO_NEW_H

#include <arch/gpio.h>

#define OUPUT_HIGH_LEVEL (1)
#define OUPUT_LOW_LEVEL	 (0)
#define PORT_NO_USE	 (0xFF)
#define PORT_NUM_NO_USE	 (0xFF)
/**#############################################################################################################
 *
 *                           GPIO(PIN) NEW Operations
 *
-##############################################################################################################*/
#define NEW_PIO_REG_CFG(n, i)                                                  \
	((volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +               \
				   (n)*PIOC_o_OFFSET + ((i) << 2) +            \
				   PIOC_REG_o_CFG0))
#define NEW_PIO_REG_DLEVEL(n, i)                                               \
	((volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +               \
				   (n)*PIOC_o_OFFSET + ((i) << 2) +            \
				   PIOC_REG_o_DRV0))
#define NEW_PIO_REG_PULL(n, i)                                                 \
	((volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +               \
				   (n)*PIOC_o_OFFSET + ((i) << 2) +            \
				   PIOC_REG_o_PUL0))
#define NEW_PIO_REG_DATA(n)                                                    \
	((volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +               \
				   (n)*PIOC_o_OFFSET + PIOC_REG_o_DATA))
#define NEW_PIO_ONE_PIN_DATA(n, i)                                                 \
	(((*(volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +            \
				      (n) * PIOC_o_OFFSET +                \
				      PIOC_REG_o_DATA)) &                      \
	  (1 << i)) >>                                                         \
	 i)

#define NEW_PIO_REG_CFG_VALUE(n, i)                                            \
	readl(SUNXI_PIO_BASE + (n)*PIOC_o_OFFSET + ((i) << 2) + PIOC_REG_o_CFG0)
#define NEW_PIO_REG_DLEVEL_VALUE(n, i)                                         \
	readl(SUNXI_PIO_BASE + (n)*PIOC_o_OFFSET + ((i) << 2) + PIOC_REG_o_DRV0)
#define NEW_PIO_REG_PULL_VALUE(n, i)                                           \
	readl(SUNXI_PIO_BASE + (n)*PIOC_o_OFFSET + ((i) << 2) + PIOC_REG_o_PUL0)
#define NEW_PIO_REG_DATA_VALUE(n)                                              \
	readl(SUNXI_PIO_BASE + (n)*PIOC_o_OFFSET + PIOC_REG_o_DATA)
#define NEW_PIO_REG_BASE(n)                                                    \
	((volatile unsigned int *)((phys_addr_t)SUNXI_PIO_BASE +               \
				   (n)*PIOC_o_OFFSET))

#ifdef SUNXI_R_PIO_BASE
#define R_NEW_PIO_REG_CFG(n, i)                                                \
	((volatile unsigned int *)((phys_addr_t)SUNXI_R_PIO_BASE +             \
				   ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +     \
				   PIOC_REG_o_CFG0))
#define R_NEW_PIO_REG_DLEVEL(n, i)                                             \
	((volatile unsigned int *)((phys_addr_t)SUNXI_R_PIO_BASE +             \
				   ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +     \
				   PIOC_REG_o_DRV0))
#define R_NEW_PIO_REG_PULL(n, i)                                               \
	((volatile unsigned int *)((phys_addr_t)SUNXI_R_PIO_BASE +             \
				   ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +     \
				   PIOC_REG_o_PUL0))
#define R_NEW_PIO_REG_DATA(n)                                                  \
	((volatile unsigned int *)((phys_addr_t)SUNXI_R_PIO_BASE +             \
				   ((n)-11) * PIOC_o_OFFSET +                  \
				   PIOC_REG_o_DATA))

#define R_NEW_PIO_REG_CFG_VALUE(n, i)                                          \
	readl(SUNXI_R_PIO_BASE + ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +       \
	      PIOC_REG_o_CFG0)
#define R_NEW_PIO_REG_DLEVEL_VALUE(n, i)                                       \
	readl(SUNXI_R_PIO_BASE + ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +       \
	      PIOC_REG_o_DRV0)
#define R_NEW_PIO_REG_PULL_VALUE(n, i)                                         \
	readl(SUNXI_R_PIO_BASE + ((n)-11) * PIOC_o_OFFSET + ((i) << 2) +       \
	      PIOC_REG_o_PUL0)
#define R_NEW_PIO_REG_DATA_VALUE(n)                                            \
	readl(SUNXI_R_PIO_BASE + ((n)-11) * PIOC_o_OFFSET + PIOC_REG_o_DATA)
#define R_NEW_PIO_REG_BASE(n)                                                  \
	((volatile unsigned int *)((phys_addr_t)SUNXI_R_PIO_BASE +             \
				   ((n)-11) * PIOC_o_OFFSET))
#endif

#ifdef CFG_SET_GPIO_NEW
int boot_set_gpio_new(void *user_gpio_list, u32 group_count_max, int set_gpio);
#endif

#endif