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
 * (C) Copyright 2019
 * frank@allwinnertech.com
 */

#ifndef __PRIVATE_OPENSBI__
#define __PRIVATE_OPENSBI__

/* copy from OpenSBI: fw_dynamic.h */

/** Expected value of info magic ('OSBI' ascii string in hex) */
#define FW_DYNAMIC_INFO_MAGIC_VALUE     0x4942534f

/** Maximum supported info version */
#define FW_DYNAMIC_INFO_VERSION_2       0x2
#define FW_DYNAMIC_INFO_VERSION_MAX     FW_DYNAMIC_INFO_VERSION_2

/** Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U     0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S     0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M     0x3

/** Representation dynamic info passed by previous booting stage */
struct fw_dynamic_info {
	/** Info magic */
	unsigned long magic;
	/** Info version */
	unsigned long version;
	/** Next booting stage address */
	unsigned long next_addr;
	/** Next booting stage mode */
	unsigned long next_mode;
	/** Options for OpenSBI library */
	unsigned long options;
	/**
	 * Preferred boot HART id
	 *
	 * It is possible that the previous booting stage uses same link
	 * address as the FW_DYNAMIC firmware. In this case, the relocation
	 * lottery mechanism can potentially overwrite the previous booting
	 * stage while other HARTs are still running in the previous booting
	 * stage leading to boot-time crash. To avoid this boot-time crash,
	 * the previous booting stage can specify last HART that will jump
	 * to the FW_DYNAMIC firmware as the preferred boot HART.
	 *
	 * To avoid specifying a preferred boot HART, the previous booting
	 * stage can set it to -1UL which will force the FW_DYNAMIC firmware
	 * to use the relocation lottery mechanism.
	 */
	unsigned long boot_hart;
} __attribute__((packed));

/** Possible options for OpenSBI library (copy from OpenSBI: sbi_scratch.h) */
enum sbi_scratch_options {
	/** Disable prints during boot */
	SBI_SCRATCH_NO_BOOT_PRINTS = (1 << 0),
	/** Enable runtime debug prints */
	SBI_SCRATCH_DEBUG_PRINTS = (1 << 1),
};

#define PRIVATE_OPENSBI_HEAD_MAGIC0			(('o' << 0)| ('p' << 8) | ('e' << 16) | ('n' << 24))
#define PRIVATE_OPENSBI_HEAD_MAGIC1			(('s' << 0)| ('b' << 8) | ('i' << 16))

/******************************************************************************/
/*               the control information stored in file head                  */
/******************************************************************************/
struct private_opensbi_head {
	unsigned int  jump_instruction;	/* jumping to real code */
	unsigned char magic[8];			/* ="opensbi" */
	unsigned int  dtb_base;			/* the address of dtb base*/
	unsigned int  uboot_base;		/* the address of uboot base*/
	unsigned int  res3;
	unsigned int  res4;
	unsigned char res5[8];
	unsigned char res6[8];
	unsigned int opensbi_base;		/* the address of opensbi base*/
};
#endif /* __PRIVATE_OPENSBI__ */
