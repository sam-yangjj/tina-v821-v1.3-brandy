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

#include <asm/assembler.h>

	.text
	.align	5

	.syntax unified
#if defined(SYS_THUMB_BUILD) && !defined(MEMSET_NO_THUMB_BUILD)
	.thumb
	.thumb_func
#endif

.globl memset0
memset0:
	ands	r3, r0, #3		@ 1 unaligned?
	mov	ip, r0			@ preserve r0 as return value
	bne	6f			@ 1
/*
 * we know that the pointer in ip is aligned to a word boundary.
 */
1:	orr	r1, r1, r1, lsl #8
	orr	r1, r1, r1, lsl #16
	mov	r3, r1
	cmp	r2, #16
	blt	4f

#if ! CALGN(1)+0

/*
 * We need 2 extra registers for this loop - use r8 and the LR
 */
	stmfd	sp!, {r8, lr}
	mov	r8, r1
	mov	lr, r1

2:	subs	r2, r2, #64
	stmiage	ip!, {r1, r3, r8, lr}	@ 64 bytes at a time.
	stmiage	ip!, {r1, r3, r8, lr}
	stmiage	ip!, {r1, r3, r8, lr}
	stmiage	ip!, {r1, r3, r8, lr}
	bgt	2b
	ldmfdeq	sp!, {r8, pc}		@ Now <64 bytes to go.
/*
 * No need to correct the count; we're only testing bits from now on
 */
	tst	r2, #32
	stmiane	ip!, {r1, r3, r8, lr}
	stmiane	ip!, {r1, r3, r8, lr}
	tst	r2, #16
	stmiane	ip!, {r1, r3, r8, lr}
	ldmfd	sp!, {r8, lr}

#else

/*
 * This version aligns the destination pointer in order to write
 * whole cache lines at once.
 */

	stmfd	sp!, {r4-r8, lr}
	mov	r4, r1
	mov	r5, r1
	mov	r6, r1
	mov	r7, r1
	mov	r8, r1
	mov	lr, r1

	cmp	r2, #96
	tstgt	ip, #31
	ble	3f

	and	r8, ip, #31
	rsb	r8, r8, #32
	sub	r2, r2, r8
	movs	r8, r8, lsl #(32 - 4)
	stmiacs	ip!, {r4, r5, r6, r7}
	stmiami	ip!, {r4, r5}
	tst	r8, #(1 << 30)
	mov	r8, r1
	strne	r1, [ip], #4

3:	subs	r2, r2, #64
	stmiage	ip!, {r1, r3-r8, lr}
	stmiage	ip!, {r1, r3-r8, lr}
	bgt	3b
	ldmfdeq	sp!, {r4-r8, pc}

	tst	r2, #32
	stmiane	ip!, {r1, r3-r8, lr}
	tst	r2, #16
	stmiane	ip!, {r4-r7}
	ldmfd	sp!, {r4-r8, lr}

#endif

4:	tst	r2, #8
	stmiane	ip!, {r1, r3}
	tst	r2, #4
	strne	r1, [ip], #4
/*
 * When we get here, we've got less than 4 bytes to zero.  We
 * may have an unaligned pointer as well.
 */
5:	tst	r2, #2
	strbne	r1, [ip], #1
	strbne	r1, [ip], #1
	tst	r2, #1
	strbne	r1, [ip], #1
	bx	lr

6:	subs	r2, r2, #4		@ 1 do we have enough
	blt	5b			@ 1 bytes to align with?
	cmp	r3, #2			@ 1
	strblt	r1, [ip], #1		@ 1
	strble	r1, [ip], #1		@ 1
	strb	r1, [ip], #1		@ 1
	add	r2, r2, r3		@ 1 (r2 = r2 - (4 - r3))
	b	1b
