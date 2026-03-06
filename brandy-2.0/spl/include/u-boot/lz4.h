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
#ifndef LZ4_H
#define LZ4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/types.h>

//#define LZ4_PART_DECOMPRESS_DEBUG

#if defined(LZ4_DECOMPRESS_DEBUG) || defined(LZ4_PART_DECOMPRESS_DEBUG)
#define LZ4_DEBUG_LOG(fmt, ...) printf("LZ4 [%s:%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define LZ4_DEBUG_LOG(fmt, ...)
#endif

/* lz4 wrapper */
#define LZ4_PART_INIT	   (1)
#define LZ4_PART_BLOCK	   (2)
#define LZ4_PART_BLOCK_CPY (3)
#define LZ4_PART_DECOMP	   (4)
#define LZ4_PART_FINISH	   (10)

/* lz4 internal */
#define LZ4_PART_PARSE_TOKEN (5)
#define LZ4_PART_PARSE_MATCH (6)

/* struct LZ4_block_part_dec_context */
struct LZ4_part_decompress_ctrl {
	/* argument */
	const uint8_t *source;
	uint8_t *dest;
	int inputSize;
	int outputSize;
	int endOnInput;
	int partialDecoding;
	int targetOutputSize;
	int dict;
	uint8_t *lowPrefix;
	uint8_t *dictStart;
	size_t dictSize; /* note : = 0 if noDict */
	/* internal */
	unsigned char state;
	unsigned char token;

	const uint8_t *ip;
	uint8_t *op;
	size_t in_pos;

	uint8_t *oexit;
	int safeDecode;
	int checkOffset;
};

typedef struct lz4_part_dec_context {
	unsigned char state;
	unsigned char has_block_checksum;

	const void *src;
	size_t src_data_total_size;
	size_t input_data_size;

	void *dst;
	size_t dst_buf_size;
	size_t *decompressed_size;

	const void *in;
	void *out;
	size_t lz4_block_size;

	struct LZ4_part_decompress_ctrl lz4_block_ctx;
} lz4_part_dec_context_t;
/* lib/lz4_wrapper.c */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

void lz4_part_dec_init(lz4_part_dec_context_t *ctx, const void *src, size_t srcn, void *dst, size_t *dstn);
int lz4_part_decompress(lz4_part_dec_context_t *ctx, size_t size);
uint32_t lz4_part_dec_get_decompressed_size(lz4_part_dec_context_t *ctx);
int is_lz4_part_dec_finish(lz4_part_dec_context_t *ctx);

#ifdef LZ4_PART_DECOMPRESS_DEBUG
void dump_lz4_part_dec_context(const lz4_part_dec_context_t *head);
void dump_lz4_part_dec_full_context(const lz4_part_dec_context_t *ctx);
void dump_lz4_block_part_dec_context(const struct LZ4_part_decompress_ctrl *ctx);
void dump_lz4_block_part_dec_full_context(const struct LZ4_part_decompress_ctrl *ctx);
#else
static inline void dump_lz4_part_dec_context(const lz4_part_dec_context_t *ctx)
{

}

static inline void dump_lz4_part_dec_full_context(const lz4_part_dec_context_t *ctx)
{

}

static inline void dump_lz4_block_part_dec_context(const struct LZ4_part_decompress_ctrl *ctx)
{

}

static inline void dump_lz4_block_part_dec_full_context(const struct LZ4_part_decompress_ctrl *ctx)
{

}
#endif

#ifdef __cplusplus
}
#endif

#endif /* LZ4_H */
