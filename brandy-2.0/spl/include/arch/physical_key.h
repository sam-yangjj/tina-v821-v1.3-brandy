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

#ifndef _SUNXI_KEY_H
#define _SUNXI_KEY_H

#include "arch/cpu.h"

struct sunxi_lradc {
	volatile u32 ctrl;         /* lradc control */
	volatile u32 intc;         /* interrupt control */
	volatile u32 ints;         /* interrupt status */
	volatile u32 data0;        /* lradc 0 data */
	volatile u32 data1;        /* lradc 1 data */
};

#define SUNXI_KEY_ADC_CRTL        (SUNXI_KEYADC_BASE + 0x00)
#define SUNXI_KEY_ADC_INTC        (SUNXI_KEYADC_BASE + 0x04)
#define SUNXI_KEY_ADC_INTS        (SUNXI_KEYADC_BASE + 0x08)
#define SUNXI_KEY_ADC_DATA0       (SUNXI_KEYADC_BASE + 0x0C)

#define LRADC_EN                  (0x1)   /* LRADC enable */
#define LRADC_SAMPLE_RATE         0x2    /* 32.25 Hz */
#define LEVELB_VOL                0x2    /* 0x33(~1.6v) */
#define LRADC_HOLD_EN             (0x1 << 6)    /* sample hold enable */
#define KEY_MODE_SELECT           0x0    /* normal mode */

#define ADC0_DATA_PENDING         (1 << 0)    /* adc0 has data */
#define ADC0_KEYDOWN_PENDING      (1 << 1)    /* key down */
#define ADC0_HOLDKEY_PENDING      (1 << 2)    /* key hold */
#define ADC0_ALRDY_HOLD_PENDING   (1 << 3)    /* key already hold */
#define ADC0_KEYUP_PENDING        (1 << 4)    /* key up */

#define GP_SR_CON		(SUNXI_GPADC_BASE + 0x0)
#define GP_CTRL			(SUNXI_GPADC_BASE + 0x04)
#define GP_CS_EN		(SUNXI_GPADC_BASE + 0x08)
#define GP_CDATA		(SUNXI_GPADC_BASE + 0x18)
#define GP_DATA_INTC		(SUNXI_GPADC_BASE + 0x28)
#define GP_DATA_INTS		(SUNXI_GPADC_BASE + 0x38)
#define GP_CH0_DATA    		(SUNXI_GPADC_BASE + 0x80)
#define GP_BOOTSTRAP_EN_REG	(SUNXI_GPADC_BASE + 0x160)

#define GPADC0_DATA_PENDING	(1 << 0)	/* gpadc0 has data */


int sunxi_key_init(void);

int sunxi_key_exit(void);

int sunxi_key_read(void);

int sunxi_key_probe(void);

int sunxi_key_clock_open(void);

int sunxi_key_clock_close(void);

#endif
