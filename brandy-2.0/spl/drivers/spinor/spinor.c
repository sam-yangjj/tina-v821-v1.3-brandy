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
 * (C) Copyright 20018-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: spinor driver for General spinor operations
 * Author: wangwei <wangwei@allwinnertech.com>
 * Date: 2018-11-15 14:18:18
 */

#include <arch/spi.h>
#include <arch/spinor.h>
#include <arch/spi-mem.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <boot_param.h>
#include "../spi-sunxi.h"
#include "../spif-sunxi.h"
#include <sunxi_flashmap.h>

#define SPINOR_DEBUG 0

#if SPINOR_DEBUG
#define spinor_debug(fmt, arg...)    printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#else
#define spinor_debug(fmt, arg...)
#endif

#define SYSTEM_PAGE_SIZE 512
#define READ_LEN (128*512)
#define JEDEC_MFR(id)		(id&0xff)

#ifndef CONFIG_SYS_CACHELINE_SIZE
#define CONFIG_SYS_CACHELINE_SIZE 64
#endif

/*
 * Clock Frequency for all commands (except Read), depend on flash, maybe 104M or 133M
 * Clock Frequency for READ(0x03) instructions is special, depend on flash, maybe only 50M or 80M
 * Default use FASTREAD(0x0b) which can work with sunxi spi clk(max 100M)
 */
static u8 read_cmd = CMD_READ_ARRAY_SLOW;
static enum spi_nor_protocol read_proto = SNOR_PROTO_1_1_1;
static enum spi_nor_protocol write_proto = SNOR_PROTO_1_1_1;
static u8 read_dummy;
static u8 spinor_4bytes_addr_mode;
boot_spinor_info_t *spinor_info;
#ifdef CFG_SOFT_ESTIMATE_SPIF_SPEED
static unsigned int rx_bytes_per_ms;
static unsigned int async_rx_start_ms, async_rx_bytes;
#endif
#if defined(CFG_BOOT0_LOAD_KERNEL) && defined(CONFIG_ARCH_SUN8IW21)
unsigned char *boot_param_cache = (unsigned char *)0x42FFF000;
#else
unsigned char boot_param_cache[BOOT_PARAM_SIZE] __aligned(CONFIG_SYS_CACHELINE_SIZE);
#endif

void dump_spinor_info(boot_spinor_info_t *spinor_info)
{
	spinor_debug("\n"
		"----------------------\n"
		"magic:%s\n"
		"readcmd:%x\n"
		"read_mode:%d\n"
		"write_mode:%d\n"
		"flash_size:%dM\n"
		"addr4b_opcodes:%d\n"
		"erase_size:%d\n"
		"frequency:%d\n"
		"sample_mode:%x\n"
		"sample_delay:%x\n"
		"read_proto:%x\n"
		"write_proto:%x\n"
		"read_dummy:%d\n"
		"----------------------\n",
		spinor_info->magic, spinor_info->readcmd,
		spinor_info->read_mode, spinor_info->write_mode,
		spinor_info->flash_size, spinor_info->addr4b_opcodes,
		spinor_info->erase_size, spinor_info->frequency,
		spinor_info->sample_mode, spinor_info->sample_delay,
		spinor_info->read_proto, spinor_info->write_proto,
		spinor_info->read_dummy);
}

static int spi_nor_read_write_reg(struct spi_mem_op *op, void *buf)
{
	if (op->data.dir == SPI_MEM_DATA_IN)
		op->data.buf.in = buf;
	else
		op->data.buf.out = buf;
	return spi_mem_exec_op(op, 0);
}

static int spi_nor_read_reg(u8 code, u8 *val, int len)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(code, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_MODE,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_IN(len, NULL, 1));
	int ret;

	op.cmd.buswidth = spi_nor_get_protocol_inst_nbits(read_proto);
	op.dummy.buswidth = 1;
	op.data.buswidth = op.cmd.buswidth;

	if (op.cmd.buswidth == 8)
		op.dummy.cycle = 8;

	ret = spi_nor_read_write_reg(&op, val);
	if (ret < 0)
		printf("error %d reading %x\n", ret, code);

	return ret;
}

#ifdef CFG_SUNXI_SPIF
static int spi_nor_write_reg(u8 opcode, u8 *buf, int len)
{
	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_CMD(opcode, 1),
				   SPI_MEM_OP_NO_ADDR,
				   SPI_MEM_OP_MODE(buf, 1),
				   SPI_MEM_OP_NO_DUMMY,
				   SPI_MEM_OP_NO_DATA);

	/* get transfer protocols. */
	op.cmd.buswidth = spi_nor_get_protocol_inst_nbits(write_proto);
	op.mode.buswidth = spi_nor_get_protocol_data_nbits(read_proto);

	return spi_mem_exec_op(&op, 0);
}

static int spi_nor_write_reg_2byte(u8 opcode, u8 *buf)
{
	loff_t cmd_sr_cr = (opcode << 16) | (buf[0] << 8) | buf[1];
	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_NO_CMD,
				   SPI_MEM_OP_ADDR(3, cmd_sr_cr, 1),
				   SPI_MEM_OP_NO_MODE,
				   SPI_MEM_OP_NO_DUMMY,
				   SPI_MEM_OP_NO_DATA);

	/* The command line width is the same as the address line width. */
	op.addr.buswidth = spi_nor_get_protocol_inst_nbits(write_proto);

	return spi_mem_exec_op(&op, 0);
}
#else
static int spi_nor_write_reg(u8 opcode, u8 *buf, int len)
{
	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(opcode, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_MODE,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_OUT(len, NULL, 1));

	op.cmd.buswidth = spi_nor_get_protocol_inst_nbits(read_proto);
	op.data.buswidth = spi_nor_get_protocol_data_nbits(read_proto);

	return spi_nor_read_write_reg(&op, buf);
}
#endif

static ssize_t spi_nor_read_data(loff_t from, size_t len, u_char *buf, int async)
{
	u8 addr_width = spinor_4bytes_addr_mode ? 4 : 3;

	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_CMD(read_cmd, 1),
				   SPI_MEM_OP_ADDR(addr_width, from, 1),
				   SPI_MEM_OP_NO_MODE,
				   SPI_MEM_OP_DUMMY(read_dummy, 1),
				   SPI_MEM_OP_DATA_IN(len, buf, 1));
	size_t remaining = len;
	int ret;

	/* get transfer protocols. */
	op.cmd.buswidth = spi_nor_get_protocol_inst_nbits(read_proto);
	op.addr.buswidth = spi_nor_get_protocol_addr_nbits(read_proto);
	op.dummy.buswidth = op.addr.buswidth;
	op.data.buswidth = spi_nor_get_protocol_data_nbits(read_proto);

	while (remaining) {
		ret = spi_mem_exec_op(&op, async);
		if (ret)
			return ret;

		op.addr.val += op.data.nbytes;
		remaining -= op.data.nbytes;
		op.data.buf.in += op.data.nbytes;
	}

	return len;
}

static ssize_t spi_nor_io_read_data(loff_t from, size_t len, u_char *buf, int async)
{
	u8 addr_width = spinor_4bytes_addr_mode ? 4 : 3;
	u8 mode = 0;

	struct spi_mem_op op =
			SPI_MEM_OP(SPI_MEM_OP_CMD(read_cmd, 1),
				   SPI_MEM_OP_ADDR(addr_width, from, 1),
				   SPI_MEM_OP_MODE(&mode, 1),
				   SPI_MEM_OP_DUMMY(read_dummy, 1),
				   SPI_MEM_OP_DATA_IN(len, buf, 1));
	size_t remaining = len;
	int ret;

	/* get transfer protocols. */
	op.cmd.buswidth = spi_nor_get_protocol_inst_nbits(read_proto);
	op.addr.buswidth = spi_nor_get_protocol_addr_nbits(read_proto);
	op.mode.buswidth = op.addr.buswidth;
	op.dummy.buswidth = op.addr.buswidth;
	op.data.buswidth = spi_nor_get_protocol_data_nbits(read_proto);

	while (remaining) {
		ret = spi_mem_exec_op(&op, async);
		if (ret)
			return ret;

		op.addr.val += op.data.nbytes;
		remaining -= op.data.nbytes;
		op.data.buf.in += op.data.nbytes;
	}

	return len;
}

static int is_valid_read_cmd(u8 cmd)
{
	if (SPINOR_OP_READ_1_1_2 == cmd
		|| SPINOR_OP_READ_1_1_4 == cmd
		|| SPINOR_OP_READ4_1_1_2 == cmd
		|| SPINOR_OP_READ4_1_1_4 == cmd
		|| CMD_READ_ARRAY_FAST == cmd
		|| CMD_READ_ARRAY_FAST_4B == cmd
		|| CMD_READ_ARRAY_SLOW == cmd
		|| CMD_READ_ARRAY_SLOW_4B == cmd
		|| SPINOR_OP_READ_1_4_4 == cmd
		|| SPINOR_OP_READ_1_4_4_DTR == cmd
		|| SPINOR_OP_READ4_1_4_4 == cmd
		|| SPINOR_OP_READ4_1_4_4_DTR == cmd)
		return 1;
	return 0;
}

static int is_quad_read_cmd(u8 cmd)
{
	if (SPINOR_OP_READ_1_1_4 == cmd
		|| SPINOR_OP_READ4_1_1_4 == cmd
		|| SPINOR_OP_READ_1_4_4 == cmd
		|| SPINOR_OP_READ4_1_4_4 == cmd
		|| SPINOR_OP_READ_1_4_4_DTR == cmd
		|| SPINOR_OP_READ4_1_4_4_DTR == cmd)
		return 1;
	return 0;
}

static int read_sr(u8 *status)
{
	return spi_nor_read_reg(CMD_READ_STATUS, status, 1);
}

static int read_sr1(u8 *status)
{
	return spi_nor_read_reg(CMD_READ_STATUS1, status, 1);
}

static int spi_flash_ready(void)
{
	u8 sr;
	int ret;

	ret = read_sr(&sr);
	if (ret < 0)
		return ret;

	return !(sr & STATUS_WIP);
}

static int spi_flash_wait_till_ready(void)
{
	uint timeout = 0x40;
	int ret;

	while (timeout --) {
		ret = spi_flash_ready();
		if (ret < 0)
			return ret;
		if (ret)
			return 0;
	}

	printf("SF: Timeout!\n");
	return -1;
}

static int spi_flash_write_en(void)
{
	return spi_nor_write_reg(CMD_WRITE_ENABLE, NULL, 0);
}

static int write_sr(u8 status)
{
	spi_flash_write_en();

	if (spi_nor_write_reg(CMD_WRITE_STATUS, &status, 1))
		return -1;

	return spi_flash_wait_till_ready();

}

static int write_sr1(u8 status)
{
	spi_flash_write_en();

	if (spi_nor_write_reg(CMD_WRITE_STATUS1, &status, 1))
		return -1;

	return spi_flash_wait_till_ready();
}

static int read_cr(u8 *status)
{
	return spi_nor_read_reg(CMD_READ_CONFIG, status, 1);
}

static int write_cr(u8 wc)
{
	u8 dout[4];
	u8 status;
	int ret;

	ret = read_sr(&status);
	if (ret < 0)
		return ret;

	spi_flash_write_en();

	dout[0] = status;
	dout[1] = wc;

#ifdef CFG_SUNXI_SPIF
	if (spi_nor_write_reg_2byte(CMD_WRITE_STATUS, dout))
		return -1;
#else
	if (spi_nor_write_reg(CMD_WRITE_STATUS, dout, 2))
		return -1;
#endif

	return spi_flash_wait_till_ready();

}

/*Order code“05H”/“35H”/“15H”,The corresponding status registerS7~S0 / S15~S8 / S16~S23.*/
static int GigaDevice_quad_enable(void)
{
	u8 qeb_status;
	int ret;

	ret = read_sr1(&qeb_status);
	if (ret < 0)
		return ret;

	if (qeb_status & STATUS_QEB_GIGA) {
//		printf("already enable\n");
		return 0;
	}

	ret = write_sr1(qeb_status | STATUS_QEB_GIGA);
	if (ret < 0)
		return ret;

	/* read SR and check it */
	ret = read_sr1(&qeb_status);
	if (!(ret >= 0 && (qeb_status & STATUS_QEB_GIGA))) {
		printf("SF: Macronix SR Quad bit not clear\n");
		return -1;
	}
	return ret;
}

static int macronix_quad_enable(void)
{
	u8 qeb_status;
	int ret;

	ret = read_sr(&qeb_status);
	if (ret < 0)
		return ret;

	if (qeb_status & STATUS_QEB_MXIC) {
//		printf("already enable\n");
		return 0;
	}

	ret = write_sr(qeb_status | STATUS_QEB_MXIC);
	if (ret < 0)
		return ret;

	/* read SR and check it */
	ret = read_sr(&qeb_status);
	if (!(ret >= 0 && (qeb_status & STATUS_QEB_MXIC))) {
		printf("SF: Macronix SR Quad bit not clear\n");
		return -1;
	}
	return ret;
}

static int spansion_quad_enable(void)
{
	u8 qeb_status;
	int ret;

	ret = read_cr(&qeb_status);
	if (ret < 0)
		return ret;

	if (qeb_status & STATUS_QEB_WINSPAN)
		return 0;

	ret = write_cr(qeb_status | STATUS_QEB_WINSPAN);
	if (ret < 0)
		return ret;

	/* read CR and check it */
	ret = read_cr(&qeb_status);
	if (!(ret >= 0 && (qeb_status & STATUS_QEB_WINSPAN))) {
		printf("SF: Spansion CR Quad bit not clear\n");
		return -1;
	}

	return ret;
}

static int set_quad_mode(u8 id_0, u8 id_1)
{
	switch (JEDEC_MFR(id_0)) {

	case SPI_FLASH_CFI_MFR_MACRONIX:
	case SPI_FLASH_CFI_MFR_XMC:
		if (JEDEC_MFR(id_1) >> 4 == 'b') {
			printf("SF: QEB is volatile for %02xb flash\n", JEDEC_MFR(id_0));
			return 0;
		}
		return macronix_quad_enable();
	case SPI_FLASH_CFI_MFR_GIGADEVICE:
	case SPI_FLASH_CFI_MFR_ADESTO:
	case SPI_FLASH_CFI_MFR_PUYA:
	case SPI_FLASH_CFI_MFR_ZETTA:
	case SPI_FLASH_CFI_MFR_BOYA:
	case SPI_FLASH_CFI_MFR_ESMT:
		return GigaDevice_quad_enable();
	case SPI_FLASH_CFI_MFR_SPANSION:
	case SPI_FLASH_CFI_MFR_WINBOND:
	case SPI_FLASH_CFI_MFR_XTX:
	case SPI_FLASH_CFI_MFR_FM:
		return spansion_quad_enable();
	default:
		printf("SF: Need set QEB func for %02x flash\n",
		       JEDEC_MFR(id_0));
		return -1;
	}
}


static int spinor_read_id(u8 *id)
{
	return spi_nor_read_reg(CMD_READ_ID, id, 3);
}

static int spinor_enter_4bytes_addr(int enable)
{
	u8 command = 0;
	u8 buf = 0;

	if (enable)
		command = 0xB7;
	else
		command = 0xE9;

	spi_nor_write_reg(command, NULL, 0);

	read_cr(&buf);
	if ((buf >> 5)  & 0x1)
		printf("4byte mode ok\n");
	else {
		printf("4byte mode error\n");
		return 0;
	}
	return 1;
}

void spi_nor_set_4byte_opcodes(void)
{
	if (!spinor_info->addr4b_opcodes)
		spinor_enter_4bytes_addr(1);
	else {
		if (read_cmd == CMD_READ_ARRAY_FAST)
			read_cmd = CMD_READ_ARRAY_FAST_4B;
		else if (read_cmd == SPINOR_OP_READ_1_1_2)
			read_cmd = SPINOR_OP_READ4_1_1_2;
		else if (read_cmd == SPINOR_OP_READ_1_1_4)
			read_cmd = SPINOR_OP_READ4_1_1_4;
		else if (read_cmd == SPINOR_OP_READ_1_4_4)
			read_cmd = SPINOR_OP_READ4_1_4_4;
		else if (read_cmd == SPINOR_OP_READ_1_4_4_DTR)
			read_cmd = SPINOR_OP_READ_1_4_4_DTR;
	}
	spinor_4bytes_addr_mode = 1;/* set 4byte opcodes*/
	return;
}

static int spinor_set_readcmd(void)
{
	if (is_valid_read_cmd(spinor_info->readcmd)) {
		read_cmd = spinor_info->readcmd;
		read_proto = spinor_info->read_proto;
		write_proto = spinor_info->write_proto;
		read_dummy = spinor_info->read_dummy;
	} else
		return -1;

	if (spinor_info->flash_size > 16)
		spi_nor_set_4byte_opcodes();

	return 0;
}

int get_boot_param(void)
{
	boot_spinor_info_t *info = NULL;
	int ret = 0;
	struct sunxi_boot_param_region *boot_param =
				(struct sunxi_boot_param_region *)boot_param_cache;

	ret = spinor_read(sunxi_flashmap_bootparam_start(FLASHMAP_SPI_NOR),
			BOOT_PARAM_SIZE >> 9, (void *)boot_param);
	if (ret)
		goto err;

	if (!strncmp((const char *)boot_param->header.magic,
				(const char *)BOOT_PARAM_MAGIC,
				sizeof(boot_param->header.magic)))
		info = (boot_spinor_info_t *)boot_param->spiflash_info;

	if (info && !strncmp((const char *)info->magic,
			(const char *)SPINOR_BOOT_PARAM_MAGIC,
			sizeof(info->magic))) {
		spinor_info = info;
		return 0;
	}

	printf("boot param check error \n");
err:
	return -1;
}

void set_delay_para(void)
{
	if (spinor_info->frequency &&
		spinor_info->sample_delay != SAMP_MODE_DL_DEFAULT &&
		(spinor_info->sample_delay || spinor_info->sample_mode)) {
		printf("spi sample_mode:%x sample_delay:%x\n",
			spinor_info->sample_mode, spinor_info->sample_delay);
#ifdef CFG_SUNXI_SPIF
		spif_samp_mode((void *)SUNXI_SPIF_BASE, 1);
		spif_samp_dl_sw_rx_status((void *)SUNXI_SPIF_BASE, 1);
		spif_set_sample_mode((void *)SUNXI_SPIF_BASE,
				spinor_info->sample_mode);
		spif_set_sample_delay((void *)SUNXI_SPIF_BASE,
				spinor_info->sample_delay);
#else
		spi_samp_mode((void *)SUNXI_SPI0_BASE, 1);
		spi_samp_dl_sw_status((void *)SUNXI_SPI0_BASE, 1);
		spi_set_sample_mode((void *)SUNXI_SPI0_BASE,
				spinor_info->sample_mode);
		spi_set_sample_delay((void *)SUNXI_SPI0_BASE,
				spinor_info->sample_delay);
#endif
	}
	return ;
}

int spinor_init(int stage)
{
	u8 id[4] = {0};
	int ret = 0;
	spinor_info = NULL;
	read_dummy = 0;
	spinor_4bytes_addr_mode = 0;

#ifndef CFG_SUNXI_SBOOT
	spinor_info = (boot_spinor_info_t *)BT0_head.prvt_head.storage_data;
#endif

#ifdef CFG_SUNXI_SPIF
	if (spif_init())
		return -1;
#else
	if (spi_init())
		return -1;
#endif

	ret = spinor_read_id(id);
	if (ret || id[0] == 0x0) {
		printf("read id error, spinor id is: %02x %02x %02x\n",
				id[0], id[1], id[2]);
		return -1;
	}

	ret = get_boot_param();
	if (spinor_info && !ret && !spinor_set_readcmd()) {
#ifdef CFG_SUNXI_SPIF
		struct sunxi_spif_slave *sspif = get_sspif();
		sspif->max_hz = spinor_info->frequency;
		if (spi_nor_protocol_is_dtr(read_proto))
			sspif->rx_dtr_en = 1;
		if (spi_nor_protocol_is_dtr(write_proto))
			sspif->tx_dtr_en = 1;
		if (set_spif_clk()) {
#else
		if (set_spi_clk()) {
#endif
			printf("spi set clk error\n");
			return -1;
		} else
			set_delay_para();
#ifdef CFG_SOFT_ESTIMATE_SPIF_SPEED
		rx_bytes_per_ms = spinor_info->frequency / 1000;
		if (sspif->rx_dtr_en)
			rx_bytes_per_ms *= 2;
		if (spi_nor_get_protocol_data_nbits(read_proto) == 4)
			rx_bytes_per_ms /= 2;
		else
			rx_bytes_per_ms /= 8;
		/* factor 0.8 */
		rx_bytes_per_ms = rx_bytes_per_ms * 8 / 10;
		printf("rx_bytes_per_ms: %d\n", rx_bytes_per_ms);
#endif
	}
	dump_spinor_info(spinor_info);

	printf("spinor id is: %02x %02x %02x, read cmd: %02x\n",
		id[0], id[1], id[2], read_cmd);

	/*
	 * Flash powers up read-only, so clear BP# bits.
	 *
	 * Note on some flash (like Macronix), QE (quad enable) bit is in the
	 * same status register as BP# bits, and we need preserve its original
	 * value during a reboot cycle as this is required by some platforms
	 * (like Intel ICH SPI controller working under descriptor mode).
	 */
	if (JEDEC_MFR(id[0]) == SPI_FLASH_CFI_MFR_ATMEL ||
	   (JEDEC_MFR(id[0]) == SPI_FLASH_CFI_MFR_SST) ||
	   (JEDEC_MFR(id[0]) == SPI_FLASH_CFI_MFR_MACRONIX)) {
		u8 sr = 0;

		read_sr(&sr);
		if (JEDEC_MFR(id[0]) == SPI_FLASH_CFI_MFR_MACRONIX) {
			sr &= STATUS_QEB_MXIC;
		}
		write_sr(sr);
	}

	if (is_quad_read_cmd(read_cmd))
		set_quad_mode(id[0], id[1]);

	return 0;
}

int spinor_exit(int stage)
{
#ifdef CFG_SUNXI_SPIF
	spif_exit();
#else
	spi_exit();
#endif
	return 0;
}

static int _spinor_read(uint start, uint sector_cnt, void *buffer, int async)
{
	u32 addr;
	u8 *buf = (u8 *)buffer;
	u32 len = sector_cnt * 512;
	u32 from = start * 512;
	int ret;
	size_t rxnum = len;
	u_char *cache_buf;
	size_t cache_len = 0;

#ifdef CFG_SOFT_ESTIMATE_SPIF_SPEED
	if (async) {
		async_rx_start_ms = get_sys_ticks();
		async_rx_bytes = len;
	}
#endif

	while (len) {
		/*read 128*512 bytes per time*/
		addr = from;

		if (spi_nor_get_protocol_addr_nbits(read_proto) != 1) {
			if (rxnum % 4) {
				cache_len = (rxnum + 4) & ~0x3;
				cache_buf = malloc(cache_len);
				if (!cache_buf) {
					ret = -1;
					goto read_err;
				}
				ret = spi_nor_io_read_data(addr, cache_len, cache_buf, async);
				memcpy(buf, cache_buf, rxnum);
				free(cache_buf);
			} else
				ret = spi_nor_io_read_data(addr, rxnum, buf, async);
		} else {
			if (rxnum % 4) {
				cache_len = (rxnum + 4) & ~0x3;
				cache_buf = malloc(cache_len);
				if (!cache_buf) {
					ret = -1;
					goto read_err;
				}
				ret = spi_nor_read_data(addr, cache_len, cache_buf, async);
				memcpy(buf, cache_buf, rxnum);
				free(cache_buf);
			} else
				ret = spi_nor_read_data(addr, rxnum, buf, async);
		}
		if (ret == 0) {
			/* We shouldn't see 0-length reads */
			ret = -1;
			goto read_err;
		}
		if (ret < 0)
			goto read_err;

		if (rxnum % 4)
			ret = rxnum;

		buf += ret;
		from += ret;
		len -= ret;
	}
	ret = 0;

read_err:
	return ret;
}

int spinor_read(uint start, uint sector_cnt, void *buffer)
{
	return _spinor_read(start, sector_cnt, buffer, 0);
}

int spinor_read_async(uint start, uint sector_cnt, void *buffer)
{
	return _spinor_read(start, sector_cnt, buffer, 1);
}

int spinor_get_read_cnt(void)
{
#ifdef CFG_SOFT_ESTIMATE_SPIF_SPEED
	int kbytes, max;

	if (async_rx_start_ms) {
		max = async_rx_start_ms + (async_rx_bytes / async_rx_start_ms) + 1;
		kbytes = get_sys_ticks() - async_rx_start_ms;
		if (kbytes)
			kbytes--; /* reserved 1ms */
		if (kbytes >= max)
			return async_rx_bytes;
		kbytes *= rx_bytes_per_ms;
		/* printf("kbytes: %d\n", kbytes); */
		return kbytes;
	} else
		return -1;
#endif
#ifdef CFG_AMP_DELEG_SPIF_IRQ
	return spi_mem_read_cnt();
#endif
	return -1;
}
