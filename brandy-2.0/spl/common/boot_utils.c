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
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <arch/uart.h>
#include <arch/dram.h>

#ifdef CFG_SUNXI_USE_NEON
/*stdint.h included by arm_neon.h has conflict on uintptr_t with linux/types.h, undef it here*/
#undef __UINTPTR_TYPE__
#include <arm_neon.h>

int verify_addsum(void *mem_base, u32 size)
{
	uint32x4_t sum_vec;
	uint32x4_t adding;
	u32 neon_sum = 0;
	u32 src_sum;
	u32 *buf  = (u32 *)mem_base;
	u32 count = size >> 2;
	sbrom_toc1_head_info_t *bfh;
	bfh = (sbrom_toc1_head_info_t *)mem_base;

	neon_enable();

	src_sum	     = bfh->add_sum;
	bfh->add_sum = STAMP_VALUE;

	sum_vec = vdupq_n_u32(0);
	while (count >= 4) {
		adding	= vld1q_u32(buf);
		sum_vec = vaddq_u32(sum_vec, adding);
		buf += 4;
		count -= 4;
	}
	neon_sum += vgetq_lane_u32(sum_vec, 0);
	neon_sum += vgetq_lane_u32(sum_vec, 1);
	neon_sum += vgetq_lane_u32(sum_vec, 2);
	neon_sum += vgetq_lane_u32(sum_vec, 3);

	while (count-- > 0)
		neon_sum += *buf++;

		//printf("sum=%x\n", neon_sum);
		//printf("src_sum=%x\n", src_sum);

	bfh->add_sum = src_sum;
	if (neon_sum == src_sum)
		return 0;
	else
		return -1;
	return 0;
}
#else
/* check: 0-success  -1:fail */
int verify_addsum(void *mem_base, u32 size)
{
	u32 *buf;
	u32 count;
	u32 src_sum;
	u32 sum;
	sbrom_toc1_head_info_t *bfh;

#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	bfh = (sbrom_toc1_head_info_t *)mem_base;
	/*generate checksum*/
	src_sum = bfh->add_sum;
	bfh->add_sum = STAMP_VALUE;
	count = size >> 2;
	sum = 0;
	buf = (u32 *)mem_base;
	do
	{
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;

	bfh->add_sum = src_sum;

#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif
//	printf("sum=%x\n", sum);
//	printf("src_sum=%x\n", src_sum);
	if( sum == src_sum )
		return 0;
	else
		return -1;
}
#endif

#ifdef CFG_KERNEL_CHECKSUM
/* check: 0-success  -1:fail */
int verify_addsum_for_kernel(void *mem_base, u32 size, u32 src_sum)
{
#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	u32 *buf;
	u32 count;
	u32 sum;

	/*generate checksum*/
	count = size >> 2;
	sum   = 0;
	buf   = (u32 *)mem_base;
	do {
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
	} while ((count -= 4) > (4 - 1));

	while (count-- > 0)
		sum += *buf++;

#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif
	if (sum == src_sum) {
		return 0;
	} else {
		printf("boot image checksum error sum=%x src_sum=%x\n", sum,
		       src_sum);
		return -1;
	}
}
#endif

u32 sunxi_generate_checksum(void *buffer, uint length, uint div, uint src_sum)
{
	uint *buf;
	int count;
	uint sum;

	count = length >> 2;
	sum   = 0;
	buf   = (__u32 *)buffer;
	do {
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
	} while ((count -= (4*div)) > (4 - 1));

	while (count-- > 0)
		sum += *buf++;

	sum = sum - src_sum + STAMP_VALUE;

	return sum;
}

int sunxi_verify_checksum(void *buffer, uint length, uint src_sum)
{
	uint sum;
	sum = sunxi_generate_checksum(buffer, length, 1, src_sum);

	boot_info("src sum=%x, check sum=%x\n", src_sum, sum);
	if (sum == src_sum)
		return 0;
	else
		return -1;
}

u32 g_mod( u32 dividend, u32 divisor, u32 *quot_p)
{
	if (divisor == 0) {
		*quot_p = 0;
		return 0;
	}
	if (divisor == 1) {
		*quot_p = dividend;
		return 0;
	}

	for (*quot_p = 0; dividend >= divisor; ++(*quot_p))
		dividend -= divisor;
	return dividend;
}

u8  *get_page_buf( void )
{

	return (u8 *)(CONFIG_SYS_DRAM_BASE + 1024 * 1024);
}

char get_uart_input(void)
{

	char c = 0;
	if (sunxi_serial_tstc()) {
		c = sunxi_serial_getc();
		printf("key press : %c\n", c);
		/* test time: 30 ms */
		/*
		if (get_sys_ticks() - start > 30)
			break;
		*/
	}
	return c;
}

static uint8_t uboot_func_mask;
void set_uboot_func_mask(uint8_t mask)
{
	uboot_func_mask |= mask;
}

uint8_t get_uboot_func_mask(uint8_t mask)
{
	return uboot_func_mask & mask;
}

__weak int sunxi_board_init(void)
{
	return 0;
}

__weak uint8_t sunxi_board_late_init(void)
{
	return 0;
}

__weak int sunxi_board_init_early(void)
{
	return 0;
}

__weak int sunxi_fes_board_init_early(void)
{
	return 0;
}

__weak uint32_t *sunxi_bootparam_get_dram_buf(void)
{
	return
#if CFG_SUNXI_SBOOT
				(uint32_t *)toc0_config->dram_para;
#elif CFG_SUNXI_FES
				(uint32_t *)fes1_head.prvt_head.dram_para;
#else
				(uint32_t *)BT0_head.prvt_head.dram_para;
#endif
}

__weak int sunxi_bootparam_load(void)
{
	return 0;
}

__weak int load_and_run_fastboot(u32 uboot_base, u32 optee_base, u32 monitor_base,
		u32 rtos_base, u32 opensbi_base, u32 cpus_rtos_base)
{
	return 0;
}

__weak void boot_riscv(phys_addr_t base, unsigned long fdt_addr)
{
}

__weak int init_DRAM(int type, dram_para_t *buff)
{
	printf("__weak %s...%d\n", __func__, __LINE__);
	return 0;
}
