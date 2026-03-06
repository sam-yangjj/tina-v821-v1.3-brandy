// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Allwinnertech Technology Corporation
 *
 */

#include <common.h>
#include <asm/cache.h>
#include <asm/csr.h>

#define L1_CACHE_BYTES CONFIG_SYS_CACHELINE_SIZE

#define CSR_MHCR 0x7c1
#define CSR_MXSTATUS 0x7c0
#define CSR_MHINT 0x7c5

extern void sysmap_init(void);

void flush_dcache_all(void)
{
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a5") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x0297800B"); /*dcache.cpa a0*/

	asm volatile(".long 0x01b0000b");
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a5") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x02a7800b"); /*dcache.cipa a0*/

	asm volatile(".long 0x01b0000b");
}

void hardware_config(void)
{
	sysmap_init();
	/*
	 * (0) When IE=1, Icache is enabled
	 * (1) When DE=1, Dcache is enabled
	 * (2) When WB=1, data cache is in write-back mode (write-back data goes to dcache, write-through data goes to memory)
	 * (3) When WA=1, data cache is in write-allocate mode
	 * (4) When RS=1, return stack is enabled
	 * (5) When BPE=1, branch prediction is enabled
	 * (12) When BTB=1, branch target prediction is enabled
	 */
	csr_write(CSR_MHCR, (1 << 12) | (0xf << 2));

	/*
	 * (10) PMDU=0 means event detection counter in user mode counts normally, =1 disables counting
	 * (13) PMDM=0 means event detection counter in machine mode counts normally, =1 disables counting
	 * (15) MM=1 supports unaligned access, hardware handles unaligned access
	 * (22) THEADISAEE=1 allows using THEAD extended instruction set
	 * (30,31) PM: current privilege mode, b11 machine mode, b00 user mode
	 */
	csr_set(CSR_MXSTATUS, (1 << 22) | (1 << 15));

	/*
	 * (20) AEE=1 for precise exception, which will block the pipeline for Load/Store instructions, causing performance degradation
	 */
	/* csr_write(CSR_MHINT, (1 << 20)); */

	/*
	 * (2) D_PLD=1 enables Dcache prefetching
	 * (3,4) AMR=1 means that after 3 consecutive cache line store operations, subsequent stores to continuous addresses will no longer write to Dcache
	 * (13,14) PREF_N=3 means prefetching 16 cache lines
	 */
	csr_write(CSR_MHINT, 0x600c);
}

void icache_enable(void)
{
	hardware_config();
	csr_set(CSR_MHCR, 0x1);
}

void icache_disable(void)
{
	csr_clear(CSR_MHCR, 1 << 0);
}

void dcache_enable(void)
{
	csr_set(CSR_MHCR, 0x1 << 1);
}

void dcache_disable(void)
{
	;
}

int icache_status(void)
{
	return (csr_read(CSR_MHCR) & (1 << 0)) != 0;
}

int dcache_status(void)
{
	return (csr_read(CSR_MHCR) & (1 << 1)) != 0;
}
