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

#include "spic.h"
#include "spinand.h"
#include "spinand_osal_boot0.h"

extern __s32 ecc_type;
extern __u32 SPN_BLOCK_SIZE;
extern __u32 SPN_PAGE_SIZE;

__s32 spi_nand_getsr(__u32 spi_no, __u8 *reg)
{
	__s32 ret = NAND_OP_TRUE;
	__u8 sdata[2] ;
	__u32 txnum;
	__u32 rxnum;

	txnum = 2;
	rxnum = 1;

	sdata[0] = SPI_NAND_GETSR;
	/*status adr:0xc0;feature adr:0xb0;protection adr:0xa0*/
	sdata[1] = 0xc0;
	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, (void *)reg, 0);

	return ret;
}

__s32 spi_wait_status(__u32 spi_no, __u8 *status)
{
	__s32 timeout = 0xfffff;
	__s32 ret = NAND_OP_TRUE;

	while (1) {
		ret = spi_nand_getsr(spi_no, status);
		if (ret != NAND_OP_TRUE) {
			SPINAND_Print("m0_spi_wait_status getsr fail!\n");
			return ret;
		}
		if (!(*(__u8 *)status & SPI_NAND_READY))
			break;
		if (timeout < 0) {
			SPINAND_Print("m0_spi_wait_status timeout!\n");
			return -ERR_TIMEOUT;
		}
		timeout--;
	}
	return NAND_OP_TRUE;
}


/*mode=0:check ecc status  mode=1:check operation status*/
__s32 spi_nand_read_status(__u32 spi_no, __u32 chip, __u8 status, __u32 mode)
{
	__s32 ret = NAND_OP_TRUE;

	Spic_sel_ss(spi_no, chip);

	if (mode) {
		ret = spi_wait_status(spi_no, &status);
		if (ret != NAND_OP_TRUE)
			return ret;

		if (status & SPI_NAND_ERASE_FAIL) {
			SPINAND_Print("spi_nand_read_status : erase fail,\
					status = 0x%x\n", status);
			ret = NAND_OP_FALSE;
		}
		if (status & SPI_NAND_WRITE_FAIL) {
			SPINAND_Print("spi_nand_read_status : write fail,\
					status = 0x%x\n", status);
			ret = NAND_OP_FALSE;
		}
	} else {
		ret = spi_wait_status(spi_no, &status);
		if (ret != NAND_OP_TRUE)
			return ret;

		if (ecc_type) {
			ret = aw_spinand_ecc_check_ecc(ecc_type, status >> SPI_NAND_ECC_FIRST_BIT);
		} else {
			ret = aw_spinand_check_ecc_old_scheme(status);
		}
	}

	return ret;
}

__s32 spi_nand_setecc(__u32 spi_no, __u32 chip, __u8 reg)
{
	__s32 ret = NAND_OP_TRUE;
	__u8 sdata[3];
	__u32 txnum;
	__u32 rxnum;
	__u8 status = 0;

	txnum = 3;
	rxnum = 0;

	sdata[0] = SPI_NAND_SETSR;
	sdata[1] = 0x90;       /*feature adr:0x90 for XTX spinand*/
	sdata[2] = reg;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, NULL, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	return ret;
}

 __s32 spi_nand_setblocklock(__u32 spi_no, __u32 chip, __u8 reg)
{
	__s32 ret = NAND_OP_TRUE;
	__u8 sdata[3];
	__u32 txnum;
	__u32 rxnum;
	__u8 status = 0;

	txnum = 3;
	rxnum = 0;

	sdata[0] = SPI_NAND_SETSR;
	/*status adr:0xc0;feature adr:0xb0;protection adr:0xa0*/
	sdata[1] = 0xa0;
	sdata[2] = reg;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, NULL, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	return ret;
}

 __s32 spi_nand_setotp(__u32 spi_no, __u32 chip, __u8 reg)
{
	__s32 ret = NAND_OP_TRUE;
	__u8 sdata[3];
	__u32 txnum;
	__u32 rxnum;
	__u8 status = 0;

	txnum = 3;
	rxnum = 0;

	sdata[0] = SPI_NAND_SETSR;
	/*status adr:0xc0;feature adr:0xb0;protection adr:0xa0*/
	sdata[1] = 0xb0;
	sdata[2] = reg;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, NULL, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	return ret;
}

 __s32 spi_nand_getotp(__u32 spi_no, __u32 chip, __u8 *reg)
{
	__s32 ret = NAND_OP_TRUE;
	__u8 sdata[3];
	__u32 txnum;
	__u32 rxnum;
	__u8 status = 0;

	txnum = 2;
	rxnum = 1;

	sdata[0] = SPI_NAND_GETSR;
	/*status adr:0xc0;feature adr:0xb0;protection adr:0xa0*/
	sdata[1] = 0xb0;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, reg, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	return ret;
}


__s32 spi_nand_reset(__u32 spi_no, __u32 chip)
{
	__u8 sdata = SPI_NAND_RESET;
	__s32 ret = NAND_OP_TRUE;
	__u32 txnum;
	__u32 rxnum;
	__u8  status = 0;

	txnum = 1;
	rxnum = 0;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)&sdata, rxnum, NULL, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = NAND_OP_TRUE;

	return ret;
}

__s32 spi_nand_read_id(__u32 spi_no, __u32 chip, __u8 *id, __u8 len, int dummy)
{
	__u8 txbuf[2] = {SPI_NAND_RDID, 0x00};
	__s32 ret = NAND_OP_TRUE;
	__u32 txnum;
	__u32 rxnum;

	if (dummy)
		txnum = 2;
	else
		txnum = 1;

	rxnum = len;

	Spic_sel_ss(spi_no, chip);

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)&txbuf, rxnum, (void *)id, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = NAND_OP_TRUE;

	return ret;
}

__s32 spi_nand_read_x1(__u32 spi_no, __u32 page_num, __u32 mbyte_cnt,
		__u32 sbyte_cnt, void *mbuf, void *sbuf, __u32 column)
{
	__u32 txnum;
	__u32 rxnum;
	__u32 page_addr = page_num ;
	__u8  sdata[8] = {0};
	__s32 ret = NAND_OP_TRUE;
	__u8 status = 0;
	__s32 ecc_status = 0;
	__u8 bad_flag[16];
	__u8 plane_select;

	plane_select = (page_addr / (SPN_BLOCK_SIZE / SPN_PAGE_SIZE)) & 0x1;

	txnum = 4;
	rxnum = 0;

	sdata[0] = SPI_NAND_PAGE_READ;
	sdata[1] = (page_addr>>16)&0xff; /*9dummy+15bit row adr*/
	sdata[2] = (page_addr>>8)&0xff;
	sdata[3] = page_addr&0xff;

	Spic_config_dual_mode(spi_no, 0, 0, txnum);
	ret = Spic_rw(spi_no, txnum, (void *)sdata, rxnum, NULL, 0);
	if (ret != NAND_OP_TRUE)
		return ret;

	ret = spi_wait_status(spi_no, &status);
	if (ret != NAND_OP_TRUE)
		return ret;

	ecc_status = spi_nand_read_status(spi_no, 0, status, 0);
	if (ecc_status == -ERR_ECC)
		return NAND_OP_FALSE;

	if (mbuf) {
		if (OperationOpt & SPINAND_ONEDUMMY_AFTER_RANDOMREAD) {
			txnum = 5;
			rxnum = mbyte_cnt;

			if (OperationOpt & SPINAND_QUAD_READ)
				sdata[0] = SPI_NAND_READ_X4;
			else if (OperationOpt & SPINAND_DUAL_READ)
				sdata[0] = SPI_NAND_READ_X2;
			else
				sdata[0] = SPI_NAND_FAST_READ_X1;

			sdata[1] = 0x0; /*1byte dummy*/
			/*4bit dummy,12bit column adr*/
			sdata[2] = ((column >> 8) & 0xff);
			sdata[3] = column & 0xff;
			sdata[4] = 0x0; /*1byte dummy*/
		} else {
			/*read main data*/
			txnum = 4;
			rxnum = mbyte_cnt;

			if (OperationOpt & SPINAND_QUAD_READ)
				sdata[0] = SPI_NAND_READ_X4;
			else if (OperationOpt & SPINAND_DUAL_READ)
				sdata[0] = SPI_NAND_READ_X2;
			else
				sdata[0] = SPI_NAND_FAST_READ_X1;

			if (OperationOpt & SPINAND_TWO_PLANE_SELECT) {
				/*3bit dummy,1bit plane,12bit column adr*/
				if (plane_select)
					sdata[1] = ((column >> 8) & 0x0f)|0x10;
				else
					sdata[1] = ((column >> 8) & 0x0f);
			} else {
				/*4bit dummy,12bit column adr*/
				sdata[1] = ((column >> 8) & 0xff);
			}
				sdata[2] = column & 0xff;
				sdata[3] = 0x0; /*1byte dummy*/
		}
		/*signal read, dummy:1byte, signal tx:3*/
		if (OperationOpt & SPINAND_QUAD_READ)
			Spic_config_dual_mode(spi_no, 2, 0, txnum);
		else if (OperationOpt & SPINAND_DUAL_READ)
			Spic_config_dual_mode(spi_no, 1, 0, txnum);
		else
			Spic_config_dual_mode(spi_no, 0, 0, txnum);
		ret = Spic_rw(spi_no, txnum,
				(void *)sdata, rxnum, mbuf, 0);
		if (ret != NAND_OP_TRUE)
			return ret;
	}

	if (sbuf) {
		if (OperationOpt & SPINAND_ONEDUMMY_AFTER_RANDOMREAD) {
			txnum = 5;
			rxnum = 1;

			if (OperationOpt & SPINAND_QUAD_READ)
				sdata[0] = SPI_NAND_READ_X4;
			else if (OperationOpt & SPINAND_DUAL_READ)
				sdata[0] = SPI_NAND_READ_X2;
			else
				sdata[0] = SPI_NAND_FAST_READ_X1;

			sdata[1] = 0x0; /*1byte dummy*/
			/*4bit dummy,12bit column adr*/
			sdata[2] = (((512 * (SPN_PAGE_SIZE >> \
				SCT_SZ_WIDTH)) >> 8) & 0xff);
			sdata[3] = (512 * (SPN_PAGE_SIZE >> SCT_SZ_WIDTH)) & 0xff;
			sdata[4] = 0x0; /*1byte dummy*/
		} else {
			/*read bad mark area*/
			txnum = 4;
			rxnum = sbyte_cnt;

			if (OperationOpt & SPINAND_QUAD_READ)
				sdata[0] = SPI_NAND_READ_X4;
			else if (OperationOpt & SPINAND_DUAL_READ)
				sdata[0] = SPI_NAND_READ_X2;
			else
				sdata[0] = SPI_NAND_FAST_READ_X1;

			if (OperationOpt & SPINAND_TWO_PLANE_SELECT) {
			/*3bit dummy,1bit plane,12bit column adr*/
				if (plane_select)
					sdata[1] = (((512 * (SPN_PAGE_SIZE >>\
					SCT_SZ_WIDTH)) >> 8) & 0x0f) | 0x10;
				else
					sdata[1] = (((512 * (SPN_PAGE_SIZE >>\
						SCT_SZ_WIDTH)) >> 8) & 0x0f);
			} else {
				/*4bit dummy,12bit column adr*/
				sdata[1] = (((512 * (SPN_PAGE_SIZE >>\
						SCT_SZ_WIDTH)) >> 8) & 0xff);
			}
			sdata[2] = (512 * (SPN_PAGE_SIZE >> \
						SCT_SZ_WIDTH)) & 0xff;
			sdata[3] = 0x0; /*1byte dummy*/
		}
		/*signal read, dummy:1byte, signal tx:3*/
		if (OperationOpt & SPINAND_QUAD_READ)
			Spic_config_dual_mode(spi_no, 2, 0, txnum);
		else if (OperationOpt & SPINAND_DUAL_READ)
			Spic_config_dual_mode(spi_no, 1, 0, txnum);
		else
			Spic_config_dual_mode(spi_no, 0, 0, txnum);

		ret = Spic_rw(spi_no, txnum,
				(void *)sdata, rxnum, bad_flag, 0);
		if (ret != NAND_OP_TRUE)
			return ret;

		if (bad_flag[0] != 0xff)
			*(__u8 *)sbuf = 0;
		else
			*(__u8 *)sbuf = 0xff;

	}

	return 0;
}


__s32 read_single_page(struct boot_spinand_physical_param *readop,
		__u32 data_size, __u32 spare_only_flag)
{
	__s32 ret = NAND_OP_TRUE;
	__u32 addr;

	addr = readop->block * SPN_BLOCK_SIZE / SPN_PAGE_SIZE + readop->page;

	Spic_sel_ss(0, readop->chip);

	if (spare_only_flag)
		ret = spi_nand_read_x1(0, addr, 0, 16, NULL, readop->oobbuf, 0);
	else
		ret = spi_nand_read_x1(0, addr, data_size, 16,
				(__u8 *)readop->mainbuf, readop->oobbuf, 0);
	if (ret == NAND_OP_FALSE)
		printf("block%d page%d ecc err\n", readop->block, readop->page);
	return ret;
}

