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
 * (C) Copyright 2017-2020
 *Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#ifndef __spinand_h
#define __spinand_h

#include <common.h>
//#include <asm/arch/gpio.h>
#include <private_boot0.h>
#include <asm/io.h>


#define BOOT0_START_BLK_NUM             0
#define BLKS_FOR_BOOT0                  8
#define BOOT1_START_BLK_NUM             BLKS_FOR_BOOT0


#define BLKS_FOR_BOOT1_IN_128K_BLK   18
#define BLKS_FOR_BOOT1_IN_256K_BLK   18
#define BLKS_FOR_BOOT1_IN_512K_BLK   10
#define BLKS_FOR_BOOT1_IN_1M_BLK     5
#define BLKS_FOR_BOOT1_IN_2M_BLK     5
#define BLKS_FOR_BOOT1_IN_4M_BLK     5
#define BLKS_FOR_BOOT1_IN_8M_BLK     5

extern __u32 SPN_BLOCK_SIZE;
extern __u32 SPN_BLK_SZ_WIDTH;
extern __u32 SPN_PAGE_SIZE;
extern __u32 SPN_PG_SZ_WIDTH;
extern __u32 UBOOT_START_BLK_NUM;
extern __u32 UBOOT_LAST_BLK_NUM;
extern __u32 page_for_bad_block;
extern __u32 OperationOpt;

#define BIT(nr)                 (1UL << (nr))

#define SECTOR_SIZE             512U
#define SCT_SZ_WIDTH            9U

#define NAND_OP_TRUE            (0)				/*define the successful return value*/
#define NAND_OP_FALSE           (-1)				/*define the failed return value*/
#define ERR_TIMEOUT             14				/*hardware timeout*/
#define ERR_ECC                 12				/*too much ecc error*/
#define ERR_NANDFAIL            13				/*nand flash program or erase fail*/
#define SPINAND_BAD_BLOCK       1
#define SPINAND_GOOD_BLOCK	0

#define CFG_BUF_MODE		BIT(3)
#define CFG_ECC_ENABLE		BIT(4)
#define CFG_QUAD_ENABLE		BIT(0)

#define SPINAND_DUAL_READ			BIT(0)
#define SPINAND_QUAD_READ			BIT(1)
#define SPINAND_QUAD_PROGRAM			BIT(2)
#define SPINAND_QUAD_NO_NEED_ENABLE		BIT(3)

#define SPINAND_TWO_PLANE_SELECT	 			(1<<7)			/*nand flash need plane select for addr*/
#define SPINAND_ONEDUMMY_AFTER_RANDOMREAD			(1<<8)			/*nand flash need a dummy Byte after random fast read*/

#define HAS_EXT_ECC_SE01 (1 << 17)
#define HAS_EXT_ECC_STATUS (1 << 16)
#define EXT_ECC_SE01_SHIFT (17)
#define EXT_ECC_STATUS_SHIFT (16)

enum ecc_limit_err {
       ECC_TYPE_ERR = 0,
       BIT3_LIMIT2_TO_6_ERR7,
       BIT2_LIMIT1_ERR2,
       BIT2_LIMIT1_ERR2_TO_ERR3,
       BIT2_LIMIT2_ERR3,
       BIT2_LIMIT1_ERR2_LIMIT3,
       BIT2_ERR2_LIMIT3,
       BIT4_LIMIT3_TO_4_ERR15,
       BIT3_LIMIT3_TO_4_ERR7,
       BIT3_LIMIT5_ERR2,
       BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
       BIT4_LIMIT5_TO_8_ERR9_TO_15,
};

struct aw_spinand_phy_info {
#define MAX_ID_LEN 8
	unsigned char NandID[MAX_ID_LEN];
	unsigned int PageCntPerBlk;
	unsigned int SectCntPerPage;
	unsigned int OperationOpt;
	/*MHz*/
	unsigned int Freq;
    __u32	EccFlag;
	enum	ecc_limit_err EccType;
};
typedef struct {
    __u8        ChipCnt;                            /*the count of the total nand flash chips are currently connecting on the CE pin*/
    __u8        ConnectMode;                        /*the rb connect  mode*/
    __u8        BankCntPerChip;                     /*the count of the banks in one nand chip, multiple banks can support Inter-Leave*/
    __u8        DieCntPerChip;                      /*the count of the dies in one nand chip, block management is based on Die*/
    __u8        PlaneCntPerDie;                     /*the count of planes in one die, multiple planes can support multi-plane operation*/
    __u8        SectorCntPerPage;                   /*the count of sectors in one single physic page, one sector is 0.5k*/
    __u16       ChipConnectInfo;                    /*chip connect information, bit == 1 means there is a chip connecting on the CE pin*/
    __u32       PageCntPerPhyBlk;                   /*the count of physic pages in one physic block*/
    __u32       BlkCntPerDie;                       /*the count of the physic blocks in one die, include valid block and invalid block*/
    __u32       OperationOpt;                       /*the mask of the operation types which current nand flash can support support*/
    __u32       FrequencePar;                       /*the parameter of the hardware access clock, based on 'MHz'*/
    __u32       SpiMode;                            /*spi nand mode, 0:mode 0, 3:mode 3*/
    __u8        NandChipId[8];                      /*the nand chip id of current connecting nand chip*/
    __u32       pagewithbadflag;                    /*bad block flag was written at the first byte of spare area of this page*/
    __u32       MultiPlaneBlockOffset;              /*the value of the block number offset between the two plane block*/
    __u32       MaxEraseTimes;                      /*the max erase times of a physic block*/
    __u32       MaxEccBits;                         /*the max ecc bits that nand support*/
    __u32       EccLimitBits;                       /*the ecc limit flag for tne nand*/
    __u32       uboot_start_block;
    __u32       uboot_next_block;
    __u32       logic_start_block;
    __u32       nand_specialinfo_page;
    __u32       nand_specialinfo_offset;
    __u32       physic_block_reserved;
    __u32	sample_mode;
    __u32	sample_delay;
    __u32	EccFlag;
    enum	ecc_limit_err EccType;
    __u32	boot_offset;
    __u32	boot_size;
    __u32	boot_param_offset;
    __u32       Reserved[4];
} boot_spinand_para_t;


typedef struct boot_spiflash_info {
	__u32 chip_cnt;
	__u32 blk_cnt_per_chip;
	__u32 blocksize;
	__u32 pagesize;
	__u32 pagewithbadflag; /*bad block flag was written at the first byte of spare area of this page*/
} boot_spiflash_info_t;

struct boot_spinand_physical_param {
	__u32  chip; /*chip no*/
	__u32  block; /* block no within chip*/
	__u32  page; /* apge no within block*/
	__u32  sectorbitmap;
	void   *mainbuf; /*data buf*/
	void   *oobbuf; /*oob buf*/
};

int general_check_ecc(unsigned char ecc,
		unsigned char limit_from, unsigned char limit_to,
		unsigned char err_from, unsigned char err_to);
int check_ecc_bit2_limit1_err2_limit3(unsigned char ecc);
int check_ecc_bit3_limit5_err2(unsigned char ecc);
int check_ecc_bit4_limit5_7_err8_limit12(unsigned char ecc);
int check_ecc_bit2_err2_limit3(unsigned char ecc);
int aw_spinand_ecc_check_ecc(enum ecc_limit_err type, u8 status);
int aw_spinand_check_ecc_old_scheme(u8 status);

#endif
