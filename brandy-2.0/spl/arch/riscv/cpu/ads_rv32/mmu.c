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

#include <common.h>
#include <asm/io.h>
#include <asm/csr.h>
#include <sunxi-chips.h>

#define BIT(nr)						(1UL << (nr))

#define L1_CACHE_BYTES					(64)

#define CSR_MCACHE_CTL					0x7ca
/* MCACHE control register */
#define V5_MCACHE_CTL_DC_EN_OFFSET			1
#define V5_MCACHE_CTL_DC_EN				BIT(V5_MCACHE_CTL_DC_EN_OFFSET)

/* nds cctl command */
#define V5_UCCTL_L1D_VA_INVAL				0
#define V5_UCCTL_L1D_VA_WB				1
#define V5_UCCTL_L1D_VA_WBINVAL				2
#define V5_UCCTL_L1D_WBINVAL_ALL			6
#define V5_UCCTL_L1D_WB_ALL				7
#define V5_UCCTL_L1D_INVAL_ALL				23

/***********************************
 *  * AndeStar V5 user mode CSRs
 *   **********************************/
#define CSR_UCCTLBEGINADDR	0x80b
#define CSR_UCCTLCOMMAND	0x80c

#define ANDES_L2_BASE		(0x49000000)
#define MAX_CACHE_LINE_SIZE	256

/* L2 cache registers */
#define L2C_REG_CFG_OFFSET	0
#define L2C_REG_CTL_OFFSET	0x8
#define L2C_HPM_C0_CTL_OFFSET	0x10
#define L2C_HPM_C1_CTL_OFFSET	0x18
#define L2C_HPM_C2_CTL_OFFSET	0x20
#define L2C_HPM_C3_CTL_OFFSET	0x28
#define L2C_REG_C0_CMD_OFFSET	0x40
#define L2C_REG_C0_ACC_OFFSET	0x48
#define L2C_REG_C1_CMD_OFFSET	0x50
#define L2C_REG_C1_ACC_OFFSET	0x58
#define L2C_REG_C2_CMD_OFFSET	0x60
#define L2C_REG_C2_ACC_OFFSET	0x68
#define L2C_REG_C3_CMD_OFFSET	0x70
#define L2C_REG_C3_ACC_OFFSET	0x78
#define L2C_REG_C0_STATUS_OFFSET	0x80
#define L2C_REG_C0_HPM_OFFSET	0x200

/* L2 Cache Control */
#define L2_CONTROL_TRAMICTL				BIT(10)
#define L2_CONTROL_DRAMICTL				BIT(13)
#define L2_CONTROL_CEN					BIT(0)

#define L2_CONTROL_IPFDPT_OFFSET			3
#define L2_CONTROL_IPFDPT_MASK				(0x3 << L2_CONTROL_IPFDPT_OFFSET)
#define L2_CONTROL_IPFDPT_3_REQ				(0x3 << L2_CONTROL_IPFDPT_OFFSET)

#define L2_CONTROL_DPFDPT_OFFSET			5
#define L2_CONTROL_DPFDPT_MASK				(0x3 << L2_CONTROL_DPFDPT_OFFSET)
#define L2_CONTROL_DPFDPT_8_REQ				(0x3 << L2_CONTROL_DPFDPT_OFFSET)

/* L2 CCTL status */
#define CCTL_L2_STATUS_IDLE	0
#define CCTL_L2_STATUS_PROCESS	1
#define CCTL_L2_STATUS_ILLEGAL	2
/* L2 CCTL status cores mask */
#define CCTL_L2_STATUS_C0_MASK	0xF
#define CCTL_L2_STATUS_C1_MASK	0xF0
#define CCTL_L2_STATUS_C2_MASK	0xF00
#define CCTL_L2_STATUS_C3_MASK	0xF000

/* L2 cache operation */
#define CCTL_L2_PA_INVAL	0x8
#define CCTL_L2_PA_WB		0x9
#define CCTL_L2_PA_WBINVAL	0xA
#define CCTL_L2_WBINVAL_ALL	0x12

#define L2C_HPM_PER_CORE_OFFSET		0x8
#define L2C_REG_PER_CORE_OFFSET		0x10
#define CCTL_L2_STATUS_PER_CORE_OFFSET	0x4
#define L2C_REG_STATUS_OFFSET		0x0

#define L2C_REG_CN_CMD_OFFSET(n)	\
		(L2C_REG_C0_CMD_OFFSET + (n * L2C_REG_PER_CORE_OFFSET))
#define L2C_REG_CN_ACC_OFFSET(n)	\
		(L2C_REG_C0_ACC_OFFSET + (n * L2C_REG_PER_CORE_OFFSET))
#define CCTL_L2_STATUS_CN_MASK(n)	\
		(CCTL_L2_STATUS_C0_MASK << (n * CCTL_L2_STATUS_PER_CORE_OFFSET))
#define L2C_HPM_CN_CTL_OFFSET(n)	\
		(L2C_HPM_C0_CTL_OFFSET + (n * L2C_HPM_PER_CORE_OFFSET))
#define L2C_REG_CN_HPM_OFFSET(n)	\
		(L2C_REG_C0_HPM_OFFSET + (n * L2C_HPM_PER_CORE_OFFSET))

#ifdef CFG_USE_DCACHE

/* actually it's open dcache and icache */
__weak void dcache_enable(void)
{
	unsigned long long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);
	unsigned long long l2_control_val = readq((void *)ANDES_L2_BASE + L2C_REG_CTL_OFFSET);

	if (!(mcache_ctl_val & V5_MCACHE_CTL_DC_EN))
		mcache_ctl_val |= V5_MCACHE_CTL_DC_EN;

	csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

	l2_control_val |= L2_CONTROL_DRAMICTL;
	l2_control_val |= L2_CONTROL_TRAMICTL;

	/* config L2 I/D prefetch depth */
	if (sunxi_chip_alter_version() == SUNXI_CHIP_ALTER_VERSION_V821B) {
		l2_control_val &= ~L2_CONTROL_IPFDPT_MASK;
		l2_control_val |= L2_CONTROL_IPFDPT_3_REQ;
		l2_control_val &= ~L2_CONTROL_DPFDPT_MASK;
		l2_control_val |= L2_CONTROL_DPFDPT_8_REQ;
	}

	/* enable L2 cache */
	l2_control_val |= L2_CONTROL_CEN;

	writeq(l2_control_val, (void *)ANDES_L2_BASE + L2C_REG_CTL_OFFSET);
}

__weak void dcache_disable(void)
{

}
#endif

#ifdef CFG_USE_DCACHE

#define  CCTL_L1D_VA_INVAL	0
#define	 CCTL_L1D_VA_WB		1
#define  NDS_MCCTLBEGINADDR	0x7cb
#define  NDS_MCCTLCOMMAND	0x7cc
#define  L2_CACHESIZE		(128 * 1024)

static uint32_t cpu_l2c_get_cctl_status(void)
{
	void *l2c_addr = (void *)ANDES_L2_BASE;
	u32 hartid = 0;

	return readl((void *)(l2c_addr + L2C_REG_C0_STATUS_OFFSET + hartid * L2C_REG_STATUS_OFFSET));
}

void flush_dcache_all(void)
{
	u32 hartid = 0;
	void *l2c_addr = (void *)ANDES_L2_BASE;

	csr_write(CSR_UCCTLCOMMAND, V5_UCCTL_L1D_WBINVAL_ALL);
	if (l2c_addr) {
		writel(CCTL_L2_WBINVAL_ALL, (void *)(l2c_addr + L2C_REG_CN_CMD_OFFSET(hartid)));
		while ((cpu_l2c_get_cctl_status() & CCTL_L2_STATUS_CN_MASK(hartid)) != CCTL_L2_STATUS_IDLE)
			;
	}
}

static unsigned long get_cache_line_size(void)
{
	return L1_CACHE_BYTES;
}

static void cpu_dcache_wb_line(unsigned long start, unsigned long last_hartid)
{
	u32 hartid = 0;
	void *l2c_addr = (void *)ANDES_L2_BASE;

	csr_write(CSR_UCCTLBEGINADDR, start);
	csr_write(CSR_UCCTLCOMMAND, V5_UCCTL_L1D_VA_WB);

	if (l2c_addr && (hartid == last_hartid)) {
		writel(start, l2c_addr + L2C_REG_CN_ACC_OFFSET(hartid));
		writel(CCTL_L2_PA_WB, l2c_addr + L2C_REG_CN_CMD_OFFSET(hartid));
		while ((cpu_l2c_get_cctl_status() & CCTL_L2_STATUS_CN_MASK(hartid)) != CCTL_L2_STATUS_IDLE)
			;
	}
}

static void cpu_dcache_wb_range(unsigned long start, unsigned long end, unsigned long last_hartid)
{
	unsigned long line_size = get_cache_line_size();

	if ((end - start) >= L2_CACHESIZE) {
		flush_dcache_all();
		return;
	}

	while (end > start) {
		cpu_dcache_wb_line(start, last_hartid);
		start += line_size;
	}
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	unsigned long line_size = get_cache_line_size();

	start = start & (~(line_size - 1));
	cpu_dcache_wb_range(start, end, 0);
}

static void cpu_dcache_inval_line(unsigned long start, unsigned long last_hartid)
{
	u32 hartid = 0;
	void *l2c_addr = (void *)ANDES_L2_BASE;

	csr_write(CSR_UCCTLBEGINADDR, start);
	csr_write(CSR_UCCTLCOMMAND, V5_UCCTL_L1D_VA_INVAL);
	if (l2c_addr && (hartid == last_hartid)) {
		writel(start, (void *)(l2c_addr + L2C_REG_CN_ACC_OFFSET(hartid)));
		writel(CCTL_L2_PA_INVAL, (void *)(l2c_addr + L2C_REG_CN_CMD_OFFSET(hartid)));
		while ((cpu_l2c_get_cctl_status() & CCTL_L2_STATUS_CN_MASK(hartid)) != CCTL_L2_STATUS_IDLE)
			;
	}
}

static void cpu_dcache_inval_range(unsigned long start, unsigned long end, unsigned long last_hartid)
{
	unsigned long line_size = get_cache_line_size();

	if ((end - start) >= L2_CACHESIZE) {
		flush_dcache_all();
		return;
	}
	while (end > start) {
		cpu_dcache_inval_line(start, last_hartid);
		start += line_size;
	}
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	unsigned long line_size = get_cache_line_size();
	unsigned long last_hartid = 0;

	if (start == end)
		return;

	start = start & (~(line_size - 1));
	end = ((end + line_size - 1) & (~(line_size - 1)));

	cpu_dcache_inval_range(start, end, last_hartid);
}

#else /*CFG_USE_DCACHE*/
void invalidate_dcache_range(__maybe_unused unsigned long start,
			     __maybe_unused unsigned long stop)
{
}

void flush_dcache_range(__maybe_unused unsigned long start,
			__maybe_unused unsigned long stop)
{
}
#endif
__weak void mmu_enable(u32 dram_size)
{
#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
}

/* Keep L1&L2 dcache open */
__weak void mmu_disable(void)
{
#ifdef CFG_USE_DCACHE
	flush_dcache_all();
#endif
}

void data_sync_barrier(void)
{
	asm volatile("fence");
}
