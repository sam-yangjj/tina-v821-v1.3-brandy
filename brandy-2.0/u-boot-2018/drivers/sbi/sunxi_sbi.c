/*
 * (C) Copyright 2017-2018
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <efuse_map.h>
#include <asm/sbi.h>
#include <sbi.h>

DECLARE_GLOBAL_DATA_PTR;

#define SBI_EXT_SUNXI			0x54535251
#define SBI_EXT_SUNXI_EFUSE_WRITE	0
#define SBI_EXT_SUNXI_EFUSE_READ	1

#define SBI_EXT_TIME			0x54494D45
#define SBI_EXT_TIME_SET_TIMER	0

#define SBI_EXT_SRST			0x53525354
/* SBI function IDs for SRST extension */
#define SBI_EXT_SRST_RESET			0x0

int sbi_ecall_efuse_write(void *key_buf)
{
	struct sbiret ret;
	efuse_key_info_t *keyinfo = (efuse_key_info_t *)key_buf;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_EFUSE_WRITE, (unsigned long)keyinfo, 0, 0, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return ret.error;
	}
	return ret.value;
}

int sbi_ecall_efuse_read(void *key_buf, void *read_buf, void *read_len)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_EFUSE_READ, (unsigned long)key_buf, (unsigned long)read_buf, (unsigned long)read_len, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return ret.error;
	}
	return ret.value;
}

void sbi_set_arch_timer(uint64_t stime_value)
{
	sbi_ecall(SBI_EXT_TIME, SBI_EXT_TIME_SET_TIMER, stime_value,
		stime_value >> 32, 0, 0, 0, 0);
}

void sbi_srst_reset(unsigned long type, unsigned long reason)
{
	sbi_ecall(SBI_EXT_SRST, SBI_EXT_SRST_RESET, type, reason, 0, 0, 0, 0);
}

void sbi_andes_set_ultra_standby(u32 cpux_shutdown_type)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_ULTRA_STANDBY, cpux_shutdown_type, 0, 0, 0, 0, 0);
}