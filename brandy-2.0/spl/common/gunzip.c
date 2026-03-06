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
#include <u-boot/zlib.h>

#define ZLIB_HEADER0 '\x1f'
#define ZLIB_HEADER1 '\x8b'

#define ZLIB_ZALLOC_ALIGNMENT 16
#define ZLIB_HEAD_CRC	      2
#define ZLIB_EXTRA_FIELD      4
#define ZLIB_ORIG_NAME	      8
#define ZLIB_COMMENT	      0x10
#define ZLIB_RESERVED	      0xe0
#define ZLIB_DEFLATED	      8

static void *gzip_malloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZLIB_ZALLOC_ALIGNMENT - 1) &
	       ~(ZLIB_ZALLOC_ALIGNMENT - 1);

	p = malloc(size);

	return p;
}

static void gzip_free(void *x, void *addr, unsigned nb)
{
	free(addr);
}

static int gzip_parse_header(const unsigned char *src, unsigned long len)
{
	int i, flags;

	/* skip header */
	i     = 10;
	flags = src[3];
	if (src[2] != ZLIB_DEFLATED || (flags & ZLIB_RESERVED) != 0) {
		printf("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & ZLIB_EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ZLIB_ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & ZLIB_COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & ZLIB_HEAD_CRC) != 0)
		i += 2;
	if (i >= len) {
		printf("Error: gunzip out of data in header\n");
		return (-1);
	}
	return i;
}

static int zunzip(void *dst, int dst_len, unsigned char *src,
		  unsigned long *lenp, int stop_on_err, int offset)
{
	z_stream s;
	int err = 0;
	int r;

	s.zalloc = gzip_malloc;
	s.zfree	 = gzip_free;

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("Error: inflateInit2() fail, returned %d\n", r);
		return -1;
	}
	s.next_in   = src + offset;
	s.avail_in  = *lenp - offset;
	s.next_out  = dst;
	s.avail_out = dst_len;
	do {
		r = inflate(&s, Z_FINISH);
		if (stop_on_err == 1 && r != Z_STREAM_END &&
		    (s.avail_in == 0 || s.avail_out == 0 || r != Z_BUF_ERROR)) {
			printf("Error: inflate() fail, returned %d\n", r);
			err = -1;
			break;
		}
	} while (r == Z_BUF_ERROR);
	*lenp = s.next_out - (unsigned char *)dst;
	inflateEnd(&s);

	return err;
}

int gunzip(void *dst, int dst_len, unsigned char *src, unsigned long *lenp)
{
	int offset = gzip_parse_header(src, *lenp);

	if (offset < 0)
		return offset;

	return zunzip(dst, dst_len, src, lenp, 1, offset);
}