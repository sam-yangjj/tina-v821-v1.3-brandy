/*
 * (C) Copyright 2018 allwinnertech  <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SBI_H__
#define __SBI_H__

#include <asm/io.h>

int sbi_ecall_efuse_write(void *key_buf);
int sbi_ecall_efuse_read(void *key_buf, void *read_buf, void *read_len);
void sbi_set_arch_timer(uint64_t stime_value);
void sbi_andes_set_ultra_standby(u32 cpux_shutdown_type);
void sbi_srst_reset(unsigned long type, unsigned long reason);

#endif
