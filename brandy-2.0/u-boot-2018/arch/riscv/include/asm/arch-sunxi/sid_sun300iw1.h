/*
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * wangwei <wangwei@allwinnertech.com>
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __SID_H__
#define __SID_H__

#include <linux/types.h>
#include <asm/arch/cpu.h>

#define SID_PRCTL               (IOMEM_ADDR(SUNXI_SID_BASE) + 0x40)
#define SID_PRKEY               (IOMEM_ADDR(SUNXI_SID_BASE) + 0x50)
#define SID_RDKEY               (IOMEM_ADDR(SUNXI_SID_BASE) + 0x60)

#define SID_ROTPK_VALUE(n)      (IOMEM_ADDR(SUNXI_SID_BASE) + 0x120 + (n * 4))
#define SID_ROTPK_CTRL          (IOMEM_ADDR(SUNXI_SID_BASE) + 0x140)
#define SID_ROTPK_CMP_RET_BIT   (0)
#define SID_ROTPK_EFUSED_BIT    (1)
#define SUNXI_EFUSE_RAM_BASE_IN_DDR	(0x80FFFF00)
#define SID_EFUSE               ((volatile void *)SUNXI_EFUSE_RAM_BASE_IN_DDR)
#define SID_OP_LOCK  (0xAC)

/*efuse power ctl*/
#define EFUSE_HV_SWITCH         (IOMEM_ADDR(SUNXI_PMU_AON_BASE) + 0x84)

#define EFUSE_CHIPID            (0x0)
#define EFUSE_BROM              (0x4c)
#define EFUSE_LCJS		EFUSE_BROM
#define EFUSE_ANTI_BRUSH	EFUSE_BROM
#define EFUSE_ROTPK             (0x74)
#define ANTI_BRUSH_MODE		(IOMEM_ADDR(SUNXI_SID_BASE) + 0x200 + EFUSE_ANTI_BRUSH)
#define ANTI_BRUSH_BIT_OFFSET	(0)

#define EFUSE_WMAC                (0x3c)
#define EFUSE_WMAC_SEL_FLAG(a)    ((a & 0xc0000000) >> 30)
#define EFUSE_WMAC_ADDR           (0x40)
#define EFUSE_WMAC_ADDR1          (0x40)
#define EFUSE_WMAC_ADDR2          (0x46)
#define SID_WMAC_SIZE             (96)
#define SID_WMAC1_SIZE            (48)
#define SID_WMAC2_SIZE            (48)

#define EFUSE_OEM_PROGRAM	  (0x98)
#define SID_OEM_PROGRAM_SIZE	  (488)

/* secure enbale bit */
#define SECURE_BIT_VAL 		(3)

/* write protect */
#define EFUSE_WRITE_PROTECT	(0x50)
#define EFUSE_READ_PROTECT	EFUSE_WRITE_PROTECT

/*write protect*/
#define SCC_ROTPK_BURNED_FLAG	(26)

/* The read/write protection bit occupies 32 bits */
#define LONG_EFUSE_RW_PROTECT_OFFSET 1
#define EFUSE_ACL_SET_BRUN_BIT (0x100000000)
#define EFUSE_ACL_SET_RD_FORBID_BIT (0x100000000)
#define EFUSE_BRUN_RD_OFFSET_MASK (0xFFFFFFFF)

/* ebp setting */
#define SID_EBP_BITMAP_START    (1664)
#define SID_EBP_BITMAP_BASE     (0xD0)
#define SID_EBP_BITMAP_WIDTH    (26)
#define SID_EBP_BITMAP_MASK     (0x3)
#define SID_EBP_MAP_BIT(i)      (SID_EBP_BITMAP_MASK << (i * 2))
#define SID_EBP_START           (1700)
#define SID_EBP_WIDTH           (12)
#define SID_EBP_POST_MASK       (0x7ff)
#define SID_EBP_CRC_SHIFT       (11)
#define SID_EBP_CRC_MASK        (0x1 << SID_EBP_CRC_SHIFT)
#define SID_EBP_IDX_MAX         (12)
#define SID_EBP_SIZE    (SID_EBP_IDX_MAX + 1)
#define SID_EBP_ID_OFFSET(i)    (SID_EBP_START + (SID_EBP_IDX_MAX * i))
#define SID_EBP_ID_MASK         (0xfff)
#define SID_EBP_ODD_BIT_SHIFT   (SID_EBP_WIDTH - 1)
#define SID_EBP_BITMAPALL_MASK  (0x3ffffff)

#endif    /*  #ifndef __SID_H__  */
