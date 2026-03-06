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
 * Sunxi RTC data area ops
 *
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <arch/rtc.h>

#define CRASHDUMP_RESET_FLAG                (0x5AA55AA5)
#define CRASHDUMP_RESET_READY               (0x5AA55AA6)
#define CRASHDUMP_REFRESH_READY             (0x5AA55AA7)
#define EFEX_FLAG                           (0x5AA5A55A)
#define RTC_INDEX  2

#ifdef CFG_FASTBOOT_EFEX
#define RTC_BOOT_INDEX                      6
#define RTC_BOOT_EFEX_FLAG                  (0x5A)
#endif

void rtc_write_data(int index, u32 val)
{
	void __iomem *rtc_base = sunxi_get_iobase(SUNXI_RTC_DATA_BASE);
	writel(val, rtc_base + index * 4);
}

u32 rtc_read_data(int index)
{
	void __iomem *rtc_base = sunxi_get_iobase(SUNXI_RTC_DATA_BASE);
	return readl(rtc_base + index * 4);
}

static void _rtc_clear_data(int index)
{
	do {
		rtc_write_data(index, 0);
		data_sync_barrier();
	} while (rtc_read_data(index) != 0);
}

static void _rtc_set_data(int index, u32 val)
{
	do {
		rtc_write_data(index, val);
		data_sync_barrier();
	} while (rtc_read_data(index) != val);
}

void rtc_clear_data(int index)
{
	_rtc_clear_data(index);
}

void rtc_set_data(int index, u32 val)
{
	_rtc_set_data(index, val);
}

#ifdef CFG_BOOT0_WIRTE_RTC_TO_ISP
/*
 *CFG_BOOT0_WIRTE_RTC_TO_ISP=y
 *CFG_ISPFLAG_RTC_INDEX=0x1
 *CFG_ISPFLAG_RTC_VALUE=0x1
 */

#if !defined(CFG_ISPFLAG_RTC_INDEX) ||                                     \
	!defined(CFG_ISPFLAG_RTC_VALUE)
#error CFG_ISPFLAG_RTC_INDEX CFG_ISPFLAG_RTC_VALUE  \
write rtc register for isp !
#endif

#endif

#ifdef CFG_BOOT0_WIRTE_RTC_TO_ISP
void rtc_set_isp_flag(void)
{
	u32 rtc_data = rtc_read_data(CFG_ISPFLAG_RTC_INDEX);
	rtc_set_data(CFG_ISPFLAG_RTC_INDEX, rtc_data | (1 << CFG_ISPFLAG_RTC_INDEX));
}

void rtc_clear_isp_flag(void)
{
	u32 rtc_data = rtc_read_data(CFG_ISPFLAG_RTC_INDEX);
	rtc_set_data(CFG_ISPFLAG_RTC_INDEX, rtc_data & ~(1 << CFG_ISPFLAG_RTC_VALUE));
}

#endif

void rtc_set_fel_flag(void)
{
	_rtc_set_data(RTC_INDEX, EFEX_FLAG);
}

u32 rtc_probe_fel_flag(void)
{
	u32 i, reg_value,  number;

#ifndef RTC_GDATA_NUMBER
	number = 8;  //default
#else
	number = RTC_GDATA_NUMBER;
	printf("rtc total: %d\n", number);
#endif

	for (i = 0; i < number; i++) {
		reg_value = rtc_read_data(i);
		if (reg_value)
			printf("rtc[%d] value = 0x%x\n", i, reg_value);
	}

	reg_value = rtc_read_data(RTC_INDEX);
	if (reg_value == EFEX_FLAG) {
		printf("eraly jump fel\n");
		return 1;
	} else if (reg_value == CRASHDUMP_RESET_FLAG){
		rtc_write_data(RTC_INDEX, CRASHDUMP_RESET_READY);
		do {
			mdelay(150);
			reg_value = rtc_read_data(RTC_INDEX);
		}
		while (reg_value != CRASHDUMP_REFRESH_READY);
		printf("carshdump mode , jump fel\n");
		return 1;
	}

#ifdef CFG_FASTBOOT_EFEX
	reg_value = rtc_read_data(RTC_BOOT_INDEX);
	if (reg_value == RTC_BOOT_EFEX_FLAG) {
		printf("early jump fel\n");
		return 1;
	}
#endif // CFG_FASTBOOT_EFEX
	return 0;
}

void rtc_clear_fel_flag(void)
{
	_rtc_clear_data(RTC_INDEX);
}

void rtc_set_hash_entry(phys_addr_t entry)
{
	do {
		rtc_write_data(RTC_INDEX, GET_LO32(entry));
		data_sync_barrier();
	} while (rtc_read_data(RTC_INDEX) != entry);
}
