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
#include <asm/io.h>

static u32 s_hosc_freq = 24;

void aw_set_hosc_freq(u32 hosc_freq)
{
	if (hosc_freq == 24 || hosc_freq == 40)
		s_hosc_freq = hosc_freq;
	else
		asm volatile("j .");
}

u32 aw_get_hosc_freq(void)
{
	return s_hosc_freq;
}
/*
 * 64bit arch timer.CNTPCT
 * Freq = s_hosc_freq
 */

static inline u64 get_arch_counter(void)
{
	u64 cnt = 0;
	unsigned int upper, lower;
	unsigned int upper_new;

	asm volatile(
			"1:  rdtimeh %[upper]\n"
			"    rdtime %[lower]\n"
			"    rdtimeh %[upper_new]\n"
			"    bne %[upper], %[upper_new], 1b\n"
			: [upper] "=r"(upper),
			[lower] "=r"(lower),
			[upper_new] "=&r"(upper_new)
			:
			: "memory"
		    );

	cnt = ((unsigned long long)upper << 32) | lower;

	return cnt;
}

/*
 * get current time.(millisecond)
 */
u32 get_sys_ticks(void)
{
	return (u32)get_arch_counter() / (s_hosc_freq * 1000);
}

/*
 * get current time.(microsecond)
 */
u32 timer_get_us(void)
{
	return (u32)get_arch_counter() / s_hosc_freq;
}

__weak void udelay(unsigned long us)
{
	u64 t1, t2;

	t1 = get_arch_counter();
	t2 = t1 + us * s_hosc_freq;
	do {
		t1 = get_arch_counter();
	} while (t2 >= t1);
}

__weak void mdelay(unsigned long ms)
{
	udelay(ms * 1000);
}

__weak void __usdelay(unsigned long us)
{
	udelay(us);
}

__weak void __msdelay(unsigned long ms)
{
	mdelay(ms);
}

__weak int timer_init(void)
{
	return 0;
}

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
 *************************************************************/
__weak void sdelay(unsigned long loops)
{
}

__weak void timer_exit(void)
{
}


