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
#include "spinand.h"
#include "spic.h"

int general_check_ecc(unsigned char ecc,
		unsigned char limit_from, unsigned char limit_to,
		unsigned char err_from, unsigned char err_to)
{
	if (ecc < limit_from) {
		return NAND_OP_TRUE;
	} else if (ecc >= limit_from && ecc <= limit_to) {
		return NAND_OP_TRUE;
	} else if (ecc >= err_from && ecc <= err_to) {
		SPINAND_Print("ecc error 0x%x\n", ecc);
		return -ERR_ECC;
	} else {
		SPINAND_Print("unknown ecc value 0x%x\n", ecc);
		return -ERR_ECC;
	}
}

int check_ecc_bit2_limit1_err2_limit3(unsigned char ecc)
{
	if (ecc == 0) {
		return NAND_OP_TRUE;
	} else if (ecc == 1 || ecc == 3) {
		return NAND_OP_TRUE;
	} else {
		SPINAND_Print("ecc error 0x%x\n", ecc);
		return -ERR_ECC;
	}
}

int check_ecc_bit3_limit5_err2(unsigned char ecc)
{
	if (ecc <= 1) {
		return NAND_OP_TRUE;
	} else if (ecc == 3 || ecc == 5) {
		return NAND_OP_TRUE;
	} else {
		SPINAND_Print("ecc error 0x%x\n", ecc);
		return -ERR_ECC;
	}
}

int check_ecc_bit4_limit5_7_err8_limit12(unsigned char ecc)
{
	if (ecc <= 4) {
		return NAND_OP_TRUE;
	} else if ((ecc >= 5 && ecc <= 7) || (ecc >= 12)) {
		return NAND_OP_TRUE;
	} else {
		SPINAND_Print("ecc err 0x%x\n", ecc);
		return -ERR_ECC;
	}
}
int check_ecc_bit2_err2_limit3(unsigned char ecc)
{
	if (ecc == 0 || ecc == 1) {
		return NAND_OP_TRUE;
	} else if (ecc == 3) {
		return NAND_OP_TRUE;
	} else {
		SPINAND_Print("ecc error 0x%x\n", ecc);
		return -ERR_ECC;
	}
}

int aw_spinand_ecc_check_ecc(enum ecc_limit_err type, u8 status)
{
	unsigned char ecc;

	switch (type) {
	case BIT3_LIMIT2_TO_6_ERR7:
		ecc = status & 0x07;
		return general_check_ecc(ecc, 2, 6, 7, 7);
	case BIT2_LIMIT1_ERR2:
		ecc = status & 0x03;
		return general_check_ecc(ecc, 1, 1, 2, 2);
	case BIT2_LIMIT1_ERR2_TO_ERR3:
		ecc = status & 0x03;
		return general_check_ecc(ecc, 1, 1, 2, 3);
	case BIT2_LIMIT2_ERR3:
		ecc = status & 0x03;
		return general_check_ecc(ecc, 2, 2, 3, 3);
	case BIT2_LIMIT1_ERR2_LIMIT3:
		ecc = status & 0x03;
		return check_ecc_bit2_limit1_err2_limit3(ecc);
	case BIT2_ERR2_LIMIT3:
		ecc = status & 0x03;
		return check_ecc_bit2_err2_limit3(ecc);
	case BIT4_LIMIT3_TO_4_ERR15:
		ecc = status & 0x0f;
		return general_check_ecc(ecc, 3, 4, 15, 15);
	case BIT3_LIMIT3_TO_4_ERR7:
		ecc = status & 0x07;
		return general_check_ecc(ecc, 3, 4, 7, 7);
	case BIT3_LIMIT5_ERR2:
		ecc = status & 0x07;
		return check_ecc_bit3_limit5_err2(ecc);
	case BIT4_LIMIT5_TO_7_ERR8_LIMIT_12:
		ecc = status & 0x0f;
		return check_ecc_bit4_limit5_7_err8_limit12(ecc);
	case BIT4_LIMIT5_TO_8_ERR9_TO_15:
		ecc = status & 0x0f;
		return general_check_ecc(ecc, 5, 8, 9, 15);
	default:
		SPINAND_Print("%s() %d: no support ecc type:%d \n", __func__, __LINE__, type);
		return -ERR_ECC;
	}
}

int aw_spinand_check_ecc_old_scheme(u8 status)
{
	if (((status >> SPI_NAND_ECC_FIRST_BIT) & SPI_NAND_ECC_BITMAP) == 0x0) {
		return NAND_OP_TRUE;
	} else if (((status >> SPI_NAND_ECC_FIRST_BIT) & SPI_NAND_ECC_BITMAP) == 0x1) {
		return NAND_OP_TRUE;
	} else {
		SPINAND_Print("ecc error 0x%x, ecc_type = 0\n", status);
		return -ERR_ECC;
	}
}
