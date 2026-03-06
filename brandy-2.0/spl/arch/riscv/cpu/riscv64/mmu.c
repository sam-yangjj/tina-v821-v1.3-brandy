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

#define L1_CACHE_BYTES (64)

#ifdef CFG_USE_DCACHE
/* init and enable dcache */
__weak void dcache_init(void)
{
	/*
	(0:1) CACHE_SEL=2’b11时，选中指令和数据高速缓存
	(4) INV=1时高速缓存进行无效化
	(16) BHT_INV=1时分支历史表内的数据进行无效化
	(17) TB_INV=1时分支目标缓冲器内的数据进行无效化
	*/
	csr_write(CSR_MCOR, 0x70013);

	/*
	(0) IE=1时Icache打开
	(1) DE=1时Dcache打开
	(2) WA=1时数据高速缓存为write allocate模式 (c906不支持)
	(3) WB=1时数据高速缓存为写回模式  (c906固定为1)
	(4) RS=1时返回栈开启
	(5) BPE=1时预测跳转开启
	(6) BTB=1时分支目标预测开启
	(8) WBR=1时支持写突发传输写 （c906固定为1）
	(12) L0BTB=1时第一级分支目标预测开启
	*/
#ifdef CFG_SUNXI_RTOS_MELIS
	csr_write(CSR_MHCR, 0x11fb);
#else
	csr_write(CSR_MHCR, 0x11ff);
#endif


	/*
	(15) MM为1时支持非对齐访问，硬件处理非对齐访问
	(16) UCME为1时，用户模式可以执行扩展的cache操作指令
	(17) CLINTEE为1时，CLINT发起的超级用户软件中断和计时器中断可以被响应
	(21) MAEE为1时MMU的pte中扩展地址属性位，用户可以配置页面的地址属性
	(22) THEADISAEE为1时可以使用C906扩展指令集
	*/
	csr_set(CSR_MXSTATUS, 0x638000);


	/*
	(2) DPLD=1，dcache预取开启
	(3,4,5,6,7) AMR=1，时，在出现连续3条缓存行的存储操作时后续连续地址的存储操作不再写入L1Cache
	(8) IPLD=1ICACHE预取开启
	(9) LPE=1循环加速开启
	(13,14) DPLD为2时，预取8条缓存行
	*/
	csr_write(CSR_MHINT, 0x16e30c);
}

__weak void dcache_enable(void)
{

}

__weak void dcache_disable(void)
{

}
#endif



__weak void mmu_enable(u32 dram_size)
{
#ifdef CFG_USE_DCACHE
	dcache_init();
#endif
}

__weak void mmu_disable(void)
{

}

void data_sync_barrier(void)
{
	asm volatile("fence.i");
}

#ifdef CFG_USE_DCACHE
void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x0295000b");	/*dcache.cpa a0*/
	asm volatile(".long 0x01b0000b");		/*sync.is*/
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile ("dcache.ipa a0");

	asm volatile (".long 0x01b0000b");
	/* flush_dcache_all(); */
}
#else /*CFG_USE_DCACHE*/
void invalidate_dcache_range(__maybe_unused unsigned long start,
			     __maybe_unused unsigned long stop) { }

void flush_dcache_range(__maybe_unused unsigned long start,
			__maybe_unused unsigned long stop) { }
#endif

