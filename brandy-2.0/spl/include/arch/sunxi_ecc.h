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
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _SUNXI_ECC_H_
#define _SUNXI_ECC_H_

#include <linux/types.h>

/**
 * The Hamming (72,64) code is an Error-Correction Code (ECC) mechanism, which
 * can be used to correct 1-bit error and detect 2-bit unrecoverable errors.
 *   - The size of the original/decoded symbol data is 64-bit.
 *   - The size of the encoded symbol data is 72-bit, which includes:
 *       - The first 8-bit parity data.
 *       - The last 64-bit original symbol data.
 */
#define HAMMING7264_DEC_SYMBOL_SIZE 8 /* 64-bit */
#define HAMMING7264_PARITY_SIZE	    1 /*  8-bit */
#define HAMMING7264_ENC_SYMBOL_SIZE 9 /* 72-bit == 8-bit + 64-bit*/

/**
 * @brief Calculate the parity data of the original symbol data using the
 *        SEC-DEC (72,64) encoder
 * @param[in] dec_sym Original symbol data buffer.
 *                    Size MUST be HAMMING7264_DEC_SYMBOL_SIZE byte.
 * @param[out] enc_parity Encoded parity data buffer.
 *                        Size MUST be HAMMING7264_PARITY_SIZE byte.
 * @return None
 */
void hamming7264_encode_parity(const uint8_t *dec_sym, uint8_t *enc_parity);

/**
 * @brief Encode the original symbol data using the SEC-DEC (72,64) encoder
 * @param[in] dec_sym Original symbol data buffer.
 *                    Size MUST be HAMMING7264_DEC_SYMBOL_SIZE byte.
 * @param[out] enc_sym Encoded symbol data buffer.
 *                     Size MUST be HAMMING7264_ENC_SYMBOL_SIZE byte.
 * @return None
 */
void hamming7264_encode(const uint8_t *dec_sym, uint8_t *enc_sym);

/**
 * @brief Decode the encoded symbol using the SEC-DEC (72,64) decoder
 * @param[in] enc_sym Encoded symbol data buffer.
 *                    Size MUST be HAMMING7264_ENC_SYMBOL_SIZE byte.
 * @param[out] dec_sym Decoded symbol data buffer.
 *                     Size MUST be HAMMING7264_DEC_SYMBOL_SIZE byte.
 * @param[out] dec_parity Decoded parity data buffer.
 *                        Size MUST be HAMMING7264_PARITY_SIZE byte.
 * @return Decoding status.
 *            0 for no error detected.
 *            1 for one error detected and corrected.
 *           -2 for two unrecoverable errors detected.
 */
int hamming7264_decode(uint8_t *enc_sym, uint8_t *dec_sym, uint8_t *dec_parity);


int sunxi_ecc_correct(u32 *efuse_data, int size);

#endif /* _SUNXI_ECC_H_ */
