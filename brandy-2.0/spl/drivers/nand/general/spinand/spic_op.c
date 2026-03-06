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
#include <common.h>
#include "spic.h"
#include "spinand.h"
#include "spinand_osal_boot0.h"

#define SPINAND_NOT_INIT  0
#define SPINAND_EARLY_INIT 1

#ifdef CFG_SUNXI_DMA
#include <arch/dma.h>
static ulong rx_dma_chan;
#endif

extern unsigned int def_freq;

extern __s32 BOOT_NandGetPara(boot_spinand_para_t *param, uint size);
__u32 SPIC_IO_BASE;

__s32 Wait_Tc_Complete(void)
{
	__u32 timeout = 0xffffff;

	/*wait transfer complete*/
	while (!(readl(SPI_ISR) & (0x1 << 12))) {
		timeout--;
		if (!timeout)
			break;
	}
	if (timeout == 0) {
		SPINAND_Print("wait xfer complete timeout!\n");
		return -ERR_TIMEOUT;
	}

	return 0;
}

static void spic0_set_sdm(unsigned int smod)
{
	unsigned int rval = readl(SPI_TCR) & (~(1 << 13));

	rval |= smod << 13;
	writel(rval, SPI_TCR);
}

static void spic0_set_sdc(unsigned int sample)
{
	unsigned int rval = readl(SPI_TCR) & (~(1 << 11));

	rval |= sample << 11;
	writel(rval, SPI_TCR);
}

static void spic0_set_sdc1(unsigned int sample)
{
	unsigned int rval = readl(SPI_TCR) & (~(1 << 15));

	rval |= sample << 15;
	writel(rval, SPI_TCR);
}

static void spic0_set_sample_mode(unsigned int mode)
{
	unsigned int sample_mode[7] = {
		DELAY_NORMAL_SAMPLE, DELAY_0_5_CYCLE_SAMPLE,
		DELAY_1_CYCLE_SAMPLE, DELAY_1_5_CYCLE_SAMPLE,
		DELAY_2_CYCLE_SAMPLE, DELAY_2_5_CYCLE_SAMPLE,
		DELAY_3_CYCLE_SAMPLE
	};
	spic0_set_sdm((sample_mode[mode] >> 8) & 0xf);
	spic0_set_sdc((sample_mode[mode] >> 4) & 0xf);
	spic0_set_sdc1(sample_mode[mode] & 0xf);
}

static void spic0_set_sample_delay(unsigned char sample_delay)
{
	unsigned int rval = readl(SPI_SDC)&(~(0x3f << 0));

	rval |= sample_delay;
	writel(rval, SPI_SDC);
}

static void spic0_samp_dl_sw_status(unsigned int status)
{
	unsigned int rval = readl(SPI_SDC);

	if (status)
		rval |= SPI_SAMP_DL_SW_EN;
	else
		rval &= ~SPI_SAMP_DL_SW_EN;

	writel(rval, SPI_SDC);
}

static void spic0_samp_mode(unsigned int status)
{
	unsigned int rval = readl(SPI_GCR);

	if (status)
		rval |= SPI_SAMP_MODE_EN;
	else
		rval &= ~SPI_SAMP_MODE_EN;

	writel(rval, SPI_GCR);
}

void spic0_sample_point_delay_set_legacy(unsigned int freq)
{
	unsigned int rval = 0;
	rval = readl(SPI_TCR);

	if (freq >= 60) {
		rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
		rval |= SPI_SAMPLE_CTRL;
	} else if (freq <= 24) {
		rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
		rval |= SPI_SAMPLE_MODE;
	} else {
		rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
	}

	writel(rval, SPI_TCR);
}

__s32 Spic_init(__u32 spi_no)
{
	__u32 rval;
	boot_spinand_para_t nand_info;
	static int early_init_flag = SPINAND_NOT_INIT;
	memset(&nand_info, 0, sizeof(boot_spinand_para_t));

	if (BOOT_NandGetPara(&nand_info, sizeof(boot_spinand_para_t)) != 0)
		return -1;

	if (early_init_flag == SPINAND_EARLY_INIT)
		goto skip_hardware_init;

#ifdef CFG_SUNXI_DMA
	sunxi_dma_init();
	rx_dma_chan = sunxi_dma_request(DMAC_DMATYPE_NORMAL);
#endif
	SPINAND_PIORequest(spi_no);
	SPIC_IO_BASE = SPINAND_GetIOBaseAddr();
	SPINAND_ClkRequest(spi_no);

	rval = SPI_SOFT_RST | SPI_TXPAUSE_EN | SPI_MASTER | SPI_ENABLE;
	writel(rval, SPI_GCR);

	writel(SPI_TXFIFO_RST | (SPI_TX_WL << 16) | (SPI_RX_WL), SPI_FCR);
	writel(SPI_ERROR_INT, SPI_IER);

skip_hardware_init:
	if (nand_info.FrequencePar)
		SPINAND_SetClk(spi_no, nand_info.FrequencePar);
	else
		SPINAND_SetClk(spi_no, def_freq);

	/*set ss to high,discard unused burst,SPI select signal
	 * polarity(low,1=idle)*/
	rval = SPI_SET_SS_1 | SPI_DHB | SPI_SS_ACTIVE0;
	printf("mode:0x%x sample delay:0x%x\n",
			nand_info.sample_mode, nand_info.sample_delay);
	if (nand_info.sample_delay == 0xaaaaffff || (nand_info.sample_delay == 0
				&& nand_info.sample_mode == 0)) {
#ifdef FPGA_PLATFORM
		nand_info.FrequencePar = 24;
		rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
#else
		if (nand_info.FrequencePar == 0)
			nand_info.FrequencePar = def_freq;
		if (nand_info.FrequencePar >= 60) {
			rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
			rval |= SPI_SAMPLE_CTRL;
		} else if (nand_info.FrequencePar <= 24) {
			rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
			rval |= SPI_SAMPLE_MODE;
		} else
			rval &= ~(SPI_SAMPLE_CTRL | SPI_SAMPLE_MODE);
#endif
	} else {
		spic0_samp_mode(1);
		spic0_samp_dl_sw_status(1);
		spic0_set_sample_mode(nand_info.sample_mode);
		spic0_set_sample_delay(nand_info.sample_delay);
	}
	writel(rval | readl(SPI_TCR), SPI_TCR);

	early_init_flag = SPINAND_EARLY_INIT;
	return 0;
}

__s32 Spic_exit(__u32 spi_no)
{
	__u32 rval;

	rval = readl(SPI_GCR);
	rval &= (~(SPI_SOFT_RST | SPI_MASTER | SPI_ENABLE));
	writel(rval, SPI_GCR);

	SPINAND_ClkRelease(spi_no);

	SPINAND_PIORelease(spi_no);

	/* set ss to high,discard unused burst,SPI select signal
	  * polarity(low,1=idle) */
	rval = SPI_SET_SS_1 | SPI_DHB | SPI_SS_ACTIVE0;
	writel(rval, SPI_TCR);

#ifdef CFG_SUNXI_DMA
	sunxi_dma_release(rx_dma_chan);
	sunxi_dma_exit();
#endif
	return 0;
}

void Spic_sel_ss(__u32 spi_no, __u32 ssx)
{
	__u32 rval = readl(SPI_TCR) & (~(3 << 4));
	rval |= ssx << 4;
	writel(rval, SPI_TCR);
}

void Spic_config_dual_mode(__u32 spi_no, __u32 rxdual, __u32 dbc, __u32 stc)
{
	writel((rxdual << 28) | (dbc << 24) | (stc), SPI_BCC);
}


/*
 * spi txrx
 * _ _______ ______________
 *  |_______|/_/_/_/_/_/_/_|
 */
static __s32 xfer_by_cpu(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
	__u32 i = 0, fcr;

	writel(0, SPI_IER);
	writel(0xffffffff, SPI_ISR); /*clear status register*/

	writel(tcnt, SPI_MTC);
	writel(tcnt + rcnt + dummy_cnt, SPI_MBC);

	/*read and write by cpu operation*/
	if (tcnt) {
		i = 0;
		while (i < tcnt) {
			if (((readl(SPI_FSR) >> 16) & 0x7f) == SPI_FIFO_SIZE)
				SPINAND_Print("TX FIFO size error!\n");
			writeb(*(txbuf + i), SPI_TXD);
			i++;
		}
	}
	/* start transmit */
	writel(readl(SPI_TCR) | SPI_EXCHANGE, SPI_TCR);
	if (rcnt) {
		i = 0;
		while (i < rcnt) {
			/*receive valid data*/
			while (((readl(SPI_FSR)) & 0xff) == 0)
				;
			*(rxbuf + i) = readb(SPI_RXD);
			i++;
		}
	}

	if (Wait_Tc_Complete()) {
		SPINAND_Print("wait tc complete timeout!\n");
		return -ERR_TIMEOUT;
	}

	fcr = readl(SPI_FCR);
	fcr &= ~(SPI_TXDMAREQ_EN | SPI_RXDMAREQ_EN);
	writel(fcr, SPI_FCR);
	/* (1U << 11) | (1U << 10) | (1U << 9) | (1U << 8)) */
	if (readl(SPI_ISR) & (0xf << 8)) {
		SPINAND_Print("FIFO status error: 0x%x!\n", readl(SPI_ISR));
		return NAND_OP_FALSE;
	}

	if (readl(SPI_TCR) & SPI_EXCHANGE) {
		SPINAND_Print("XCH Control Error!!\n");
	}

	writel(0xffffffff, SPI_ISR); /* clear  flag */
	return NAND_OP_TRUE;
}

int spic0_dma_start(unsigned int tx_mode, unsigned int addr, unsigned length)
{
	int ret = 0;
	sunxi_dma_set dma_set;

	dma_set.loop_mode       = 0;
	dma_set.wait_cyc	= 32;
	dma_set.data_block_size = 0;

	if (!tx_mode) {
		dma_set.channal_cfg.src_drq_type     = DMAC_CFG_TYPE_SPI0;
		dma_set.channal_cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;
		dma_set.channal_cfg.src_burst_length = DMAC_CFG_SRC_8_BURST;
		dma_set.channal_cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_32BIT;
		dma_set.channal_cfg.reserved0	= 0;

		dma_set.channal_cfg.dst_drq_type     = DMAC_CFG_TYPE_DRAM;
		dma_set.channal_cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE;
		dma_set.channal_cfg.dst_burst_length = DMAC_CFG_DEST_8_BURST;
		dma_set.channal_cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_32BIT;
		dma_set.channal_cfg.reserved1	= 0;
	}

	if (!tx_mode) {
		ret = sunxi_dma_setting(rx_dma_chan, &dma_set);
		if (ret < 0) {
			SPINAND_Print("rx dma set fail\n");
			return -1;
		}
	}

	/* cache is disabled in spi0 */
	if (!tx_mode) {
		/* cache is disabled in boot0, needn`t flush cache */
		ret = sunxi_dma_start(rx_dma_chan, (__u32)SPI_RXD, addr, length);
	}
	if (ret < 0) {
		SPINAND_Print("rx dma start fail\n");
		return -1;
	}

	return 0;
}

static int spic0_wait_dma_finish(unsigned int tx_flag, unsigned int rx_flag)
{
	__u32 timeout = 0xffffff;

	if (rx_flag) {
		timeout = 0xffffff;
		while (sunxi_dma_querystatus(rx_dma_chan)) {
			timeout--;
			if (!timeout)
				break;
		}

		if (timeout <= 0) {
			SPINAND_Print("RX DMA wait status timeout!\n");
			return -ERR_TIMEOUT;
		}
	}

	return 0;
}
static __s32 xfer_by_dma(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
	__u32 i		  = 0, fcr;
	__u32 tx_dma_flag = 0;
	__u32 rx_dma_flag = 0;

	//printf("dma tcnt:%d rcnt:%d dummy:%d \n", tcnt, rcnt, dummy_cnt);

	writel(0, SPI_IER);
	writel(0xffffffff, SPI_ISR); /*clear status register*/

	writel(tcnt, SPI_MTC);
	writel(tcnt + rcnt + dummy_cnt, SPI_MBC);

	/*read and write by cpu operation*/
	if (tcnt) {
		i = 0;
		while (i < tcnt) {
			if (((readl(SPI_FSR) >> 16) & 0x7f) == SPI_FIFO_SIZE)
				SPINAND_Print("TX FIFO size error!\n");
			writeb(*(txbuf + i), SPI_TXD);
			i++;
		}
	}
	/* start transmit */
	writel(readl(SPI_TCR) | SPI_EXCHANGE, SPI_TCR);
	if (rcnt) {
		if (rcnt <= 64) {
			i = 0;

			while (i < rcnt) {
				/*receive valid data*/
				while (((readl(SPI_FSR)) & 0x7f) == 0)
					;
				*(rxbuf + i) = readb(SPI_RXD);
				i++;
			}
		} else {
			rx_dma_flag = 1;
			writel((readl(SPI_FCR) | SPI_RXDMAREQ_EN), SPI_FCR);
			spic0_dma_start(0, (unsigned int)rxbuf, rcnt);
		}
	}

	if (spic0_wait_dma_finish(tx_dma_flag, rx_dma_flag)) {
		SPINAND_Print("DMA wait status timeout!\n");
		return -ERR_TIMEOUT;
	}

	if (Wait_Tc_Complete()) {
		SPINAND_Print("wait tc complete timeout!\n");
		return -ERR_TIMEOUT;
	}

#ifdef CFG_USE_DCACHE
	if (rcnt > 64)
		invalidate_dcache_range((unsigned long)rxbuf, (unsigned long)rxbuf + rcnt);
#endif

	fcr = readl(SPI_FCR);
	fcr &= ~(SPI_TXDMAREQ_EN | SPI_RXDMAREQ_EN);
	writel(fcr, SPI_FCR);
	/* (1U << 11) | (1U << 10) | (1U << 9) | (1U << 8)) */
	if (readl(SPI_ISR) & (0xf << 8)) {
		SPINAND_Print("FIFO status error: 0x%x!\n", readl(SPI_ISR));
		return NAND_OP_FALSE;
	}

	if (readl(SPI_TCR) & SPI_EXCHANGE) {
		SPINAND_Print("XCH Control Error!!\n");
	}

	writel(0xffffffff, SPI_ISR); /* clear  flag */
	return NAND_OP_TRUE;
}

/*
 * spi txrx
 * _ _______ ______________
 *  |_______|/_/_/_/_/_/_/_|
 */
__s32 Spic_rw(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
#ifndef CFG_SUNXI_DMA
	return xfer_by_cpu(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
#else
#ifdef CFG_SUNXI_BOOT_PARAM
	/* When reading boot_param, DDR may not be initialized, so cannot move data using dma. */
	if (flash_disable_dma)
		return xfer_by_cpu(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
	else
		return xfer_by_dma(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
#else
	return xfer_by_dma(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
#endif
#endif
}
