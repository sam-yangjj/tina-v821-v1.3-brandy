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

#include <common.h>

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
	(((long)X & (sizeof(long) - 1)) | ((long)Y & (sizeof(long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof(long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

void * memcpy( void *dst0, const void* src0, size_t len0)
{
	char *dst = dst0;
	const char *src = src0;
	long *aligned_dst;
	long *aligned_src;

	/* If the size is small, or either SRC or DST is unaligned,
	   then punt into the byte copy loop.  This should be rare.  */
	if (!TOO_SMALL(len0) && !UNALIGNED(src, dst)) {
		aligned_dst = (long *)dst;
		aligned_src = (long *)src;

		/* Copy 4X long words at a time if possible.  */
		while (len0 >= BIGBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len0 -= BIGBLOCKSIZE;
		}

		/* Copy one long word at a time if possible.  */
		while (len0 >= LITTLEBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			len0 -= LITTLEBLOCKSIZE;
		}

		/* Pick up any residual with a byte copier.  */
		dst = (char *)aligned_dst;
		src = (char *)aligned_src;
	}

	while (len0--)
		*dst++ = *src++;

	return dst0;
}

/* src/dst 8byte align */
typedef u64 local_t;
/*
 * copy 49152 bytes;
 * local_t bytes      time(us)
 * 16                 1929
 * 32                 973
 * 64                 770
 * 128                1069
 */
#define LARGEBLOCKSIZE (sizeof(local_t) * 4)
void *memcpy_large(void *dst0, const void *src0, size_t len0)
{
	local_t *aligned_dst;
	local_t *aligned_src;

	aligned_dst = (local_t *)dst0;
	aligned_src = (local_t *)src0;

	while (len0 >= LARGEBLOCKSIZE) {
		*aligned_dst++ = *aligned_src++;
		*aligned_dst++ = *aligned_src++;
		*aligned_dst++ = *aligned_src++;
		*aligned_dst++ = *aligned_src++;
		len0 -= LARGEBLOCKSIZE;
	}

	while (len0 > 0) {
		*aligned_dst++ = *aligned_src++;
		len0 -= sizeof(local_t);
	}

	return dst0;
}
