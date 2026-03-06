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
 *
 * (C) Copyright 2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * frank <frank@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <arch/rtc.h>
#include <arch/watchdog.h>
#include <arch/clock.h>
#ifdef CFG_SUNXI_SBOOT
#include <arch/smc.h>
#endif

#define DRAM_CRC_MAGIC			(0x76543210)

typedef struct pm_dram_para {
	unsigned int selfresh_flag;
	unsigned int crc_en;
	unsigned int crc_start;
	unsigned int crc_len;
} pm_dram_para_t;

static pm_dram_para_t soc_dram_state;
static u32 before_crc;
static u32 after_crc;

static u32 standby_dram_crc_enable(pm_dram_para_t *pdram_state)
{
	return pdram_state->crc_en;
}

static u32 standby_dram_crc(pm_dram_para_t *pdram_state)
{
	u32 *pdata;
	u32 crc = 0;

	pdata = (u32 *)(pdram_state->crc_start);
	printf("src:0x%x\n", (unsigned int)pdata);
	printf("len addr = 0x%x, len:0x%x\n",
			(unsigned int) (&(pdram_state->crc_len)),
			pdram_state->crc_len);
	while (pdata < (u32 *)(pdram_state->crc_start +
				pdram_state->crc_len)) {
		crc += *pdata;
		pdata++;
	}
	printf("crc finish...\n");
	return crc;
}

static int probe_super_standby_flag(void)
{
	uint reg_value = 0;
	int standby_flag = 0;

	reg_value = readl(RTC_STANDBY_FLAG_REG);
	standby_flag = (reg_value & ~(0xfffe0000)) >> 16;
	printf("rtc standby flag is 0x%x, super standby flag is 0x%x\n",
			reg_value, standby_flag);
	writel(0, RTC_STANDBY_FLAG_REG);

	return standby_flag;
}

int super_standby_mode(void)
{
	uint reg_value = 0;
	int standby_flag = 0;

	reg_value = readl(RTC_STANDBY_FLAG_REG);
	standby_flag = (reg_value & ~(0xfffe0000)) >> 16;

	return standby_flag;
}

void handler_super_standby(void)
{
	if (probe_super_standby_flag()) {

		/* high 28 bits for magic number, low 4 bits for enable */
		if ((rtc_read_data(CRC_EN) & (~0xf)) == DRAM_CRC_MAGIC) {
			soc_dram_state.crc_en = rtc_read_data(CRC_EN) & 0xf;
			soc_dram_state.crc_start = rtc_read_data(CRC_START);
			soc_dram_state.crc_len = rtc_read_data(CRC_LEN);
		}

		if (standby_dram_crc_enable(&soc_dram_state)) {
			before_crc = rtc_read_data(CRC_VALUE_BEFORE);
			after_crc = standby_dram_crc(&soc_dram_state);
			printf("before_crc = 0x%x, after_crc = 0x%x\n",
					before_crc, after_crc);
			if (before_crc != after_crc) {
				printf("dram crc error ...\n");
				wdt_start(0);
				asm ("b .");
			}
		}
		/*
		 * /dram_enable_all_master(); Not implemented on fpga
		 */
		printf("find standby flag, jump to addr 0x%x\n",
			readl(RTC_STANDBY_SOFT_ENTRY_REG));
		/* FIX ME: need 500ms ?
		 *__msdelay(500);
		 */
#ifdef CFG_SUNXI_SBOOT
		/* Borrowing the standby jmp address
		 * to calculate the address of monitor or optee
		 */
		sunxi_smc_resume(readl(RTC_STANDBY_SOFT_ENTRY_REG) & 0xfff00000);
#endif
		udelay(20);
#ifdef CONFIG_MONITOR
		boot0_jmp_monitor(readl(RTC_STANDBY_SOFT_ENTRY_REG));
#else
		boot0_jmp(readl(RTC_STANDBY_SOFT_ENTRY_REG));
#endif
	}
}
