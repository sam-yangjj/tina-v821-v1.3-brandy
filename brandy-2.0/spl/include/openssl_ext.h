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
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2014, Allwinner Technology Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#ifndef _SUNXI_OPENSSL_EXT_H__
#define _SUNXI_OPENSSL_EXT_H__
#include <x509.h>
#include <bn.h>
#include <aw_cert.h>
typedef struct {
	int head;
	int head_len;
	u8 *data;
	int data_len;
} sunxi_asn1_t;
int sunxi_certif_create(X509 **certif, u8 *buf, int len);
int sunxi_certif_free(X509 *certif);
int sunxi_certif_probe_serial_num(X509 *x);
int sunxi_certif_probe_version(X509 *x);
int sunxi_certif_probe_extension(X509 *x, sunxi_certif_info_t *sunxi_certif);
int sunxi_certif_probe_pubkey(X509 *x, sunxi_key_t *pubkey);
int sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len);
int sunxi_certif_probe_ext(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len);
int sunxi_X509_certif_verify_itself(sunxi_certif_info_t *sunxi_certif, u8 *buf,
				u32 len);
int OBJ_obj2name(char *dst_buf, int buf_len, const ASN1_OBJECT *a);
int ASN1_STRING_mem(char *bp, const ASN1_STRING *v);
void sunxi_certif_mem_reset(void);
void reset_BIO_reset(void);
void reset_OBJ_nid2ln_reset(void);
void reset_CRYPTO_reset(void);
void reset_D2I_reset(void);
#endif
