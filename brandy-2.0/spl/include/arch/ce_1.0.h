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


#define SS_N_BASE       SUNXI_CE_BASE              //non security
#define SS_S_BASE       (SUNXI_CE_BASE+0x800)      //security

#define SS_TDQ          (SS_N_BASE + 0x00 + 0x800*ss_base_mode)
#define SS_ICR          (SS_N_BASE + 0x08 + 0x800*ss_base_mode)
#define SS_ISR          (SS_N_BASE + 0x0C + 0x800*ss_base_mode)
#define SS_TLR          (SS_N_BASE + 0x10 + 0x800*ss_base_mode)
#define SS_TSR          (SS_N_BASE + 0x14 + 0x800*ss_base_mode)
#define SS_ERR          (SS_N_BASE + 0x18 + 0x800*ss_base_mode)
#define SS_TPR          (SS_N_BASE + 0x1C + 0x800*ss_base_mode)
//#define SS_VER          (SS_N_BASE + 0x90 + 0x800*ss_base_mode)
#define SS_VER          (SS_N_BASE + 0x90)


//security
#define SS_S_TDQ        (SS_S_BASE + 0x00)
#define SS_S_CTR        (SS_S_BASE + 0x04)
#define SS_S_ICR        (SS_S_BASE + 0x08)
#define SS_S_ISR        (SS_S_BASE + 0x0C)
#define SS_S_TLR        (SS_S_BASE + 0x10)
#define SS_S_TSR        (SS_S_BASE + 0x14)
#define SS_S_ERR        (SS_S_BASE + 0x18)
#define SS_S_TPR        (SS_S_BASE + 0x1C)

//non security
#define SS_N_TDQ        (SS_N_BASE + 0x00)
#define SS_N_ICR        (SS_N_BASE + 0x08)
#define SS_N_ISR        (SS_N_BASE + 0x0C)
#define SS_N_TLR        (SS_N_BASE + 0x10)
#define SS_N_TSR        (SS_N_BASE + 0x14)
#define SS_N_ERR        (SS_N_BASE + 0x18)
#define SS_N_TPR        (SS_N_BASE + 0x1C)


#define SHA1_160_MODE	0
#define SHA2_256_MODE	1

#define ALG_SHA256 (0x13)
#define ALG_RSA    (0x20)
#define ALG_TRNG   (0x30)
#define TRNG_BYTE_LEN (32)

typedef struct sg
{
   uint addr;
   uint length;
}sg;

typedef struct descriptor_queue
{
	uint task_id;
	uint common_ctl;
	uint symmetric_ctl;
	uint asymmetric_ctl;
	uint key_descriptor;
	uint iv_descriptor;
	uint ctr_descriptor;
	uint data_len;
	sg   source[8];
	sg   destination[8];
	uint next_descriptor;
	uint reserved[3];
}task_queue;

#if defined(SHA256_MULTISTEP_PACKAGE) || defined(SHA512_MULTISTEP_PACKAGE)
int sunxi_hash_init(u8 *dst_addr, u8 *src_addr, u32 src_len, u32 total_len);
int sunxi_hash_update(u8 *dst_addr, u8 *src_addr, u32 src_len, u32 total_len);
int sunxi_hash_final(u8 *dst_addr, u8 *src_addr, u32 src_len, u32 total_len);
#endif

#endif    /*  #ifndef _SS_H_  */
