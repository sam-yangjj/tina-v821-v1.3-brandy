/*
 * Allwinner Sun251iw1 clock register definitions
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <asm/arch/plat-sun251iw1p1/cpu_autogen.h>

#ifndef _SUNXI_CPU_SUN251IW1_H
#define _SUNXI_CPU_SUN251IW1_H

#define SUNXI_CCM_BASE		(SUNXI_CCMU_BASE)

#define SUNXI_PIO_BASE		(SUNXI_GPIO_BASE)

#define SUNXI_DMA_BASE      (SUNXI_DMAC_BASE)

#define SUNXI_PLIC_BASE     (SUNXI_RISCV_PLIC_BASE)

#define SUNXI_USBOTG_BASE	(SUNXI_USB0_BASE)

#define SUNXI_RTC_DATA_BASE (SUNXI_RTC_BASE + 0x100)

#endif /* _SUNXI_CPU_SUN251IW1_H */
