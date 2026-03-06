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
 * (C) Copyright 2018
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#ifndef _SUNXI_EFUSE_H
#define _SUNXI_EFUSE_H

#include <linux/types.h>
#include <config.h>
#include <arch/cpu.h>

/* clock control module regs definition */
#if defined(CONFIG_ARCH_SUN50IW3)
#include <arch/efuse_sun50iw3.h>
#elif defined(CONFIG_ARCH_SUN50IW9)
#include <arch/efuse_sun50iw9.h>
#elif defined(CONFIG_ARCH_SUN50IW10)
#include <arch/efuse_sun50iw10.h>
#elif defined(CONFIG_ARCH_SUN50IW11)
#include <arch/efuse_sun50iw11.h>
#elif defined(CONFIG_ARCH_SUN50IW12)
#include <arch/efuse_sun50iw12.h>
#elif defined(CONFIG_ARCH_SUN8IW15)
#include <arch/efuse_sun8iw15.h>
#elif defined(CONFIG_ARCH_SUN8IW18)
#include <arch/efuse_sun8iw18.h>
#elif defined(CONFIG_ARCH_SUN8IW7)
#include <arch/efuse_sun8iw7.h>
#elif defined(CONFIG_ARCH_SUN20IW1)
#include <arch/efuse_sun20iw1.h>
#elif defined(CONFIG_ARCH_SUN300IW1)
#include <arch/sun300iw1p1/efuse_plat.h>
#elif defined(CONFIG_ARCH_SUN8IW20)
#include <arch/efuse_sun8iw20.h>
#elif defined(CONFIG_ARCH_SUN8IW21)
#include <arch/efuse_sun8iw21.h>
#elif defined(CONFIG_ARCH_SUN50IW5)
#include <arch/efuse_sun50iw5.h>
#elif defined(CONFIG_ARCH_SUN8IW11)
#include <arch/efuse_sun8iw11.h>
#elif defined(CONFIG_ARCH_SUN60IW1)
#include <arch/sun60iw1p1/efuse_sun60iw1.h>
#elif defined(CONFIG_ARCH_SUN60IW2)
#include <arch/sun60iw2p1/efuse_sun60iw2.h>
#elif defined(CONFIG_ARCH_SUN55IW3)
#include <arch/efuse_sun55iw3.h>
#elif defined(CONFIG_ARCH_SUN55IW5)
#include <arch/efuse_sun55iw5.h>
#elif defined(CONFIG_ARCH_SUN55IW6)
#include <arch/sun55iw6p1/efuse_sun55iw6.h>
#elif defined(CONFIG_ARCH_SUN65IW1)
#include <arch/sun65iw1p1/efuse_sun65iw1.h>
#elif defined(CONFIG_ARCH_SUN251IW1)
#include <arch/sun251iw1p1/efuse_sun251iw1.h>
#elif defined(CONFIG_ARCH_SUN50IW15)
#include <arch/efuse_sun50iw15.h>
#elif defined(CONFIG_ARCH_SUN252IW1)
#include <arch/sun252iw1p1/efuse_sun252iw1.h>
#else
#error "Unsupported plat"
#endif

#define JTAG_AT_SOURCE (1 << 0)
#define JTAG_DBGEN_CORE(x) ((1 << (x)) << 0)
#define JTAG_NIDEN_CORE(x) ((1 << (x)) << 4)
#define JTAG_SPIDEN_CORE(x) ((1 << (x)) << 8)
#define JTAG_SPNIDEN_CORE(x) ((1 << (x)) << 12)
#define JTAG_DEVICEEN (1 << 0)
#define JTAG_EFUSE_AT0_OFFSET (0x0)
#define JTAG_EFUSE_AT1_OFFSET (0x8)

#ifndef __ASSEMBLY__
uint sid_sram_read(uint key_index);
uint sid_read_key(uint key_index);
void sid_program_key(uint key_index, uint key_value);
#if defined(CFG_EFUSE_BURNING)
int  sid_update_key(uint32_t key_index, uint32_t key_value, uint32_t mask);
#endif
void sid_set_security_mode(void);
int  sid_probe_security_mode(void);
void sid_read_rotpk(void *dst);
void sid_disable_jtag(void);
void sid_close_fel(void);
void sid_enable_verify_fel(void);
void sunxi_key_provision(void);
#endif /* !__ASSEMBLY__ */

#endif /* _SUNXI_EFUSE_H */
