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

#ifndef  __basic_nf_func_h
#define  __basic_nf_func_h

#define BOOT0_START_BLK_NUM             0
#if defined(CONFIG_ARCH_SUN8IW7) || defined(CONFIG_ARCH_SUN8IW11)
#define BLKS_FOR_BOOT0                  2
#else
#define BLKS_FOR_BOOT0                  4
#endif
#define BOOT1_START_BLK_NUM             BLKS_FOR_BOOT0

#define BLKS_FOR_BOOT1_IN_16K_BLK_NF    32
#define BLKS_FOR_BOOT1_IN_32K_BLK_NF    8

#define BLKS_FOR_BOOT1_IN_128K_BLK_NF   5
#if defined(CONFIG_ARCH_SUN8IW7)
#define BLKS_FOR_BOOT1_IN_256K_BLK_NF   40
#define BLKS_FOR_BOOT1_IN_512K_BLK_NF   20
#else
#define BLKS_FOR_BOOT1_IN_256K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_512K_BLK_NF   5
#endif

#define BLKS_FOR_BOOT1_IN_1M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_2M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_4M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_8M_BLK_NF     5

extern __u32 NF_BLOCK_SIZE;
extern __u32 NF_BLK_SZ_WIDTH;
extern __u32 NF_PAGE_SIZE;
extern __u32 NF_PG_SZ_WIDTH;
extern __u32 BOOT1_LAST_BLK_NUM;
extern __u32 page_with_bad_block;
extern __u32 BOOT1_START_BLK;
extern __u32 NF_LOGIC_START_BLOCK;
extern __u32 BOOT_PARAM_OFFSET;
extern __u8 uboot_backup;

#define NF_SECTOR_SIZE                  512U
#define NF_SCT_SZ_WIDTH                 9U
#define OOB_BUF_SIZE_PER_SECTOR         4


#define 	NF_OK         				0
#define 	NF_GOOD_BLOCK 				0
#define 	NF_OVERTIME_ERR  			1
#define 	NF_ECC_ERR       			2
#define 	NF_BAD_BLOCK     			3
#define 	NF_ERASE_ERR     			4
#define 	NF_PROG_ERR      			5
#define 	NF_NEW_BAD_BLOCK 			6
#define 	NF_LACK_BLKS     			7
#define 	NF_ERROR     				-1
#define 	NF_ERR_COUNT                8



#define MAX_PAGE_SIZE         SZ_8K
#define BAD_BLK_FLAG          0

typedef enum
{
	ADV_NF_OK               =0,
	ADV_NF_FIND_OK          =0,
	ADV_NF_NO_NEW_BAD_BLOCK =0,
	ADV_NF_ERROR              ,
	ADV_NF_NO_FIND_ERR        ,
	ADV_NF_OVERTIME_ERR       ,
	ADV_NF_LACK_BLKS          ,
	ADV_NF_NEW_BAD_BLOCK      ,
}adv_nf_errer_e;


extern __s32 load_and_check_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size);

extern __s32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
						        __u32 size, __u32 blk_size, __u32 *blks );

extern __s32 write_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size );

extern __s32 write_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
					             __u32 size, __u32 blk_size, __u32 * blks );

extern __s32  NF_open ( void );
extern __s32  NF_close( void );
extern __s32  NF_read ( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_write( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_erase( __u32 blk_num );
extern __s32  NF_read_status ( __u32 blk_num );
extern __s32  NF_mark_bad_block( __u32 blk_num );
extern __s32  NF_verify_block( __u32 blk_num );

struct boot_physical_param;

extern __u32 NAND_Getlsbpage_type(void);
extern __u32 NAND_GetLsbblksize(void);
extern __s32 NFB_PhyInit(void);
extern __s32 NFB_PhyExit(void);
extern __s32 NFB_PhyRead (struct boot_physical_param *readop);
extern __u32 Nand_Is_lsb_page(__u32 page);
extern __u8  *get_page_buf( void );
extern __s32 check_magic( __u32 *mem_base, const char *magic );
extern __u32 load_uboot_in_one_block_judge(__u32 length);

extern int verify_addsum( void *mem_base, __u32 size );
extern __u32 g_mod( __u32 dividend, __u32 divisor, __u32 *quot_p );
extern __s32 check_sum( __u32 *mem_base, __u32 size );

#endif

