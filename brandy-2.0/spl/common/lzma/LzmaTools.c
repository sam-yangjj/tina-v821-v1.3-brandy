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

#include <config.h>
#include <common.h>

#ifdef CFG_SUNXI_LZMA

#define LZMA_PROPERTIES_OFFSET 0
#define LZMA_SIZE_OFFSET LZMA_PROPS_SIZE
#define LZMA_DATA_OFFSET LZMA_SIZE_OFFSET + sizeof(uint64_t)

#include "LzmaTools.h"
#include "LzmaDec.h"

#define debug printf

static void *SzAlloc(void *p, size_t size)
{
	return malloc(size);
}
static void SzFree(void *p, void *address)
{
	free(address);
}

int lzmaBuffToBuffDecompress(unsigned char *outStream, SizeT *uncompressedSize,
			     unsigned char *inStream, SizeT length)
{
	int res = SZ_ERROR_DATA;
	int i;
	ISzAlloc g_Alloc;

	SizeT outSizeFull = 0xFFFFFFFF; /* 4GBytes limit */
	SizeT outProcessed;
	SizeT outSize;
	SizeT outSizeHigh;
	ELzmaStatus state;
	SizeT compressedSize = (SizeT)(length - LZMA_PROPS_SIZE);

	debug("LZMA: Image address............... 0x%08lx\n", (unsigned long)inStream);
	debug("LZMA: Properties address.......... 0x%08lx\n",
	      (unsigned long)(inStream + LZMA_PROPERTIES_OFFSET));
	debug("LZMA: Uncompressed size address... 0x%08lx\n",
	      (unsigned long)(inStream + LZMA_SIZE_OFFSET));
	debug("LZMA: Compressed data address..... 0x%08lx\n",
	      (unsigned long)(inStream + LZMA_DATA_OFFSET));
	debug("LZMA: Destination address......... 0x%08lx\n", (unsigned long)outStream);

	memset(&state, 0, sizeof(state));

	outSize     = 0;
	outSizeHigh = 0;
	/* Read the uncompressed size */
	for (i = 0; i < 8; i++) {
		unsigned char b = inStream[LZMA_SIZE_OFFSET + i];
		if (i < 4) {
			outSize += (UInt32)(b) << (i * 8);
		} else {
			outSizeHigh += (UInt32)(b) << ((i - 4) * 8);
		}
	}

	outSizeFull = (SizeT)outSize;
	if (sizeof(SizeT) >= 8) {
		/*
		* SizeT is a 64 bit uint => We can manage files larger than 4GB!
		*
		*/
		outSizeFull |= (((SizeT)outSizeHigh << 16) << 16);
	} else if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize) {
		/*
		* SizeT is a 32 bit uint => We cannot manage files larger than
		* 4GB!  Assume however that all 0xf values is "unknown size" and
		* not actually a file of 2^64 bits.
		*
		*/
		if (outSizeHigh != (SizeT)-1 || outSize != (SizeT)-1) {
			debug("LZMA: 64bit support not enabled.\n");
			return SZ_ERROR_DATA;
		}
	}

	debug("LZMA: Uncompresed size............ 0x%08lx\n", outSizeFull);
	debug("LZMA: Compresed size.............. 0x%08lx\n", compressedSize);

	g_Alloc.Alloc = SzAlloc;
	g_Alloc.Free  = SzFree;

	/* Short-circuit early if we know the buffer can't hold the results. */
	if (outSizeFull != (SizeT)-1 && *uncompressedSize < outSizeFull)
		return SZ_ERROR_OUTPUT_EOF;

	/* Decompress */
	/* outProcessed = min(outSizeFull, *uncompressedSize); */
	if (outSizeFull > *uncompressedSize)
		outProcessed = *uncompressedSize;
	else
		outProcessed = outSizeFull;

	/* WATCHDOG_RESET(); */

	res = LzmaDecode(outStream, &outProcessed, inStream + LZMA_DATA_OFFSET,
			 &compressedSize, inStream, LZMA_PROPS_SIZE,
			 LZMA_FINISH_END, &state, &g_Alloc);
	*uncompressedSize = outProcessed;

	debug("LZMA: Uncompressed ............... 0x%08lx\n", outProcessed);

	if (res != SZ_OK) {
		return res;
	}

	return res;
}

#endif
