// SPDX-License-Identifier: GPL 2.0+ OR BSD-3-Clause
/*
 * Copyright 2015 Google Inc.
 */

#include <common.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <u-boot/lz4.h>

static u16 LZ4_readLE16(const void *src)
{
	return le16_to_cpu(*(u16 *)src);
}
static void LZ4_copy4(void *dst, const void *src)
{
	*(u32 *)dst = *(u32 *)src;
}
//static void LZ4_copy8(void *dst, const void *src) { *(u64 *)dst = *(u64 *)src; }

typedef uint8_t BYTE;
typedef uint16_t U16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;

#define FORCE_INLINE static inline __attribute__((always_inline))

/* Unaltered (except removing unrelated code) from github.com/Cyan4973/lz4. */
#include "lz4.c" /* #include for inlining, do not link! */

#define LZ4F_MAGIC 0x184D2204

struct lz4_frame_header {
	u32 magic;
	union {
		u8 flags;
		struct {
			u8 reserved0 : 2;
			u8 has_content_checksum : 1;
			u8 has_content_size : 1;
			u8 has_block_checksum : 1;
			u8 independent_blocks : 1;
			u8 version : 2;
		};
	};
	union {
		u8 block_descriptor;
		struct {
			u8 reserved1 : 4;
			u8 max_block_size : 3;
			u8 reserved2 : 1;
		};
	};
	/* + u64 content_size iff has_content_size is set */
	/* + u8 header_checksum */
} __packed;

struct lz4_block_header {
	union {
		u32 raw;
		struct {
			u32 size : 31;
			u32 not_compressed : 1;
		};
	};
	/* + size bytes of data */
	/* + u32 block_checksum iff has_block_checksum is set */
} __packed;

int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn)
{
	const void *end = dst + *dstn;
	const void *in  = src;
	void *out       = dst;
	int has_block_checksum;
	int ret;
	*dstn = 0;

	{ /* With in-place decompression the header may become invalid later. */
		const struct lz4_frame_header *h = in;

		if (srcn < sizeof(*h) + sizeof(u64) + sizeof(u8))
			return -EINVAL; /* input overrun */

		/* We assume there's always only a single, standard frame. */
		if (le32_to_cpu(h->magic) != LZ4F_MAGIC || h->version != 1)
			return -EPROTONOSUPPORT; /* unknown format */
		if (h->reserved0 || h->reserved1 || h->reserved2)
			return -EINVAL; /* reserved must be zero */
		if (!h->independent_blocks)
			return -EPROTONOSUPPORT; /* we can't support this yet */
		has_block_checksum = h->has_block_checksum;

		in += sizeof(*h);
		if (h->has_content_size)
			in += sizeof(u64);
		in += sizeof(u8);
	}
	while (1) {
		struct lz4_block_header b;

		b.raw = le32_to_cpu(*(u32 *)in);
		in += sizeof(struct lz4_block_header);

		if (in - src + b.size > srcn) {
			ret = -EINVAL; /* input overrun */
			break;
		}

		if (!b.size) {
			ret = 0; /* decompression successful */
			break;
		}
		printf("lz4 block size: %d\n", b.size);

		if (b.not_compressed) {
			size_t size = min((ptrdiff_t)b.size, end - out);
			memcpy(out, in, size);
			out += size;
			if (size < b.size) {
				ret = -ENOBUFS; /* output overrun */
				break;
			}
		} else {
			/* constant folding essential, do not touch params! */
			ret = LZ4_decompress_generic(in, out, b.size, end - out,
						     endOnInputSize, full, 0,
						     noDict, out, NULL, 0);
			if (ret < 0) {
				printf("Error: LZ4_decompress_generic returned %d\n",
				       ret);
				ret = -EPROTO; /* decompression error */
				break;
			}
			out += ret;
		}

		in += b.size;
		if (has_block_checksum)
			in += sizeof(u32);
	}

	*dstn = out - dst;
	return ret;
}

#ifdef LZ4_PART_DECOMPRESS_DEBUG
void dump_lz4_part_dec_context(const lz4_part_dec_context_t *ctx)
{
	LZ4_DEBUG_LOG("state: %u", ctx->state);
	LZ4_DEBUG_LOG("input_data_size: %u", ctx->input_data_size);
	LZ4_DEBUG_LOG("handled compressed data size: %u", ctx->in - ctx->src);

	LZ4_DEBUG_LOG("in: %p", ctx->in);
	LZ4_DEBUG_LOG("out: %p", ctx->out);
	LZ4_DEBUG_LOG("lz4_block_size: %u", ctx->lz4_block_size);
}

void dump_lz4_part_dec_full_context(const lz4_part_dec_context_t *ctx)
{
	LZ4_DEBUG_LOG("state: %u", ctx->state);
	LZ4_DEBUG_LOG("has_block_checksum: %u", ctx->has_block_checksum);

	LZ4_DEBUG_LOG("src: %p", ctx->src);
	LZ4_DEBUG_LOG("src_data_total_size: %u", ctx->src_data_total_size);
	LZ4_DEBUG_LOG("input_data_size: %u", ctx->input_data_size);

	LZ4_DEBUG_LOG("dst: %p", ctx->dst);
	LZ4_DEBUG_LOG("dst_buf_size: %u", ctx->dst_buf_size);
	LZ4_DEBUG_LOG("decompressed_size pointer: %p", ctx->decompressed_size);

	LZ4_DEBUG_LOG("in: %p", ctx->in);
	LZ4_DEBUG_LOG("out: %p", ctx->out);
	LZ4_DEBUG_LOG("lz4_block_size: %u", ctx->lz4_block_size);
}

void dump_lz4_block_part_dec_context(const struct LZ4_part_decompress_ctrl *ctx)
{
	LZ4_DEBUG_LOG("state: %u", ctx->state);
	LZ4_DEBUG_LOG("in_pos: %u", ctx->in_pos);

	LZ4_DEBUG_LOG("ip: %p", ctx->ip);
	LZ4_DEBUG_LOG("op: %p", ctx->op);
	LZ4_DEBUG_LOG("token: %u", ctx->token);
}

void dump_lz4_block_part_dec_full_context(const struct LZ4_part_decompress_ctrl *ctx)
{
	LZ4_DEBUG_LOG("state: %u", ctx->state);
	LZ4_DEBUG_LOG("in_pos: %u", ctx->in_pos);

	LZ4_DEBUG_LOG("ip: %p", ctx->ip);
	LZ4_DEBUG_LOG("op: %p", ctx->op);
	LZ4_DEBUG_LOG("token: %u", ctx->token);

	LZ4_DEBUG_LOG("oexit: %p", ctx->oexit);

	LZ4_DEBUG_LOG("safeDecode: %d", ctx->safeDecode);
	LZ4_DEBUG_LOG("checkOffset: %d", ctx->checkOffset);
}
#endif

void lz4_part_dec_init(lz4_part_dec_context_t *ctx, const void *src, size_t srcn, void *dst, size_t *dstn)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state = LZ4_PART_INIT;
	ctx->dst = dst;
	ctx->decompressed_size = dstn;
	ctx->dst_buf_size = *dstn;
	ctx->src = src;
	ctx->src_data_total_size = srcn;

	ctx->in = src;
	ctx->out = dst;

	dump_lz4_part_dec_full_context(ctx);
}

enum {
	LZ4_RET_OK,
	LZ4_RET_INVALID_FRAME_SIZE = -1000,
	LZ4_RET_INVALID_MAGIC,
	LZ4_RET_INVALID_VERSION,
	LZ4_RET_RESERVED_FIELD_NOT_ZERO,
	LZ4_RET_UNSUPPORTED_BLOCK_FORMAT,
	LZ4_RET_INCORRECT_BLOCK_SIZE_OR_FRAME_SIZE,
	LZ4_RET_OUT_BUF_SIZE_NOT_ENOUGH,
	LZ4_RET_BLK_PART_DEC_INIT_FAILED,
	LZ4_RET_BLK_PART_DEC_FAILED,
	LZ4_RET_INVALID_CONTEXT_STATE,
};

int lz4_part_decompress(lz4_part_dec_context_t *head, size_t size)
{
	int ret;

	LZ4_DEBUG_LOG("input compressed data size: %u", size);
	head->input_data_size += size;

again:
	LZ4_DEBUG_LOG("enter LZ4 part dec state machine! dump context begin");
	dump_lz4_part_dec_context(head);
	LZ4_DEBUG_LOG("enter LZ4 part dec state machine! dump context end");

	if (unlikely(head->state == LZ4_PART_FINISH))
		return 0;
	else if (unlikely(head->in == (head->src + head->input_data_size)))
		return 0; /* input data is not enough */
	else if (unlikely(head->state == LZ4_PART_INIT)) { /* With in-place decompression the header may become invalid later. */
		const struct lz4_frame_header *h = head->in;

		if (head->src_data_total_size < sizeof(*h) + sizeof(u64) + sizeof(u8))
			return LZ4_RET_INVALID_FRAME_SIZE; /* the size of frame is too small */

		/* We assume there's always only a single, standard frame. */
		if (le32_to_cpu(h->magic) != LZ4F_MAGIC)
			return LZ4_RET_INVALID_MAGIC; /* unknown format */
		if (h->version != 1)
			return LZ4_RET_INVALID_VERSION; /* unknown format */
		if (h->reserved0 || h->reserved1 || h->reserved2)
			return LZ4_RET_RESERVED_FIELD_NOT_ZERO; /* reserved must be zero */
		if (!h->independent_blocks)
			return LZ4_RET_UNSUPPORTED_BLOCK_FORMAT; /* we can't support this yet */

		head->has_block_checksum = h->has_block_checksum;

		head->in += sizeof(*h);
		size -= sizeof(*h);
		if (h->has_content_size) {
			head->in += sizeof(u64);
			size -= sizeof(u64);
		}
		head->in += sizeof(u8);
		size -= sizeof(u8);
		head->state = LZ4_PART_BLOCK;
		goto again;
	} else if (head->state == LZ4_PART_BLOCK) {
		struct lz4_block_header b;

		if ((head->in + sizeof(b)) > (head->src + head->input_data_size))
			return 0;	/* no more input */

		b.raw = le32_to_cpu(*(u32 *)(head->in));
		head->in += sizeof(struct lz4_block_header);
		size -= sizeof(struct lz4_block_header);

		if (head->in - head->src + b.size > head->src_data_total_size) {
			return LZ4_RET_INCORRECT_BLOCK_SIZE_OR_FRAME_SIZE; /* block size or frame size is incorrect */
		}

		if (!b.size) {
			head->state = LZ4_PART_FINISH; /* decompression successful */
			*head->decompressed_size = head->out - head->dst;
			return 0;
		}
		head->lz4_block_size = b.size;

		if (unlikely(b.not_compressed)) {
			size_t size = min((ptrdiff_t)b.size, (head->dst + head->dst_buf_size) - head->out);
			if (size < b.size)
				return LZ4_RET_OUT_BUF_SIZE_NOT_ENOUGH; /* output overrun */
			head->state = LZ4_PART_BLOCK_CPY;
			goto again;
		} else {
			head->state = LZ4_PART_DECOMP;

			LZ4_DEBUG_LOG("before LZ4_part_decompress_init");
			dump_lz4_block_part_dec_full_context(&head->lz4_block_ctx);

			ret = LZ4_part_decompress_init(&head->lz4_block_ctx, head->in, head->out, b.size,
							(head->dst + head->dst_buf_size) - head->out,
							endOnInputSize, full, 0, noDict, head->out, NULL, 0);

			LZ4_DEBUG_LOG("after LZ4_part_decompress_init, ret: %d", ret);
			dump_lz4_block_part_dec_full_context(&head->lz4_block_ctx);

			if (ret < 0) {
				printf("Error: LZ4_part_decompress_init returned %d\n", ret);
				return LZ4_RET_BLK_PART_DEC_INIT_FAILED; /* decompression init error */
			}
			goto again;
		}
	} else if (likely(head->state == LZ4_PART_DECOMP)) {
		LZ4_DEBUG_LOG("before LZ4_part_decompress_seq, input compressed data size: %u", size);
		dump_lz4_block_part_dec_context(&head->lz4_block_ctx);

		ret = LZ4_part_decompress_seq(&head->lz4_block_ctx, size);

		LZ4_DEBUG_LOG("after LZ4_part_decompress_seq, ret: %d", ret);
		dump_lz4_block_part_dec_context(&head->lz4_block_ctx);

		if (ret > 0) {
			uint32_t handled_data_size;

			head->out += ret;
			handled_data_size = LZ4_part_decompress_get_processed_cnt(&head->lz4_block_ctx);
			head->in += handled_data_size;

			LZ4_DEBUG_LOG("after LZ4_part_decompress_get_processed_cnt, ret: %d", handled_data_size);

			if (head->has_block_checksum)
				head->in += sizeof(u32);

			head->state = LZ4_PART_BLOCK;
			head->lz4_block_size = 0;
			/* reinit size var */
			size = head->input_data_size - (uint32_t)(head->in - head->src);
			goto again;
		} else if (ret == 0) {
			return 0; /* no more input */
		} else {
			printf("Error: LZ4_decompress_generic returned %d\n", ret);
			return LZ4_RET_BLK_PART_DEC_FAILED; /* LZ4 block decompression error */
		}
	} else if (unlikely(head->state == LZ4_PART_BLOCK_CPY)) {
		size_t cpy_size = min(head->lz4_block_size, head->input_data_size - (size_t)(head->in - head->src));
		memcpy(head->out, head->in, cpy_size);
		head->out += cpy_size;
		head->in  += cpy_size;
		head->lz4_block_size -= cpy_size;
		if (head->lz4_block_size == 0) {
			if (head->has_block_checksum)
				head->in += sizeof(u32);

			/* reinit size var */
			size = head->input_data_size - (uint32_t)(head->in - head->src);
			head->state = LZ4_PART_BLOCK;
			head->lz4_block_size = 0;
		}
		goto again;
	}

	return LZ4_RET_INVALID_CONTEXT_STATE;
}

uint32_t lz4_part_dec_get_decompressed_size(lz4_part_dec_context_t *ctx)
{
	return (uint32_t)(ctx->out - ctx->dst);
}

int is_lz4_part_dec_finish(lz4_part_dec_context_t *ctx)
{
	return (ctx->state == LZ4_PART_FINISH);
}
