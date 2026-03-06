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
 * (C) Copyright 2022-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * lujianliang <lujianliang@allwinnertech.com>
 */

#include <arch/spi.h>
#include <arch/spi-mem.h>
#include <linux/errno.h>
#include <arch/rtc.h>
#include "spif-sunxi.h"

#ifdef CFG_SUNXI_SPIF

#define CONFIG_SUNXI_SPIF_V0	0x10000
#define CONFIG_SUNXI_SPIF_V1	0x10001
#define CONFIG_SUNXI_SPIF_V2	0x10002
//extern void spif_print_descriptor(struct spif_descriptor_op *spif_op);

static int spif_select_buswidth(u32 buswidth)
{
	int width = 0;

	switch (buswidth) {
	case SPIF_SINGLE_MODE:
		width = 0;
		break;
	case SPIF_DUEL_MODE:
		width = 1;
		break;
	case SPIF_QUAD_MODE:
		width = 2;
		break;
	case SPIF_OCTAL_MODE:
		width = 3;
		break;
	default:
		printf("Parameter error with buswidth:%d\n", buswidth);
	}
	return width;
}

void *malloc_align(unsigned int len, unsigned int align, void **buf)
{
	unsigned int alignd;

	if (!buf)
		return NULL;

	*buf = malloc(len + align - 1);
	if (!*buf)
		return NULL;
	alignd = ((unsigned int)(*buf) + align - 1) &  ~(align - 1);

	//printf("malloc len:%d addr:%x align:%x\n", (len + align - 1), (unsigned int)buf, alignd);
	return (void *)alignd;
}

int spif_mem_exec_op_v0(const struct spi_mem_op *op, int async)
{
	int ret;
	void *desc_cache = NULL;
	struct spif_descriptor_op *spif_op;
	uint cache_buf[CONFIG_SYS_CACHELINE_SIZE] __aligned(CONFIG_SYS_CACHELINE_SIZE);
	uint desc_count = ((op->data.nbytes + SPIF_MAX_TRANS_NUM - 1) / SPIF_MAX_TRANS_NUM) + 1;
	uint desc_size = desc_count * sizeof(struct spif_descriptor_op);

	spif_op = malloc_align(desc_size, CONFIG_SYS_CACHELINE_SIZE, desc_cache);
	if (!spif_op) {
		printf("malloc align error");
		return -1;
	}

	memset(spif_op, 0, desc_size);
	memset(cache_buf, 0, CONFIG_SYS_CACHELINE_SIZE);

	/* set hburst type */
	spif_op->hburst_rw_flag &= ~HBURST_TYPE;
	spif_op->hburst_rw_flag |= HBURST_INCR16_TYPE;

	/* the last one descriptor */
	spif_op->hburst_rw_flag |= DMA_FINISH_FLASG;

	/* set DMA block len mode */
	spif_op->block_data_len &= ~DMA_BLK_LEN;
	spif_op->block_data_len |= DMA_BLK_LEN_64B;

	spif_op->addr_dummy_data_count |= SPIF_DES_NORMAL_EN;

	/* dispose cmd */
	if (op->cmd.opcode) {
		spif_op->trans_phase |= SPIF_CMD_TRANS_EN;
		spif_op->cmd_mode_buswidth |= op->cmd.opcode << SPIF_CMD_OPCODE_POS;
		/* set cmd buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->cmd.buswidth) << SPIF_CMD_TRANS_POS;
		if (op->cmd.buswidth != 1)
			spif_op->cmd_mode_buswidth |=
				spif_select_buswidth(op->cmd.buswidth) <<
				SPIF_DATA_TRANS_POS;
	}

	/* dispose addr */
	if (op->addr.nbytes) {
		spif_op->trans_phase |= SPIF_ADDR_TRANS_EN;
		spif_op->flash_addr = op->addr.val;
		if (op->addr.nbytes == 4) {//set 4byte addr mode
			spif_op->addr_dummy_data_count &= SPIF_ADDR_SIZE_MASK;
			spif_op->addr_dummy_data_count |= SPIF_ADDR_SIZE_32BIT;
		}
		/* set addr buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->addr.buswidth) <<
			SPIF_ADDR_TRANS_POS;
	}

	/* dispose mode */
	if (op->mode.val) {
		spif_op->trans_phase |= SPIF_MODE_TRANS_EN;
		spif_op->cmd_mode_buswidth |=
				*(u8 *)op->mode.val << SPIF_MODE_OPCODE_POS;
		/* set addr buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->mode.buswidth) <<
			SPIF_MODE_TRANS_POS;
	}

	/* dispose dummy */
	if (op->dummy.cycle) {
		spif_op->trans_phase |= SPIF_DUMMY_TRANS_EN;
		spif_op->addr_dummy_data_count |=
			(op->dummy.cycle << SPIF_DUMMY_NUM_POS);
	}

	/* dispose data */
	if (op->data.nbytes) {
		/* set data buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->data.buswidth) << SPIF_DATA_TRANS_POS;

		if (op->data.dir == SPI_MEM_DATA_IN) {
			spif_op->trans_phase |= SPIF_RX_TRANS_EN;
			if (op->data.nbytes < SPIF_MIN_TRANS_NUM)
				spif_op->data_addr = (u32)cache_buf;
			else
				spif_op->data_addr = (u32)op->data.buf.in;
			/* Write:1 DMA Write to dram */
			spif_op->hburst_rw_flag |= DMA_RW_PROCESS;
		} else {
			spif_op->trans_phase |= SPIF_TX_TRANS_EN;
			spif_op->data_addr = (u32)op->data.buf.out;
			/* Read:0 DMA read for dram */
			spif_op->hburst_rw_flag &= ~DMA_RW_PROCESS;
		}
		if (op->data.nbytes < SPIF_MIN_TRANS_NUM &&
				op->data.dir == SPI_MEM_DATA_IN) {
			spif_op->addr_dummy_data_count |=
				SPIF_MIN_TRANS_NUM << SPIF_DATA_NUM_POS;
			spif_op->block_data_len |=
				SPIF_MIN_TRANS_NUM << SPIF_DATA_NUM_POS;
		} else if (op->data.nbytes <= SPIF_MAX_TRANS_NUM) {
			spif_op->addr_dummy_data_count |=
				op->data.nbytes << SPIF_DATA_NUM_POS;
			spif_op->block_data_len |=
				op->data.nbytes << SPIF_DATA_NUM_POS;
		} else {
			unsigned int total_len = op->data.nbytes;
			struct spif_descriptor_op *current_op = spif_op;

			spif_op->addr_dummy_data_count |=
				SPIF_MAX_TRANS_NUM << SPIF_DATA_NUM_POS;
			spif_op->block_data_len |=
				SPIF_MAX_TRANS_NUM << SPIF_DATA_NUM_POS;
			spif_op->hburst_rw_flag &= ~DMA_FINISH_FLASG;
			total_len -= SPIF_MAX_TRANS_NUM;

			while (total_len) {
				struct spif_descriptor_op *next_op = current_op + 1;

				memcpy(next_op, current_op, sizeof(struct spif_descriptor_op));

				current_op->next_des_addr = (unsigned int)next_op;
				//spif_print_descriptor(current_op);

				next_op->addr_dummy_data_count &= ~DMA_DATA_LEN;
				next_op->block_data_len &= ~DMA_DATA_LEN;

				if (total_len > SPIF_MAX_TRANS_NUM) {
					next_op->addr_dummy_data_count |=
						SPIF_MAX_TRANS_NUM << SPIF_DATA_NUM_POS;
					next_op->block_data_len |=
						SPIF_MAX_TRANS_NUM << SPIF_DATA_NUM_POS;
					total_len -= SPIF_MAX_TRANS_NUM;
				} else {
					next_op->addr_dummy_data_count |=
						total_len << SPIF_DATA_NUM_POS;
					next_op->block_data_len |=
						total_len << SPIF_DATA_NUM_POS;
					next_op->hburst_rw_flag |= DMA_FINISH_FLASG;
					next_op->next_des_addr = 0;
					total_len = 0;
				}

				next_op->data_addr =
					current_op->data_addr + SPIF_MAX_TRANS_NUM;
				next_op->flash_addr =
					current_op->flash_addr + SPIF_MAX_TRANS_NUM;
				current_op = next_op;
			}
			//spif_print_descriptor(current_op);
		}
	}

	ret = spif_xfer(spif_op, op->data.nbytes, async);

	if (op->data.nbytes < SPIF_MIN_TRANS_NUM &&
			op->data.dir == SPI_MEM_DATA_IN)
		memcpy((void *)op->data.buf.in,	(const void *)cache_buf,
					op->data.nbytes);
	if (desc_cache)
		free(desc_cache);
	return ret;
}

static void *spif_op_buffer;
static void *spif_op_buffer_aligned;
static u32 spif_op_buffer_size;

#define DEFAULT_SPIF_OP_BUFFER_SZ		(((16 * 1024 * 1024) / (SPIF_MAX_TRANS_NUM)) * sizeof(struct spif_descriptor_op))

int spif_mem_exec_op_v1(const struct spi_mem_op *op, int async)
{
	struct spif_descriptor_op *spif_op = NULL;
	struct spif_descriptor_op *current_op = NULL;
	uint desc_count = ((op->data.nbytes + SPIF_MAX_TRANS_NUM - 1) / SPIF_MAX_TRANS_NUM) + 1;
	uint desc_size = desc_count * sizeof(struct spif_descriptor_op);
	size_t remain_len = op->data.nbytes, handle_len = 0;
	int ret;
	void *cache_buf_align = NULL, *cache_buf = NULL;

	if (!spif_op_buffer) {
		spif_op_buffer_size = DEFAULT_SPIF_OP_BUFFER_SZ;
		spif_op_buffer_aligned = malloc_align(spif_op_buffer_size, CONFIG_SYS_CACHELINE_SIZE, &spif_op_buffer);
		if (!spif_op_buffer_aligned) {
			printf("malloc align error");
			return -1;
		}
	}

	if (desc_size > spif_op_buffer_size) {
		spif_op_buffer_size = desc_size;
		free(spif_op_buffer);
		spif_op_buffer_aligned = malloc_align(spif_op_buffer_size, CONFIG_SYS_CACHELINE_SIZE, &spif_op_buffer);
		if (!spif_op_buffer_aligned) {
			printf("malloc align error");
			return -1;
		}
	}

	spif_op = (typeof(spif_op))(spif_op_buffer_aligned);
	current_op = spif_op;

	memset(spif_op, 0, sizeof(struct spif_descriptor_op));

	/* set hburst type */
	spif_op->hburst_rw_flag &= ~HBURST_TYPE;
	spif_op->hburst_rw_flag |= HBURST_INCR16_TYPE;

	/* set DMA block len mode */
	spif_op->block_data_len &= ~DMA_BLK_LEN;
	spif_op->block_data_len |= DMA_BLK_LEN_64B;

	spif_op->addr_dummy_data_count |= SPIF_DES_NORMAL_EN;

	/* dispose cmd */
	if (op->cmd.opcode) {
		spif_op->trans_phase |= SPIF_CMD_TRANS_EN;
		spif_op->cmd_mode_buswidth |= op->cmd.opcode << SPIF_CMD_OPCODE_POS;
		/* set cmd buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->cmd.buswidth) << SPIF_CMD_TRANS_POS;
		if (op->cmd.buswidth != 1)
			spif_op->cmd_mode_buswidth |=
				spif_select_buswidth(op->cmd.buswidth) <<
				SPIF_DATA_TRANS_POS;
	}

	/* dispose addr */
	if (op->addr.nbytes) {
		spif_op->trans_phase |= SPIF_ADDR_TRANS_EN;
		spif_op->flash_addr = op->addr.val;
		if (op->addr.nbytes == 4) {//set 4byte addr mode
			if (sunxi_spif_get_version() >= CONFIG_SUNXI_SPIF_V2) {
				spif_op->addr_dummy_data_count &= ~SPIF_ADDR_SIZE_MASK_V2;
				spif_op->addr_dummy_data_count |= SPIF_ADDR_SIZE_32BIT_V2;
			} else {
				spif_op->addr_dummy_data_count &= ~SPIF_ADDR_SIZE_MASK;
				spif_op->addr_dummy_data_count |= SPIF_ADDR_SIZE_32BIT;
			}
		} else {
			if (sunxi_spif_get_version() >= CONFIG_SUNXI_SPIF_V2) {
				spif_op->addr_dummy_data_count &= ~SPIF_ADDR_SIZE_MASK_V2;
				spif_op->addr_dummy_data_count |= SPIF_ADDR_SIZE_24BIT_V2;
			} else {
				spif_op->addr_dummy_data_count &= ~SPIF_ADDR_SIZE_MASK;
				spif_op->addr_dummy_data_count |= SPIF_ADDR_SIZE_24BIT;
			}
		}
		/* set addr buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->addr.buswidth) <<
			SPIF_ADDR_TRANS_POS;
	}

	/* dispose mode */
	if (op->mode.val) {
		spif_op->trans_phase |= SPIF_MODE_TRANS_EN;
		spif_op->cmd_mode_buswidth |=
				*(u8 *)op->mode.val << SPIF_MODE_OPCODE_POS;
		/* set addr buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->mode.buswidth) <<
			SPIF_MODE_TRANS_POS;
	}

	/* dispose dummy */
	if (op->dummy.cycle) {
		spif_op->trans_phase |= SPIF_DUMMY_TRANS_EN;
		spif_op->addr_dummy_data_count |=
			(op->dummy.cycle << SPIF_DUMMY_NUM_POS);
	}

	/* dispose data */
	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN) {
			spif_op->trans_phase |= SPIF_RX_TRANS_EN;
			if ((u32)op->data.buf.in % CONFIG_SYS_CACHELINE_SIZE) {
				if (async) {
					pr_emerg("unsupport unaliged async spif read\n");
					return -1;
				}
				cache_buf_align = malloc_align(op->data.nbytes,
						CONFIG_SYS_CACHELINE_SIZE, &cache_buf);
				spif_op->data_addr = (u32)cache_buf_align;
			} else
				spif_op->data_addr = (u32)op->data.buf.in;
			/* Read Flash:1 DMA Write to dram */
			spif_op->hburst_rw_flag |= DMA_RW_PROCESS;
		} else {
			spif_op->trans_phase |= SPIF_TX_TRANS_EN;
			if ((u32)op->data.buf.out % CONFIG_SYS_CACHELINE_SIZE) {
				if (async) {
					pr_emerg("unsupport unaliged async spif read\n");
					return -1;
				}
				cache_buf_align = malloc_align(op->data.nbytes,
						CONFIG_SYS_CACHELINE_SIZE, &cache_buf);
				memcpy(cache_buf_align, op->data.buf.out, op->data.nbytes);
				spif_op->data_addr = (u32)cache_buf_align;

			} else
				spif_op->data_addr = (u32)op->data.buf.out;
			/* Write Flash:0 DMA read for dram */
			spif_op->hburst_rw_flag &= ~DMA_RW_PROCESS;
		}

		/* addr word alignment */
		spif_op->data_addr = spif_op->data_addr >> 2;

		/* set data buswidth */
		spif_op->cmd_mode_buswidth |=
			spif_select_buswidth(op->data.buswidth) << SPIF_DATA_TRANS_POS;

		handle_len = min_t(size_t, SPIF_MAX_TRANS_NUM, remain_len);

		spif_op->block_data_len |=
			handle_len << SPIF_DATA_NUM_POS;
		spif_op->addr_dummy_data_count |= handle_len == SPIF_MAX_TRANS_NUM ?
			DMA_TRANS_NUM_16BIT : handle_len << SPIF_DATA_NUM_POS;

		remain_len = op->data.nbytes - handle_len;
		while (remain_len) {
			struct spif_descriptor_op *next_op = current_op + 1;

			memcpy(next_op, current_op, sizeof(struct spif_descriptor_op));

			next_op->flash_addr += SPIF_MAX_TRANS_NUM;
			next_op->data_addr += (SPIF_MAX_TRANS_NUM >> 2);

			handle_len = min_t(size_t, SPIF_MAX_TRANS_NUM, remain_len);

			next_op->block_data_len &= ~DMA_DATA_LEN;
			next_op->addr_dummy_data_count &=
				~(DMA_TRANS_NUM_16BIT | DMA_TRANS_NUM);

			next_op->block_data_len |=
				handle_len << SPIF_DATA_NUM_POS;
			next_op->addr_dummy_data_count |=
				handle_len == SPIF_MAX_TRANS_NUM ?
				DMA_TRANS_NUM_16BIT : (handle_len << SPIF_DATA_NUM_POS);

			/* set next des addr */
			current_op->next_des_addr = (u32)next_op >> 2;
#ifdef CFG_AMP_DELEG_SPIF_IRQ
			if (async)
				current_op->hburst_rw_flag |= DMA_FINISH_FLASG;
#endif

			remain_len -= handle_len;

			current_op = next_op;
		}
		current_op->next_des_addr = 0;
		current_op->hburst_rw_flag |= DMA_FINISH_FLASG;
	}
#ifdef CFG_AMP_DELEG_SPIF_IRQ
	if (async) {
		u32 timeout = 0x3fffffff;
		rtc_clear_data(CFG_SPIF_RECORD_RTC_IDX);
		record_time("wait spif irq");
		while (!(rtc_read_data(CFG_DELEG_SPIF_IRQ_RTC_IDX) &
					(1 << CFG_SPIF_IRQ_RDY_RTC_BIT))) {
			timeout--;
			if (!timeout) {
				pr_emerg("Wait For SPIF IRQ Ready time_out\n");
				return -1;
			}
		}
		record_time("spif_irq ready");
	}
#endif

	ret = spif_xfer(spif_op, op->data.nbytes, async);

	if (op->data.dir == SPI_MEM_DATA_IN &&
			(u32)op->data.buf.in % CONFIG_SYS_CACHELINE_SIZE)
		memcpy((void *)op->data.buf.in,	(const void *)cache_buf_align,
					op->data.nbytes);

	if (cache_buf)
		free(cache_buf);

	return ret;
}
#endif /* CFG_SUNXI_SPIF */

int spi_mem_exec_op(const struct spi_mem_op *op, int async)
{
#ifdef CFG_SUNXI_SPIF
	if (sunxi_spif_get_version() == CONFIG_SUNXI_SPIF_V0)
		return spif_mem_exec_op_v0(op, async);
	else
		return spif_mem_exec_op_v1(op, async);
#else
	unsigned int pos = 0;
	const u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;
	u8 *op_buf;
	int op_len;
	int ret;
	int i;
	/* convert the dummy cycles to the number of bytes */
	int dummy_nbytes = (op->dummy.cycle * op->dummy.buswidth) / 8;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx_buf = op->data.buf.in;
		else
			tx_buf = op->data.buf.out;
	}

	op_len = sizeof(op->cmd.opcode) + op->addr.nbytes + dummy_nbytes;
	if (tx_buf)
		op_buf = malloc(op_len + op->data.nbytes);
	else
		op_buf = malloc(op_len);

	if (!op_buf) {
		return -1;
	}

	op_buf[pos++] = op->cmd.opcode;

	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			op_buf[pos + i] = op->addr.val >>
				(8 * (op->addr.nbytes - i - 1));

		pos += op->addr.nbytes;
	}

	if (op->dummy.cycle)
		memset(op_buf + pos, 0xff, dummy_nbytes);

	if (tx_buf) {
		op_len += op->data.nbytes;
		memcpy(op_buf, tx_buf, op->data.nbytes);
		ret = spi_xfer(op->data.nbytes, op_buf, 0, NULL);
		if (ret)
			goto free_op_buf;
	} else {
		ret = spi_xfer(op_len, op_buf, op->data.nbytes, rx_buf);
		if (ret)
			goto free_op_buf;
	}


free_op_buf:
	free(op_buf);

	if (ret < 0)
		return ret;

	return 0;
#endif  /* CFG_SUNXI_SPIF */
}

int spi_mem_read_cnt(void)
{
#ifdef CFG_AMP_DELEG_SPIF_IRQ
#define SPIF_TRANS_ERR				(1 << 31)
#define SPIF_TRANS_DONE				(1 << 30)

	u32 val = rtc_read_data(CFG_SPIF_RECORD_RTC_IDX);

	if (val & SPIF_TRANS_DONE) {
		if (val != SPIF_TRANS_DONE) {
			val &= ~(SPIF_TRANS_DONE);
			rtc_write_data(CFG_SPIF_RECORD_RTC_IDX, SPIF_TRANS_DONE);
			return (int)val; /* transmit error */
		} else
			return -ENODATA; /* transmit done */
	} else if (val & SPIF_TRANS_ERR) {
		val &= ~(SPIF_TRANS_DONE);
		return (int)val; /* transmit error */
	} else {
		return val;
	}
#else
	return -ENOSYS;
#endif
}
