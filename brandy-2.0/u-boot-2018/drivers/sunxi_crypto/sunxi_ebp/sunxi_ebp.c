/*
 * SPDX-License-Identifier:     GPL-2.0+
 * (C) Copyright 2018
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * zhoujie <zhoujie@allwinnertech.com>
 */
#include <common.h>
#include <efuse_map.h>
#include <sunxi_board.h>
#include <asm/io.h>
#include <sunxi_ebp.h>

#define EBP_GET_BIT_VAL(buf, start_bit, bit_cnt, val)			\
({									\
	const int32_t __size = bit_cnt;					\
	const uint32_t __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
	const int32_t __off = ((start_bit) / 32);			\
	const int32_t __shft = (start_bit) & 31;			\
	uint32_t __res;							\
									\
	__res = buf[__off] >> __shft;					\
	if (__size + __shft > 32)					\
		__res |= buf[__off + 1] << ((32 - __shft) % 32);	\
	val = __res & __mask;							\
})

#define EBP_SET_BIT_VAL(buf, bit_pos)					\
({									\
	const uint32_t __off = ((bit_pos) / 32);			\
	const uint32_t __shft = (bit_pos) & 31;				\
									\
	buf[__off] |= (1 << __shft);					\
})

#define EBP_ID_CAL(ebpx, offset, val, val_backup)			\
({									\
	const uint32_t __off = (SID_EBP_ID_OFFSET(ebpx) % 32);		\
	offset = (SID_EBP_ID_OFFSET(ebpx) / 8);				\
	offset -= (offset & 0x3);					\
	if (__off < 21) {						\
		val = (val & SID_EBP_ID_MASK) << __off;			\
		val_backup = 0;						\
	} else {							\
		const uint32_t val_temp = val;				\
		val = ((val_temp & SID_EBP_ID_MASK) >> (32 - __off));	\
		val_backup = ((val_temp & SID_EBP_ID_MASK) & (1U << ((SID_EBP_IDX_MAX - (32 - __off)) - 1)));	\
	}								\
})

static u32 ebp_12bit_parity_check(u32 val)
{
	u32 p = val;

	p = (p & 0x3f) ^ ((p >> 6) & 0x3f); /* p is 6-bit width */
	p = (p & 0x7) ^ ((p >> 3) & 0x7);  /* p is 3-bit width */
	p = (p & 0x1) ^ ((p >> 1) & 0x1) ^ ((p >> 2) & 0x1); /* p is 1-bit width */

	return p;
}

/* @efuse_data: all efuse original data */
int sunxi_ebp_correct(u32 *efuse_data, int size)
{

	u32 idx = 0, bitmap = 0, val;

#ifdef SUNXI_EBP_TEST
	sunxi_ebp_record(0, 1);
#endif

	EBP_GET_BIT_VAL(efuse_data, SID_EBP_BITMAP_START, SID_EBP_BITMAP_WIDTH, bitmap);
	while (bitmap) {
		if (bitmap & SID_EBP_BITMAP_MASK) { /* EBP[i] is valid */
			EBP_GET_BIT_VAL(efuse_data, SID_EBP_START + SID_EBP_WIDTH * idx, SID_EBP_WIDTH, val);
			if (ebp_12bit_parity_check(val)) { /* parity check is ok */
				EBP_SET_BIT_VAL(efuse_data, val & SID_EBP_POST_MASK); /* correct the error bit to 1 */
			}
		}
		bitmap >>= 2;
		++idx;
	}

	return 0;
}

int ebp_write(u32 err_bit_pos)
{
	static u32 ebpx_num;
	u32 ebpx_val, odd_bit = 0, val, bitmap;
	u32 val1_wr = 0, val2_wr = 0, val1_rd = 0, val2_rd = 0, offset = 0;

	/* check ebp id reaches the maximum value */
	bitmap = readl(SID_EFUSE + SID_EBP_BITMAP_BASE);
	if ((ebpx_num >= SID_EBP_IDX_MAX) || ((bitmap & SID_EBP_BITMAPALL_MASK) == SID_EBP_BITMAPALL_MASK)) {
		printf("the ebp ood number is reach to max\n");
		return -1;
	}

	/* cal ebp odd bit val and write to efuse */
	odd_bit = ebp_12bit_parity_check(err_bit_pos);
	ebpx_val = err_bit_pos | (!odd_bit) << SID_EBP_ODD_BIT_SHIFT;

	/* step:
	 * 1. Write val to the location corresponding to the first empty EBPx
	 * 2. If 1 is incorrect, select the next EBPx to re-flash
	 * 3. If 1 is written correctly, the corresponding EBPx bitmap is flashed to 0x3
	 * 4. If the EBPx bitmap value written in 3 is 0, select the next EBPx to re-flash it
	 */

	val1_wr = ebpx_val;
	while (ebpx_num <= SID_EBP_IDX_MAX) {
		/* Determine whether there is a remainder in the EBPX offset position divided by 32,
		 * if the remainder is less than 21,
		 * it will be directly written to the current address;
		 * Otherwise, split it into two addresses
		 */
		EBP_ID_CAL(ebpx_num, offset, val1_wr, val2_wr);
		/* write ebp */
		val = readl(SID_EFUSE + offset);
		val |= val1_wr;
		sid_program_key(offset, val);
		if (val2_wr) {
			val = readl(SID_EFUSE + offset + 0x4);
			val |= val2_wr;
			sid_program_key(offset + 0x4, val);
		}

		/* check ebp write result */
		val1_rd = readl(SID_EFUSE + offset);
		if ((val1_rd & val1_wr) != val1_wr) {
			ebpx_num++;
			continue;
		}
		if (val2_wr) {
			val2_rd = readl(SID_EFUSE + offset + 0x4);
			if ((val2_rd & val2_wr) != val1_wr) {
				ebpx_num++;
				continue;
			}
		}

		/* write ebp bitmap */
		val = readl(SID_EBP_BITMAP_BASE);
		val |= SID_EBP_MAP_BIT(ebpx_num);
		sid_program_key(SID_EBP_BITMAP_BASE, val);
		/* check ebp bitmap result */
		bitmap = readl(SID_EFUSE + SID_EBP_BITMAP_BASE);
		if ((bitmap & SID_EBP_MAP_BIT(ebpx_num) >> (ebpx_num * 2)) == SID_EBP_BITMAP_MASK) {
			ebpx_num++;
			break;
		}
		ebpx_num++;
	}

	return 0;
}

int sunxi_ebp_record(u32 key_index, u32 err_val)
{
	u32 err_bit_pos;
	int i, ret = 0;

	if (key_index >= SID_EBP_BITMAP_BASE)
		return -1;
	for (i = 0; i < 32; i++) {
		if (err_val & BIT(i)) {
			/* Calculate the offset position (unit bit) where the error bit is located */
			err_bit_pos = key_index * 8 + i;
			/* Write the offset position to the EBP */
			ret = ebp_write(err_bit_pos);
			if (ret) {
				printf("the ebp odd num has already to max:%d\n", SID_EBP_SIZE);
				return -1;
			}
		}
	}
	return 0;
}
