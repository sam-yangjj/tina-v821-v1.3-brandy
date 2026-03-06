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
#ifndef __SUNXI_BASE_IO_H
#define __SUNXI_BASE_IO_H

#include <asm/barriers.h>
#include <asm/byteorder.h>
#define GET_LO32(a)			((unsigned long)(a) & 0xffffffff)
#define __iormb() dmb()
#define __iowmb() dmb()

#define __arch_putl(v, a) (*(volatile unsigned int *)(a) = (v))
#define __arch_getl(a) (*(volatile unsigned int *)(a))

#define __arch_getb(a) (*(volatile unsigned char *)(a))
#define __arch_putb(v, a) (*(volatile unsigned char *)(a) = (v))

#define __raw_writel(v, a) __arch_putl(v, a)
#define __raw_readl(a) __arch_getl(a)

#define writeb(v, c)                                                           \
	({                                                                     \
		u8 __v = v;                                                    \
		__iowmb();                                                     \
		__arch_putb(__v, c);                                           \
		__v;                                                           \
	})
#define readb(c)                                                               \
	({                                                                     \
		u8 __v = __arch_getb(c);                                       \
		__iormb();                                                     \
		__v;                                                           \
	})
#define writel(v, c)                                                           \
	({                                                                     \
		u32 __v = v;                                                   \
		__iowmb();                                                     \
		__arch_putl(__v, c);                                           \
		__v;                                                           \
	})
#define readl(c)                                                               \
	({                                                                     \
		u32 __v = __arch_getl(c);                                      \
		__iormb();                                                     \
		__v;                                                           \
	})




#define out_arch(type, endian, a, v) __raw_write##type(v, a)
#define in_arch(type, endian, a) __raw_read##type(a)


#define out_le32(a, v) out_arch(l, le32, a, v)
#define in_le32(a) in_arch(l, le32, a)

#define clrbits(type, addr, clear)                                             \
	out_##type((addr), in_##type(addr) & ~(clear))

#define setbits(type, addr, set) out_##type((addr), in_##type(addr) | (set))

#define clrsetbits(type, addr, clear, set)                                     \
	out_##type((addr), (in_##type(addr) & ~(clear)) | (set))
#define clrbits_le32(addr, clear) clrbits(le32, addr, clear)
#define setbits_le32(addr, set) setbits(le32, addr, set)
#define clrsetbits_le32(addr, clear, set) clrsetbits(le32, addr, clear, set)

#endif
