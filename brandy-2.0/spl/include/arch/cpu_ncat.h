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


#ifndef __PLATFORM_H
#define __PLATFORM_H
#if defined(CONFIG_ARCH_SUN50IW10P1)
#define SUNXI_SRAM_A1_BASE               (0x00020000L)
#define SUNXI_SRAM_A2_BASE               (0x00100000L)
#define SUNXI_SRAM_C_BASE                (0x00024000L)
#else
#define SUNXI_SRAM_A1_BASE               (0x00020000L)
#define SUNXI_SRAM_A2_BASE               (0x00100000L)
#define SUNXI_SRAM_C_BASE                (0x00028000L)
#endif
#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_CE_BASE                    (0x03040000L)
#else
#define SUNXI_CE_BASE                    (0x01904000L)
#endif
#define SUNXI_SS_BASE                    SUNXI_CE_BASE

//CPUX
#if defined(CONFIG_ARCH_SUN50IW10P1) || defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_CPUXCFG_BASE               (0x08100000L)
#else
#define SUNXI_CPUXCFG_BASE               (0x09010000L)
#endif
#define SUNXI_CPU_SUBSYS_CTRL_BASE	(0x08100000L)
//sys ctrl
#define SUNXI_SYSCRL_BASE                (0x03000000L)
#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_CCM_BASE					(0x02001000L)
#else
#define SUNXI_CCM_BASE					(0x03001000L)
#endif
#define SUNXI_DMA_BASE                   (0x03002000L)
#define SUNXI_MSGBOX_BASE                (0x03003000L)
#define SUNXI_SPINLOCK_BASE              (0x03004000L)
#define SUNXI_HSTMR_BASE                 (0x03005000L)
#define SUNXI_SID_BASE                   (0x03006000L)

#if defined(CONFIG_ARCH_SUN50IW10P1)
#define SUNXI_SMC_BASE                   (0x04800000L)
#else
#define SUNXI_SMC_BASE                   (0x03007000L)
#endif

#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_SPC_BASE					(0x04800000L)
#else
#define SUNXI_SPC_BASE					(0x03008000L)
#endif

#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_TIMER_BASE				(0x02000000L)
#else
#define SUNXI_TIMER_BASE				(0x03009000L)
#endif
#define SUNXI_WDOG_BASE                  (0x030090A0L)
#define SUNXI_CNT64_BASE                 (0x03009C00L)
#define SUNXI_PWM_BASE                   (0x0300A000L)
#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_PIO_BASE                   (0x02000400L)
#else
#define SUNXI_PIO_BASE                   (0x0300B000L)
#endif
#define SUNXI_PSI_BASE                   (0x0300C000L)
#define SUNXI_DCU_BASE                   (0x03010000L)
#define SUNXI_GIC_BASE                   (0x03020000L)
#define SUNXI_IOMMU_BASE                 (0x030F0000L)


//storage
#define SUNXI_DRAMCTL0_BASE              (0x04002000L)
#define SUNXI_NFC_BASE                   (0x04011000L)
#define SUNXI_SMHC0_BASE                 (0x04020000L)
#define SUNXI_SMHC1_BASE                 (0x04021000L)
#define SUNXI_SMHC2_BASE                 (0x04022000L)

//noraml
#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_UART0_BASE					(0x02500000L)
#else
#define SUNXI_UART0_BASE					(0x05000000L)
#endif
#define SUNXI_UART1_BASE                  (0x05000400L)
#define SUNXI_UART2_BASE                  (0x05000800L)
#define SUNXI_UART3_BASE                  (0x05000c00L)
#define SUNXI_UART4_BASE                  (0x05001000L)

#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_TWI0_BASE                   (0x025002000L)
#else
#define SUNXI_TWI0_BASE                   (0x05002000L)
#endif
#define SUNXI_TWI1_BASE                   (0x05002400L)
#define SUNXI_TWI2_BASE                   (0x05002800L)

#define SUNXI_SCR0_BASE                   (0x05005000L)

#define SUNXI_SPI0_BASE                   (0x05010000L)
#define SUNXI_SPI1_BASE                   (0x05011000L)
#define SUNXI_GMAC_BASE                   (0x05020000L)

#define SUNXI_GPADC_BASE                  (0x05070000L)
#define SUNXI_LRADC_BASE                  (0x05070800L)
#define SUNXI_KEYADC_BASE                 SUNXI_LRADC_BASE

#define SUNXI_USBOTG_BASE                 (0x05100000L)
#define SUNXI_EHCI0_BASE                  (0x05310000L)
#define SUNXI_EHCI1_BASE                  (0x05311000L)

#define ARMV7_GIC_BASE                    (SUNXI_GIC_BASE+0x1000L)
#define ARMV7_CPUIF_BASE                  (SUNXI_GIC_BASE+0x2000L)

//cpus
#if defined(CONFIG_ARCH_SUN50IW11P1)
#define SUNXI_RTC_BASE                      (0x07090000L)
#else
#define SUNXI_RTC_BASE                      (0x07000000L)
#endif
#define SUNXI_CPUS_CFG_BASE                 (0x07000400L)
#define SUNXI_RCPUCFG_BASE                  (SUNXI_CPUS_CFG_BASE)
#define SUNXI_RPRCM_BASE                    (0x07010000L)
#define SUNXI_RPWM_BASE                     (0x07020c00L)
#define SUNXI_RPIO_BASE                     (0x07022000L)
#define SUNXI_R_PIO_BASE                    (0x07022000L)
#define SUNXI_RTWI_BASE                     (0x07081400L)
#define SUNXI_RRSB_BASE                     (0x07083000L)
#define SUNXI_RSB_BASE                      (0x07083000L)
#define SUNXI_RTWI_BRG_REG					(SUNXI_RPRCM_BASE+0x019c)
#define SUNXI_RTWI0_RST_BIT					(16)
#define SUNXI_RTWI0_GATING_BIT				(0)

#define SUNXI_RTC_DATA_BASE                 (SUNXI_RTC_BASE+0x100)

/* use for usb correct */
#if defined(CONFIG_ARCH_SUN8IW18P1)
#define VDD_SYS_PWROFF_GATING_REG			(SUNXI_RTC_BASE + 0x220)
#define RES_CAL_CTRL_REG                    (SUNXI_RTC_BASE + 0X230)
#elif defined(CONFIG_ARCH_SUN8IW19P1) || defined(CONFIG_ARCH_SUN50IW9P1) || defined(CONFIG_ARCH_SUN50IW5P1)
#define VDD_SYS_PWROFF_GATING_REG			(SUNXI_RPRCM_BASE + 0x250)
#define RES_CAL_CTRL_REG                    (SUNXI_RPRCM_BASE + 0X310)
#define VDD_ADDA_OFF_GATING					(4)
#define CAL_ANA_EN							(1)
#define CAL_EN								(0)
#elif defined(CONFIG_ARCH_SUN50IW10P1)
#define ANALOG_SYS_PWROFF_GATING_REG		(SUNXI_RPRCM_BASE + 0x254)
#define VDD_SYS_PWROFF_GATING_REG			ANALOG_SYS_PWROFF_GATING_REG
#define RES_CAL_CTRL_REG					(SUNXI_SYSCRL_BASE + 0x160)
#define VDD_ADDA_OFF_GATING					(1)
#define CAL_ANA_EN							(1)
#define CAL_EN								(0)

#else
#define VDD_SYS_PWROFF_GATING_REG			(SUNXI_RPRCM_BASE + 0x250)
#define RES_CAL_CTRL_REG                    (SUNXI_RPRCM_BASE + 0X310)
#define VDD_ADDA_OFF_GATING					(9)
#define CAL_ANA_EN							(1)
#define CAL_EN								(0)
#endif

#if defined(CONFIG_ARCH_SUN50IW9P1)
#define RVBARADDR0_L             (((readl(SUNXI_SYSCRL_BASE + 0x24) & 0x7) != 0x2) ? SUNXI_CPUXCFG_BASE+0x40 : SUNXI_CPU_SUBSYS_CTRL_BASE+0x40)
#define RVBARADDR0_H             (((readl(SUNXI_SYSCRL_BASE + 0x24) & 0x7) != 0x2) ? SUNXI_CPUXCFG_BASE+0x44 : SUNXI_CPU_SUBSYS_CTRL_BASE+0x44)
#else
#define RVBARADDR0_L             (SUNXI_CPUXCFG_BASE+0x40)
#define RVBARADDR0_H             (SUNXI_CPUXCFG_BASE+0x44)
#endif
#define SRAM_CONTRL_REG0         (SUNXI_SYSCRL_BASE+0x0)
#define SRAM_CONTRL_REG1         (SUNXI_SYSCRL_BASE+0x4)

#define GPIO_BIAS_MAX_LEN (32)
#define GPIO_BIAS_MAIN_NAME "gpio_bias"
#define GPIO_POW_MODE_REG (0x0340)
#define GPIO_POW_MODE_VAL_REG		(0x0348)
#define GPIO_3_3V_MODE 0
#define GPIO_1_8V_MODE 1

#endif
