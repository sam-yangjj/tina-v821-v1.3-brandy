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
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#include <common.h>

#ifdef CFG_RECORD_BOOTTIME
uint32_t boottime_pos;
uint32_t boottime[CFG_RECORD_BOOTTIME_MAX];
const char *boottime_desc[CFG_RECORD_BOOTTIME_MAX];
#endif

void u8_to_string_hex( u8 input, char * str )
{
	int i;
	static char base[] = "0123456789abcdef";

	for( i = 1; i >= 0; --i )
	{
		str[i] = base[input&0xf];
		input >>= 4;
	}

	str[2] = '\0';

	return;
}

int snprintf(char *buf, size_t size, const char *fmt, ...);
void ndump(u8 *buf, int count)
{
	char line[50];
	char *tmp = line;
	char *in  = (char *)buf;
	int i;
	printf("inbuf:%x\n", (phys_addr_t)buf);
	for (i = 0; i < count; i++) {
		snprintf(tmp, 5, "%02x ", *((uint8_t *)in));
		tmp += 3;
		if (((i % 16) == 15)) {
			printf("%s\n", line);
			tmp = line;
		}
		in++;
	}
	if (((i % 16) != 0)) {
		printf("%s\n", line);
		tmp = line;
	}
}

void hexdump(const char *addr, unsigned int size, const char *name)
{
	int i, j;

	printf("Dump %s addr=0x%lx, size=0x%x:\n  ", name ? name : "", (unsigned long)addr, size);
	for (j = 0; j < size; j += 16) {
		for (i = 0; (i < 16) && (i < size); i++) {
			raw_printf("%02x ", addr[j + i] & 0xff);
		}
		raw_printf("\n  ");
	}
	raw_printf("\n");
}

void __assert_fail(const char *assertion, const char *file, unsigned line,
		   const char *function)
{
	/* This will not return */
	printf("%s:%u: %s: Assertion `%s' failed.", file, line, function,
	      assertion);
	while(1);
}

#if CFG_SUNXI_SIMULATE_BOOT0 /* help build boot0 for simulation */
#include <asm/io.h>
#define SYS_CFG_BASE (SUNXI_SYSCRL_BASE)
#define SYS_MSG (SYS_CFG_BASE + 0xB0)
#define DBUG_CTRL (SYS_CFG_BASE + 0xC0)
#define DBUG_DATA (SYS_CFG_BASE + 0xC4)

void write_debug_reg(u32 data)
{
	u32 temp = 0;
	writel(data, DBUG_DATA);
	temp = (1 << 31) | (2 << 28) | (SYS_MSG & 0x0fffffff);
	writel(temp, DBUG_CTRL);
}

void pattern_goto(u32 step)
{
	u32 rdata;
	rdata = (1 << 26) | (step & 0xFFFF);
	write_debug_reg(rdata);
	write_debug_reg(0);
}

void pattern_mod_goto(u32 mod, u32 step)
{
}

void pattern_end(u32 pass)
{
	if (pass == 1)
		pattern_goto(0xfffc);
	else
		pattern_goto(0xfffD);
}
#endif
