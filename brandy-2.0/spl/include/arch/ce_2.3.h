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
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#ifndef _SS_H_
#define _SS_H_

#include <config.h>
#include <arch/cpu.h>

#define SS_N_BASE SUNXI_SS_BASE /* non security */
#define SS_S_BASE (SUNXI_SS_BASE + 0x800) /* security */

#define SS_TDQ (SS_N_BASE + 0x00 + 0x800 * ss_base_mode)
#define SS_ICR (SS_N_BASE + 0x08 + 0x800 * ss_base_mode)
#define SS_ISR (SS_N_BASE + 0x0C + 0x800 * ss_base_mode)
#define SS_TLR (SS_N_BASE + 0x10 + 0x800 * ss_base_mode)
#define SS_TSR (SS_N_BASE + 0x14 + 0x800 * ss_base_mode)
#define SS_ERR (SS_N_BASE + 0x18 + 0x800 * ss_base_mode)
#define SS_TPR (SS_N_BASE + 0x1C + 0x800 * ss_base_mode)
/* #define SS_VER          (SS_N_BASE + 0x90 + 0x800*ss_base_mode) */
#define SS_VER (SS_N_BASE + 0x90)

/* security */
#define SS_S_TDQ (SS_S_BASE + 0x00)
#define SS_S_CTR (SS_S_BASE + 0x04)
#define SS_S_ICR (SS_S_BASE + 0x08)
#define SS_S_ISR (SS_S_BASE + 0x0C)
#define SS_S_TLR (SS_S_BASE + 0x10)
#define SS_S_TSR (SS_S_BASE + 0x14)
#define SS_S_ERR (SS_S_BASE + 0x18)
#define SS_S_TPR (SS_S_BASE + 0x1C)

/* non security */
#define SS_N_TDQ (SS_N_BASE + 0x00)
#define SS_N_ICR (SS_N_BASE + 0x08)
#define SS_N_ISR (SS_N_BASE + 0x0C)
#define SS_N_TLR (SS_N_BASE + 0x10)
#define SS_N_TSR (SS_N_BASE + 0x14)
#define SS_N_ERR (SS_N_BASE + 0x18)
#define SS_N_TPR (SS_N_BASE + 0x1C)

#define SHA1_160_MODE 0
#define SHA2_256_MODE 1

#define ALG_SHA256    (0x13)
#define ALG_RSA	      (0x20)
#define ALG_ECC	      (0x21)
#define ALG_SM2	      (0x22)
#define TRNG_BYTE_LEN (32)

/*ctrl*/
#define CHN  (0) /*channel id*/
#define IVE  (8)
#define LPKG (12) /*last package*/
/*The data length is valid when it is the last packet and needs to be set to 1.*/
#define DLAV (13)
#define IE   (16)

#define MD5    (0)
#define SHA1   (1)
#define SHA244 (2)
#define SHA256 (3)
#define SHA384 (4)
#define SHA512 (5)
#define SM3    (6)

#define TRNG (2)

/*cmd*/
#define HASH_SEL 0
#define HME	 4
#define RGB_SEL	 8
#define SUB_CMD	 16

/*CE_TLR*/
#define SYMM_TRPE     0
#define HASH_RBG_TRPE 1
#define ASYM_TRPE     2
#define RAES_TRPE     3

/*CE_ISR*/
#define SUCCESS 0x1
#define FAIL	0x2
#define CLEAN	0x3

#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3

#if defined(CONFIG_ARCH_SUN55IW6)
#define CE_REG_TRNG_ENT		SS_N_ICR
#define CE_DBL_ENT_SRC_EN	(1 << 31)
#endif

#pragma pack(push, 1)
typedef struct sg {
	u8 source_addr[5];
	u8 dest_addr[5];
	u8 pad[2];
	u32 source_len;
	u32 dest_len;
} sg;

struct other_task_descriptor {
	u32 task_id;
	u32 common_ctl;
	u32 symmetric_ctl;
	u32 asymmetric_ctl;
	u8 key_addr[5];
	u8 iv_addr[5];
	u8 ctr_addr[5];
	u8 pad;
	u32 data_len;
	struct sg sg[8];
	u8 next_sg_addr[5];
	u8 next_task_addr[5];
	u8 pad2[2];
	u32 reserve[3];
};

struct hash_task_descriptor {
	u32 ctrl;
	u32 cmd;
	u8 data_toal_len_addr[5];
	u8 hmac_prng_key_addr[5];
	u8 iv_addr[5];
	u8 pad;
	struct sg sg[8];
	u8 next_sg_addr[5];
	u8 next_task_addr[5];
	u8 pad2[2];
	u32 reserve[3];
};

#pragma pack(pop)

void sunxi_ss_open(void);
void sunxi_ss_close(void);
int sunxi_sha_calc(u8 *dst_addr, u32 dst_len, u8 *src_addr, u32 src_len);

s32 sunxi_rsa_calc(u8 *n_addr, u32 n_len, u8 *e_addr, u32 e_len, u8 *dst_addr,
		   u32 dst_len, u8 *src_addr, u32 src_len);
s32 sunxi_ecc_sign_check(struct ecc_verify_params *params, u32 key_byte_size,
			 u8 *hash, u32 hash_byte_len);
s32 sunxi_rsa_sign_check(u32 mod_bit_size, u8 *key_addr, u32 key_len,
			 u8 *src_addr, u32 src_len, u8 *dest_addr, u32 want_len,
			 u8 *mod_addr);
#endif /*  #ifndef _SS_H_  */
