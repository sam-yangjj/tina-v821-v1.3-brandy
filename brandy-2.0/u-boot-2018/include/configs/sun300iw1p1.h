/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* #define FPGA_PLATFORM */

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_EHCI_SUNXI
//#ifdef CONFIG_SUNXI_RTOS
//#define CONFIG_USB_MAX_CONTROLLER_COUNT			1
//#else
#define CONFIG_USB_MAX_CONTROLLER_COUNT 		1
//#endif
#endif

#define CONFIG_SUNXI_USB_PHYS	1

/* sram layout*/

// #define SUNXI_SYS_SRAM_BASE (SUNXI_SRAM_C_BASE)
// #define SUNXI_SYS_SRAM_SIZE (0x02021c00 - SUNXI_SRAM_C_BASE)
/* Platform special configuration */
#define SUNXI_SYS_SRAM_BASE (0x82200000 - 0x20000)
#define SUNXI_SYS_SRAM_SIZE (0x20000) /*128k*/

#define CONFIG_SYS_BOOTM_LEN 0x2000000
#define PHOENIX_PRIV_DATA_ADDR (0x02020800)

#define SUNXI_SYS_MALLOC_LEN	CONFIG_SUNXI_MALLOC_LEN

#define AW_SYS_NS16550_CLK		192000000

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#if !IS_ENABLED(CONFIG_RISCV)
#define CONFIG_SYS_NONCACHED_MEMORY (1 << 20)
#endif

#endif /* __CONFIG_H */
