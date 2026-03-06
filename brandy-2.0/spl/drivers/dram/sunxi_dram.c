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
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <weidonghui@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <asm/io.h>
#include <arch/gpio.h>
#include <arch/dram.h>
#ifdef CFG_SUNXI_EFUSE
#include <arch/efuse.h>
#endif

// #define AUTO_DRAM_DEBUG

#define ADC_MASK 0x0f
#define ADCS_MASK 0xf0

#ifdef AUTO_DRAM_DEBUG
#define DRAM_DBG(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define DRAM_INF(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define DRAM_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)

#else
#define DRAM_DBG(fmt, arg...)
#define DRAM_INF(fmt, arg...)
#define DRAM_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#endif


int sunxi_read_gpadc_vol(int channel);

#ifdef FPGA_PLATFORM
unsigned int sunxi_fpga_dram_init(void *para)
{
#ifdef CFG_SUNXI_FPGA_BTIFILE_INIT_DRAM
	int ret = -1;
	ret = sunxi_fpga_bitfile_init_dram();
	if (ret > 0) {
		return ret;
	}

	return ret;
#else
	return mctl_init(para);
#endif
}
#endif

u32 get_boot_dram_training_enable_flag(const uint32_t *dram_para)
{
	return ((dram_para[30] & (1 << 11)) != 0);
}

u32 get_boot_dram_update_flag(uint32_t *dram_para)
{
	return (dram_para[23] >> 31) & 0x1;
}


#ifdef CFG_SUNXI_SELECT_DRAM_PARA
int sunxi_dram_handle(void)
{
	int i, vaild_para = 1, io_en = 0, dram_para_total = 7;
	uint32_t *dram_para, select_dram_para = 0;
	boot_extend_head_t *select_para;
	__maybe_unused u32 vol_range[] = {163, 382, 608, 811, 1050, 1315, 1569, 1800}; /*mV*/
#ifdef CFG_SUNXI_SBOOT
	dram_para   = (uint32_t *)toc0_config->dram_para;
	select_para = (boot_extend_head_t *)&sboot_head.extd_head;
#elif CFG_SUNXI_FES
	dram_para   = (uint32_t *)fes1_head.prvt_head.dram_para;
	select_para = (boot_extend_head_t *)&fes1_head.fes_union_addr.extd_head;
#else
	dram_para   = (uint32_t *)BT0_head.prvt_head.dram_para;
	select_para = (boot_extend_head_t *)&BT0_head.fes_union_addr.extd_head;
#endif

	if (!get_boot_dram_update_flag(dram_para)) {
		if (!strncmp((char *)select_para->magic, DRAM_EXT_MAGIC, strlen(DRAM_EXT_MAGIC))) {
#ifdef AUTO_DRAM_DEBUG
			for (i = 0; i < 32; i++) {
				printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
			}
			printf("select_mode:%d\n", select_para->select_mode);
			printf("adc_channel:%d\n", select_para->gpadc_channel & ADC_MASK);
#endif

			boot_set_gpio((void *)select_para->dram_select_gpio, 6, 1);
			if (select_para->select_mode == 1) {
				for (i = 0; i < 4; i++) {
					if (select_para->dram_select_gpio[i].port == 0)
						continue;
					select_dram_para |= (PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num)) << i);
				}
				dram_para_total =
					sizeof(select_para->dram_para) /
					sizeof(select_para->dram_para[0]);

			} else if (select_para->select_mode != 0) {
#ifdef CFG_GPADC_KEY
				/* read first adc */
				int adc_vol = sunxi_read_gpadc_vol(select_para->gpadc_channel & ADC_MASK);
				DRAM_DBG("adc_vol:%d mv\n", adc_vol);
				for (select_dram_para = 0; select_dram_para < (sizeof(vol_range)/sizeof(vol_range[0]) - 1); select_dram_para++) {
					if ((adc_vol < (vol_range[select_dram_para+1] + vol_range[select_dram_para]) / 2)) {
						break;
					}
				}
#endif
				/* read aother gpio */
				if (select_para->select_mode == 3) {
					for (i = 0; i < 4; i++) {
						if (select_para->dram_select_gpio[i].port == 0)
							continue;
#ifdef CFG_DRAM_SELECT_VERSION_2_0
						io_en |= (PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num)) << i);
#else
						io_en = PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num));
						break;
#endif
					}
					dram_para_total =
						sizeof(select_para->dram_para) /
						sizeof(select_para
							       ->dram_para[0]);
					printf("dram_para_total:0x%x\n", dram_para_total);
				}

				/* read aother adc */
				if (select_para->select_mode == 4) {
					int adc_vol = sunxi_read_gpadc_vol((select_para->gpadc_channel & ADCS_MASK) >> 4);
					DRAM_DBG("adc:%d\n", (select_para->gpadc_channel & ADCS_MASK) >> 4);
					DRAM_DBG("adc_vol:%d mv\n", adc_vol);
					if (adc_vol <= 1818 && adc_vol >= 1782) {
						io_en = 1;
					}
					dram_para_total =
						sizeof(select_para->dram_para) /
						sizeof(select_para
							       ->dram_para[0]);
					printf("dram_para_total:0x%x\n", dram_para_total);

				}
			}
			DRAM_DBG("adc sel:0x%x\n", select_dram_para);
			DRAM_DBG("pin sel:0x%x\n", io_en);

			select_dram_para += (io_en * 8);
			DRAM_DBG("select_dram_para:0x%x\n", select_dram_para);
			if (select_dram_para != 0) {
				if (select_para->dram_para[select_dram_para-1][0] == 0) {
					printf("dram para%d invalid use default para\n", select_dram_para);
					select_dram_para = 0;
				} else {
					memcpy(dram_para, select_para->dram_para[select_dram_para-1], sizeof(select_para->dram_para[0]));
				}
			}

			for (i = 0; i < dram_para_total; i++) {
				if (select_para->dram_para[i][0] == 0) {
					continue;
				}
				vaild_para++;
			}
			printf("vaild para:%d  select dram para%d\n", vaild_para,  select_dram_para);
#ifdef AUTO_DRAM_DEBUG
			while (1) {
				char uart_val = get_uart_input();
				if (uart_val == 'q')
					break;
				if (uart_val == 'w') {
					for (i = 0; i < 4; i++) {
						if (!select_para->dram_select_gpio[i].port)
							continue;
						printf("GPIO%c%d:0x%x\n", 'A'-1 + select_para->dram_select_gpio[i].port,
							select_para->dram_select_gpio[i].port_num,
							PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num)));
					}
					printf("gpadc val:0x%x\n", sunxi_read_gpadc_vol(select_para->gpadc_channel));
				}
				mdelay(10);
			}
			for (i = 0; i < 32; i++) {
				printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
			}
#endif
		} else {
			printf("extd_head bad magic\n");
			return -1;
		}
	} else {
		printf("dram return write ok\n");
	}
	if (sunxi_get_printf_debug_mode() >= 4) {
		for (i = 0; i < 32; i++) {
			printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
		}
	}
	return 0;
}
#endif

#ifdef CFG_DDR_SOFT_TRAIN
void sunxi_dram_soft_train(uint32_t *dram_para)
{
	if (get_boot_dram_training_enable_flag(dram_para)) {
#ifdef CFG_SUNXI_FES
		if (sid_probe_security_mode()) {
			printf("not support training dram in secure board\n");
		} else
#endif
		{
			neon_enable();
		}
	}
}
#endif
