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

#define KB (1024)
#define MB (KB * 1024)
#define to_kb(size) (size / KB)
#define to_mb(size) (size / MB)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* manufacture num */
#define MICRON_MANUFACTURE	0x2c
#define GD_MANUFACTURE		0xc8
#define ATO_MANUFACTURE		0x9b
#define WINBOND_MANUFACTURE	0xef
#define MXIC_MANUFACTURE	0xc2
#define TOSHIBA_MANUFACTURE	0x98
#define ETRON_MANUFACTURE	0xd5
#define XTXTECH_MANUFACTURE	0x0b
#define DSTECH_MANUFACTURE	0xe5
#define FORESEE_MANUFACTURE	0xcd
#define ZETTA_MANUFACTURE	0xba
#define FM_MANUFACTURE		0xa1
#define SKYHIGH_MANUFACTURE	0x01
#define HEYANGTEK_MANUFACTURE	0xc9


struct spinand_manufacture {
	unsigned char id;
	const char *name;
	struct aw_spinand_phy_info *info;
	unsigned int cnt;
};

#define SPINAND_FACTORY_INFO(_id, _name, _info)			\
	{							\
		.id = _id,					\
		.name = _name,					\
		.info = _info,					\
		.cnt = ARRAY_SIZE(_info),			\
	}

struct aw_spinand_phy_info gigadevice[] = {
	{
		.NandID		= {0xc8, 0xb1, 0x48, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_ONEDUMMY_AFTER_RANDOMREAD,
		.Freq = 100,
		.EccType	= BIT3_LIMIT2_TO_6_ERR7,
	},
	{
		.NandID		= {0xc8, 0xd1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		.NandID		= {0xc8, 0xd2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		.NandID		= {0xc8, 0x01, 0x7f, 0x7f, 0x7f, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_QUAD_NO_NEED_ENABLE,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xc8, 0x51, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		.NandID		= {0xc8, 0x52, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		.NandID		= {0xc8, 0x21, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_QUAD_NO_NEED_ENABLE,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xc8, 0x55, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		.NandID		= {0xc8, 0x92, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_SE01,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
};

struct aw_spinand_phy_info micron[] = {
	{
		.NandID		= {0x2c, 0x14, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_QUAD_NO_NEED_ENABLE,
		.Freq = 100,
		.EccType	= BIT3_LIMIT5_ERR2,
	},
	{
		.NandID		= {0x2c, 0x24, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_QUAD_NO_NEED_ENABLE |
			SPINAND_TWO_PLANE_SELECT,
		.Freq = 100,
		.EccType	= BIT3_LIMIT5_ERR2,
	},
};

struct aw_spinand_phy_info xtx[] = {
	{
		/* XTX26G02A */
		.NandID		= {0x0B, 0xE2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		/* XTX26G01A */
		.NandID		= {0x0B, 0xE1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		/* XT26G01C */
		.NandID		= {0x0B, 0x11, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT4_LIMIT5_TO_8_ERR9_TO_15,
	},
	{
		/* XT26G02C */
		.NandID		= {0x0B, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT4_LIMIT5_TO_7_ERR8_LIMIT_12,
	},
	{
		/* XT26G04C */
		.NandID		= {0x0B, 0x13, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 8,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT4_LIMIT5_TO_8_ERR9_TO_15,
	},
};

struct aw_spinand_phy_info fm[] = {
	{
		/* only rw stress test */
		.NandID		= {0xa1, 0xa1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_QUAD_NO_NEED_ENABLE,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xa1, 0xe4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
};

struct aw_spinand_phy_info etron[] = {

};

struct aw_spinand_phy_info toshiba[] = {

};

struct aw_spinand_phy_info ato[] = {

};

struct aw_spinand_phy_info mxic[] = {
	{
		.NandID		= {0xc2, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_STATUS,
		.EccType	= BIT4_LIMIT3_TO_4_ERR15,
	},
	{
		.NandID		= {0xc2, 0x26, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccFlag	= HAS_EXT_ECC_STATUS,
		.EccType	= BIT4_LIMIT5_TO_8_ERR9_TO_15,
	},
};

struct aw_spinand_phy_info winbond[] = {
	{
		.NandID		= {0xef, 0xaa, 0x21, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xef, 0xaa, 0x21, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_ERR2_LIMIT3,
	},
	{
		.NandID		= {0xef, 0xab, 0x21, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
};

struct aw_spinand_phy_info dosilicon[] = {
	{
		.NandID		= {0xe5, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xe5, 0xf2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_TWO_PLANE_SELECT,
		.Freq = 100,
		.EccType	= BIT3_LIMIT5_ERR2,
	},
	{
		.NandID		= {0xe5, 0xf4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ | SPINAND_TWO_PLANE_SELECT,
		.Freq = 100,
		.EccType	= BIT3_LIMIT5_ERR2,
	},
};

struct aw_spinand_phy_info foresee[] = {
	{
		.NandID		= {0xcd, 0xb1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT3_LIMIT3_TO_4_ERR7,
	},
	{
		.NandID		= {0xcd, 0xea, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
	{
		.NandID		= {0xcd, 0x71, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2_TO_ERR3,
	},
	{
		.NandID		= {0xcd, 0x72, 0x72, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
};

struct aw_spinand_phy_info zetta[] = {
	{
		.NandID		= {0xba, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
};

struct aw_spinand_phy_info skyhigh[] = {
	{
		.NandID		= {0x01, 0x25, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT2_ERR3,
	},
	{
		.NandID		= {0x01, 0x15, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT2_ERR3,
	},
	{
		.NandID		= {0x01, 0x35, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT2_ERR3,
	},
};

struct aw_spinand_phy_info heyangtek[] = {
	{
		.NandID		= {0xc9, 0x21, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.SectCntPerPage = 4,
		.PageCntPerBlk  = 64,
		.OperationOpt	= SPINAND_QUAD_READ | SPINAND_QUAD_PROGRAM |
			SPINAND_DUAL_READ,
		.Freq = 100,
		.EccType	= BIT2_LIMIT1_ERR2,
	},
};

static struct spinand_manufacture spinand_factory[] = {
	SPINAND_FACTORY_INFO(MICRON_MANUFACTURE, "Micron", micron),
	SPINAND_FACTORY_INFO(GD_MANUFACTURE, "GD", gigadevice),
	SPINAND_FACTORY_INFO(ATO_MANUFACTURE, "ATO", ato),
	SPINAND_FACTORY_INFO(WINBOND_MANUFACTURE, "Winbond", winbond),
	SPINAND_FACTORY_INFO(MXIC_MANUFACTURE, "Mxic", mxic),
	SPINAND_FACTORY_INFO(TOSHIBA_MANUFACTURE, "Toshiba", toshiba),
	SPINAND_FACTORY_INFO(ETRON_MANUFACTURE, "Etron", etron),
	SPINAND_FACTORY_INFO(XTXTECH_MANUFACTURE, "XTX", xtx),
	SPINAND_FACTORY_INFO(DSTECH_MANUFACTURE, "Dosilicon", dosilicon),
	SPINAND_FACTORY_INFO(FORESEE_MANUFACTURE, "Foresee", foresee),
	SPINAND_FACTORY_INFO(ZETTA_MANUFACTURE, "Zetta", zetta),
	SPINAND_FACTORY_INFO(FM_MANUFACTURE, "FM", fm),
	SPINAND_FACTORY_INFO(SKYHIGH_MANUFACTURE, "SkyHigh", skyhigh),
	SPINAND_FACTORY_INFO(HEYANGTEK_MANUFACTURE, "HeYangTek", heyangtek),
};

struct spinand_manufacture *spinand_detect_munufacture(unsigned char id)
{
	int index;
	struct spinand_manufacture *m;

	for (index = 0; index < ARRAY_SIZE(spinand_factory); index++) {
		m = &spinand_factory[index];
		if (m->id == id) {
			return m;
		}
	}

	printf("not detect any munufacture from id table\n");
	return NULL;
}

struct aw_spinand_phy_info *spinand_match_id(unsigned char *id)
{
	int i, j, match_max = 1, match_index = 0;
	struct aw_spinand_phy_info *pinfo;
	struct spinand_manufacture *m = NULL;

	m = spinand_detect_munufacture(id[0]);
	if (m == NULL)
		return NULL;


	for (i = 0; i < m->cnt; i++) {
		int match = 1;

		pinfo = &m->info[i];
		for (j = 1; j < MAX_ID_LEN; j++) {
			/* 0xFF matching all ID value */
			if (pinfo->NandID[j] != id[j] &&
					pinfo->NandID[j] != 0xFF)
				break;

			if (pinfo->NandID[j] != 0xFF)
				match++;
		}

		if (match > match_max) {
			match_max = match;
			match_index = i;
		}
	}

	if (match_max > 1)
		return &m->info[match_index];
	return NULL;
}
