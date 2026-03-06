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

#include "spinand.h"
#include "spic.h"
#include "spinand_osal_boot0.h"

extern __s32 spi_nand_reset(__u32 spi_no, __u32 chip);
extern __s32 read_single_page(struct boot_spinand_physical_param *readop,
		__u32 data_size, __u32 spare_only_flag);
extern __s32 spi_nand_setecc(__u32 spi_no, __u32 chip, __u8 reg);
extern __s32 spi_nand_setotp(__u32 spi_no, __u32 chip, __u8 reg);
extern __s32 spi_nand_getotp(__u32 spi_no, __u32 chip, __u8 *reg);
extern __s32 spi_nand_setblocklock(__u32 spi_no, __u32 chip, __u8 reg);
extern void spic0_sample_point_delay_set_legacy(unsigned int freq);
__s32 SpiNand_PhyInit(void);
__s32 SpiNand_PhyExit(void);

unsigned int def_freq = 100; // 100MHz
__s32 ecc_type;

/*extern  struct __NandStorageInfo_t  NandStorageInfo;*/
extern __s32 BOOT_NandGetPara(boot_spinand_para_t *param, uint size);
__u32 SPN_BLOCK_SIZE = 0;
__u32 SPN_BLK_SZ_WIDTH = 0;
__u32 SPN_PAGE_SIZE = 0;
__u32 SPN_PG_SZ_WIDTH = 0;
__u32 UBOOT_START_BLK_NUM = 0;
__u32 UBOOT_LAST_BLK_NUM = 0;
__u32 LOGIC_START_BLOCK;
__u32 page_for_bad_block = 0;
__u32 OperationOpt = 0;
__u32 BOOT_OFFSET;
__u32 BOOT_SIZE;
__u32 BOOT_PARAM_OFFSET;

struct aw_spinand_phy_info *spinand_phyinfo;

#define SECURE_STORAGE_BLKS 8

/*
******************************************************************************
*                           ANALYZE NAND FLASH STORAGE SYSTEM
*
*Description: Analyze nand flash storage system, generate the nand flash
physical
*             architecture parameter and connect information.
*
*Arguments  : none
*
*Return     : analyze result;
*               = 0     analyze successful;
*               < 0     analyze failed, can't recognize or some other error.
******************************************************************************
*/

void spinand_get_uboot_range(struct aw_spinand_phy_info *info, unsigned int *start, unsigned int *end)
{
#define UBOOT_START_BLOCK_SMALLNAND 8
#define UBOOT_START_BLOCK_BIGNAND 4
#define AW_MTD_SPINAND_UBOOT_BLKS 24

#define MAX_MTD_PART0_BLKS 50
#define MAX_UBOOT_END_BLK (MAX_MTD_PART0_BLKS - SECURE_STORAGE_BLKS)

	unsigned int _start = 0;
	unsigned int _end = 0;
	unsigned int pagecnt = info->PageCntPerBlk;

	unsigned int blksize = info->PageCntPerBlk * info->SectCntPerPage;
	blksize = blksize << 9;


	/* small nand:block size < 1MB;  reserve 4M for uboot */
	if (blksize <= SZ_128K) {
		_start = UBOOT_START_BLOCK_SMALLNAND; // 8
		_end = _start + 32;
	} else if (blksize <= SZ_256K) {
		_start = UBOOT_START_BLOCK_SMALLNAND;
		_end = _start + 16;
	} else if (blksize <= SZ_512K) {
		_start = UBOOT_START_BLOCK_SMALLNAND;
		_end = _start + 8;
	} else if (blksize <= SZ_1M && pagecnt <= 128) { //1M
		_start = UBOOT_START_BLOCK_SMALLNAND;
		_end = _start + 4;
	/* big nand;  reserve at least 20M for uboot */
	} else if (blksize <= SZ_1M && pagecnt > 128) {
		_start = UBOOT_START_BLOCK_BIGNAND; // 4
		_end = _start + 20;
	} else if (blksize <= SZ_2M) {
		_start = UBOOT_START_BLOCK_BIGNAND;
		_end = _start + 10;
	} else {
		_start = UBOOT_START_BLOCK_BIGNAND;
		_end = _start + 8;
	}

	if (AW_MTD_SPINAND_UBOOT_BLKS > 0)
		_end = _start + AW_MTD_SPINAND_UBOOT_BLKS;

	if (_end > MAX_UBOOT_END_BLK) {
		/*
		 *printf("uboot end blocks %u is larger than max %u, resize!\n",
		 *                _end, MAX_UBOOT_END_BLK);
		 */
		_end = MAX_UBOOT_END_BLK;
	}

	if (start)
		*start = _start;
	if (end)
		*end = _end;
}

void spinand_init_ecc_type(boot_spinand_para_t *nand_info)
{
	struct aw_spinand_phy_info *pinfo = NULL;

	if ((nand_info->EccFlag & BIT(EXT_ECC_SE01_SHIFT)) ||
			(nand_info->EccFlag & BIT(EXT_ECC_STATUS_SHIFT)))
		SPINAND_Print("%s() %d: no support EccFlag of HAS_EXT_ECC_SE01 && EXT_ECC_STATUS\n",
			__func__, __LINE__);

	if (nand_info->EccType) {
		ecc_type  = nand_info->EccType;
	} else {
		pinfo = spinand_match_id(nand_info->NandChipId);
		if (pinfo)
			ecc_type  = pinfo->EccType;
	}
}

__s32  BOOT_AnalyzeSpiNandSystem(void)
{
	__s32 result;
	unsigned char reg = 0;
	int dummy = 1;
	__u8 id[8];
	boot_spinand_para_t nand_info;

	memset(&nand_info, 0, sizeof(boot_spinand_para_t));

	if (BOOT_NandGetPara(&nand_info, sizeof(boot_spinand_para_t)) != 0)
		return -1;

	OperationOpt = nand_info.OperationOpt;
	printf("OperationOpt:%x \n", OperationOpt);

	UBOOT_START_BLK_NUM = nand_info.uboot_start_block;
	UBOOT_LAST_BLK_NUM = nand_info.uboot_next_block;
	LOGIC_START_BLOCK = nand_info.logic_start_block;
	BOOT_OFFSET = nand_info.boot_offset;
	BOOT_SIZE = nand_info.boot_size;
	BOOT_PARAM_OFFSET = nand_info.boot_param_offset;

	spinand_init_ecc_type(&nand_info);

	/*reset the nand flash chip on boot chip select*/
	result = spi_nand_reset(0, 0);
	if (result) {
		SPINAND_Print("spi nand reset fail!\n");
		return -1;
	}

	/*if OperationOpt is zero, we get bootpara from own idtbl*/
	if (OperationOpt == 0) {
retry_nodummy:
		/*read id*/
		spi_nand_read_id(0, 0, id, 8, dummy);

		/*match id table*/
		spinand_phyinfo = spinand_match_id(id);
		if (spinand_phyinfo == NULL) {
			if (dummy == 0) {
				printf("read the spinand id is err\n");
				printf("the read id: %02x %02x %02x %02x %02x %02x %02x %02x\n",
						id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
				return -1;
			}
			dummy = 0;
			goto retry_nodummy;
		}

		spinand_get_uboot_range(spinand_phyinfo, &UBOOT_START_BLK_NUM, &UBOOT_LAST_BLK_NUM);
		OperationOpt = spinand_phyinfo->OperationOpt;
	}

	reg |= CFG_ECC_ENABLE;
	if (OperationOpt & SPINAND_QUAD_READ)
		reg |= CFG_QUAD_ENABLE;
	if (OperationOpt & SPINAND_QUAD_NO_NEED_ENABLE)
		reg &= ~CFG_QUAD_ENABLE;

	if (nand_info.NandChipId[0] == 0xef || spinand_phyinfo->NandID[0] == 0xef) {
		/*winbond: 0x18,bit3 is BUF mode;*/
		reg |= CFG_BUF_MODE;
		spi_nand_setotp(0, 0, reg);
	} else if (nand_info.NandChipId[0] == 0x98 || spinand_phyinfo->NandID[0] == 0x98) {
		/* toshiba: bit2 is BBI,bit1 is HSE;*/
		reg |= 0x06;
		spi_nand_setotp(0, 0, reg);
	} else if (nand_info.NandChipId[0] == 0xa1 || spinand_phyinfo->NandID[0] == 0xa1) {
		/* XTX spinand */
		spi_nand_setotp(0, 0, 0x0);
		spi_nand_setecc(0, 0, 0x10);
	} else {
		spi_nand_setotp(0, 0, reg);  /*other:0x10,bit3~1 don't care*/
	}

	if ((OperationOpt & SPINAND_QUAD_READ) &&
			!(OperationOpt & SPINAND_QUAD_NO_NEED_ENABLE)) {
		reg = 0;
		spi_nand_getotp(0, 0, &reg);
		if (!(reg & CFG_QUAD_ENABLE)) {
			OperationOpt &= ~SPINAND_QUAD_READ;
			printf("spinand quad mode error\n");
		}
	}

	spi_nand_setblocklock(0, 0, 0);

	return 0;
}

__s32 SpiNand_GetFlashInfo(boot_spiflash_info_t *param)
{
	boot_spinand_para_t nand_info;
	memset(&nand_info, 0, sizeof(boot_spinand_para_t));

	BOOT_NandGetPara(&nand_info, sizeof(boot_spinand_para_t));

	param->chip_cnt = nand_info.ChipCnt;
	param->blk_cnt_per_chip = nand_info.BlkCntPerDie *
		nand_info.DieCntPerChip;

	if (nand_info.OperationOpt == 0) {

		if (!spinand_phyinfo) {
			printf("err:no spinand phyinfo\n");
			return -1;
		}
		param->pagesize = spinand_phyinfo->SectCntPerPage;
		param->blocksize = param->pagesize * spinand_phyinfo->PageCntPerBlk;
	} else {
		param->pagesize = nand_info.SectorCntPerPage;
		param->blocksize = nand_info.SectorCntPerPage *
					nand_info.PageCntPerPhyBlk;
	}

	param->pagewithbadflag  = 0 ;   /* fix page 0 as bad flag page index*/

	return 0;
}

int spinand_get_boot_start(int copy)
{
#if CFG_BOOT0_LOAD_KERNEL
	if (copy == 0) {
		if (BOOT_OFFSET)
			return (BOOT_OFFSET >> SCT_SZ_WIDTH);
		else
			return (((UBOOT_LAST_BLK_NUM + SECURE_STORAGE_BLKS) * SPN_BLOCK_SIZE) >> SCT_SZ_WIDTH);
	} else {
		if (BOOT_OFFSET)
			return ((BOOT_OFFSET + BOOT_SIZE)>> SCT_SZ_WIDTH);
		else
			return (((UBOOT_LAST_BLK_NUM + SECURE_STORAGE_BLKS) * SPN_BLOCK_SIZE + CFG_BIMAGE_SIZE) >> SCT_SZ_WIDTH);
	}
#endif
	return 0;
}

/*
*****************************************************************************
*                       INIT NAND FLASH
*
*Description: initial nand flash,request hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  initial ok;
*               = FAIL    initial fail.
******************************************************************************
*/
__s32 SpiNand_PhyInit(void)
{
	__s32 ret;
	boot_spiflash_info_t param;

	ret = Spic_init(0);
	if (ret) {
		SPINAND_Print("Spic_init fail\n");
		goto error;
	}

	ret = BOOT_AnalyzeSpiNandSystem();
	if (ret) {
		SPINAND_Print("spi nand scan fail\n");
		goto error;
	}

	if (SpiNand_GetFlashInfo(&param) == -1)
		goto error;

	page_for_bad_block = param.pagewithbadflag;
	SPN_BLOCK_SIZE = param.blocksize * SECTOR_SIZE;
	SPN_PAGE_SIZE = param.pagesize * SECTOR_SIZE;

	if (!BOOT_PARAM_OFFSET)
		BOOT_PARAM_OFFSET = (UBOOT_LAST_BLK_NUM + SECURE_STORAGE_BLKS) * SPN_BLOCK_SIZE;

	SPINAND_Print("spinand UBOOT_START_BLK_NUM %d UBOOT_LAST_BLK_NUM %d\n",
		UBOOT_START_BLK_NUM, UBOOT_LAST_BLK_NUM);

	if (spinand_phyinfo && spinand_phyinfo->Freq != def_freq) {
		SPINAND_SetClk(0, spinand_phyinfo->Freq);
		spic0_sample_point_delay_set_legacy(spinand_phyinfo->Freq);
	}

	return 0;

error:
	SpiNand_PhyExit();
	return -1;

}

/*
******************************************************************************
*                       RELEASE NAND FLASH
*
*Description: release  nand flash and free hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  release ok;
*               = FAIL    release fail.
******************************************************************************
*/
__s32 SpiNand_PhyExit(void)
{
	Spic_exit(0);

	/* close nand flash bus clock gate */
	/*NAND_CloseAHBClock();*/

	return 0;
}

__s32 SpiNand_Check_BadBlock(__u32 block_num)
{
	struct boot_spinand_physical_param  para;
	__u8  oob_buf[16];
	__s32 ret;

	para.chip = 0;
	para.block = block_num;
	para.page = 0;
	para.mainbuf = NULL;
	para.oobbuf = oob_buf;
	ret = read_single_page(&para, 0, 1);
	if (ret != NAND_OP_TRUE) {
		SPINAND_Print("Check_BadBlock: read_single_page FAIL\n");
		return NAND_OP_FALSE;
	}
	if (oob_buf[0] != 0xFF) {
		SPINAND_Print("oob_buf[0] = %x\n", oob_buf[0]);
		return SPINAND_BAD_BLOCK;
	} else
		return SPINAND_GOOD_BLOCK;
}

__s32 SpiNand_Read(__u32 sector_num, void *buffer, __u32 N)
{
	struct boot_spinand_physical_param  para;
	__u8  oob_buf[16];
	__u32 page_nr;
	__u32 scts_per_page = SPN_PAGE_SIZE >> SCT_SZ_WIDTH;
	__u32 start_page;
	__u32 i;
	__u32 blk_num;
	__u32 not_full_page_flag = 0;
	__u32 data_size;

	para.chip = 0;
	blk_num = sector_num / (SPN_BLOCK_SIZE >> SCT_SZ_WIDTH);
	para.block = blk_num;
	start_page = (sector_num % (SPN_BLOCK_SIZE >> \
				SCT_SZ_WIDTH)) / scts_per_page;
	para.oobbuf = oob_buf;
	page_nr = N / scts_per_page;
	if (N % scts_per_page) {
		page_nr++;
		not_full_page_flag = 1;
	}
	for (i = 0; i < page_nr; i++) {
		para.mainbuf = (__u8 *)buffer + SPN_PAGE_SIZE * i;
		para.page = start_page + i;
		data_size = SPN_PAGE_SIZE;
		if ((i == (page_nr - 1)) && not_full_page_flag)
			data_size = (N << SCT_SZ_WIDTH) - (SPN_PAGE_SIZE * i);
		if (read_single_page(&para, data_size, 0) != NAND_OP_TRUE)
			return NAND_OP_FALSE;
	}

	return NAND_OP_TRUE;
}

int spinand_read_kernel_skip_bad_block(int block)
{
	struct boot_spinand_physical_param  para;
	__u8  oob_buf[16];

	para.chip = 0;
	para.block = block;
	para.oobbuf = oob_buf;
	para.page = 0;

	if (read_single_page(&para, 0, 1) == NAND_OP_TRUE &&
			oob_buf[0] != 0xff) {
			return 1;
	}

	return 0;
}

int spinand_read_kernel(uint sector_num, uint N, void *buffer)
{
	struct boot_spinand_physical_param  para;
	__u8  oob_buf[16];
	__u32 page_nr;
	__u32 scts_per_page = SPN_PAGE_SIZE >> SCT_SZ_WIDTH;
	__u32 start_page;
	__u32 i;
	__u32 blk_num;
	__u32 not_full_page_flag = 0;
	__u32 data_size;
	__u32 pages_per_block = SPN_BLOCK_SIZE / SPN_PAGE_SIZE;


	para.chip = 0;
	blk_num = sector_num / (SPN_BLOCK_SIZE >> SCT_SZ_WIDTH);

check:
	/* kernel skip bad block store , so skip bad block read*/
	if (spinand_read_kernel_skip_bad_block(blk_num)) {
		blk_num++;
		goto check;
	}

	para.block = blk_num;
	start_page = (sector_num % (SPN_BLOCK_SIZE >> \
				SCT_SZ_WIDTH)) / scts_per_page;
	para.oobbuf = oob_buf;
	page_nr = N / scts_per_page;
	if (N % scts_per_page) {
		page_nr++;
		not_full_page_flag = 1;
	}

	for (i = 0; i < page_nr; i++) {
		para.mainbuf = (__u8 *)buffer + SPN_PAGE_SIZE * i;

check2:
		para.page = start_page + i;

		/* check middle bad block*/
		if (i != 0 && para.page % pages_per_block == 0) {
				/* kernel skip bad block store , so skip bad block read*/
				if (spinand_read_kernel_skip_bad_block(++blk_num)) {
					start_page += pages_per_block;
					goto check2;
			}
		}
		data_size = SPN_PAGE_SIZE;
		if ((i == (page_nr - 1)) && not_full_page_flag)
			data_size = (N << SCT_SZ_WIDTH) - (SPN_PAGE_SIZE * i);

		if (read_single_page(&para, data_size, 0) != NAND_OP_TRUE) {
				return NAND_OP_FALSE;

		}
	}

	return NAND_OP_TRUE;
}

/**************************************************************************
*函数名称: load_in_many_blks
*函数原型：int32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num,
		void *buf, __u32 size, __u32 blk_size, __u32 *blks )
*函数功能: 从start_blk开始，载入file_length长度的内容到内存
*入口参数: start_blk         待访问的nand flash起始块号
*          last_blk_num      最后一个块的块号，用来限制访问范围
*          buf               内存缓冲区的起始地址
*          size              文件尺寸
*          blk_size          待访问的nand flash的块大小
*          blks              所占据的块数，包括坏块
*返 回 值: ADV_NF_OK                操作成功
*          ADV_NF_OVERTIME_ERR   操作超时
*          ADV_NF_LACK_BLKS      块数不足
*备    注: 1. 本函数只载入，不校验
**************************************************************************/
__s32 Spinand_Load_Boot1_Copy(__u32 start_blk, void *buf,
		__u32 size, __u32 blk_size, __u32 *blks)
{
	__u32 buf_base;
	__u32 buf_off;
	__u32 size_loaded;
	__u32 cur_blk_base;
	__u32 rest_size;
	__u32 blk_num;

	for (blk_num = start_blk, buf_base = (__u32)buf, buf_off = 0;
		buf_off < size; blk_num++) {
		if (SpiNand_Check_BadBlock(blk_num) == SPINAND_BAD_BLOCK)
			continue;

		cur_blk_base = blk_num * blk_size;
		rest_size = size - buf_off;
		size_loaded = (rest_size < blk_size) ?  rest_size : blk_size;

		if (SpiNand_Read(cur_blk_base >> SCT_SZ_WIDTH,
		(void *)buf_base, size_loaded >> SCT_SZ_WIDTH) != NAND_OP_TRUE)
			return NAND_OP_FALSE;

		buf_base += size_loaded;
		buf_off  += size_loaded;
	}


	*blks = blk_num - start_blk;
	if (buf_off == size)
		return NAND_OP_TRUE;
	else {
		printf("lack blocks with start block %d and buf size %x.\n",
				start_blk, size);
		return NAND_OP_FALSE;
	}
}
