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

#include <common.h>
#include <arch/rtc.h>
#include <config.h>

void boot0_jmp(phys_addr_t addr)
{
    asm volatile("mov r2, #0");
    asm volatile("mcr p15, 0, r2, c7, c5, 6");
    asm volatile("bx r0");
}

void boot0_jmp_optee(phys_addr_t optee, phys_addr_t uboot)
{
	phys_addr_t optee_entry = optee;
	phys_addr_t uboot_entry = uboot;

	/* dtb_entry is fixed to 2M offset of uboot_entry */
	phys_addr_t dtb_entry = uboot_entry + 2 * 1024 * 1024;

	asm volatile ("mov r2, %0" : : "r" (dtb_entry) : "memory");
	asm volatile ("mov lr, %0" : : "r" (uboot_entry) : "memory");
	asm volatile ("bx      %0" : : "r" (optee_entry) : "memory");
}

void boot0_jmp_monitor(phys_addr_t monitor_base)
{
#ifdef CONFIG_MONITOR
	/* jmp to AA64
	 *set the cpu boot entry addr:
	 */
	writel(monitor_base,RVBARADDR0_L);
	writel(0,RVBARADDR0_H);

	/*set cpu to AA64 execution state when the cpu boots into after a warm reset*/
	asm volatile("MRC p15,0,r2,c12,c0,2");
	asm volatile("ORR r2,r2,#(0x3<<0)");
	asm volatile("DSB");
	asm volatile("MCR p15,0,r2,c12,c0,2");
	asm volatile("ISB");
__LOOP:
	asm volatile("WFI");
	goto __LOOP;
#endif
}

static void enable_smp_kernel(void)
{
	//SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg
	asm volatile("MRC     p15, 0, r0, c1, c0, 1");  // Read ACTLR
	asm volatile("ORR     r0, r0, #0x040");         // Set bit 6
	asm volatile("MCR     p15, 0, r0, c1, c0, 1");  // Write ACTLR
}

typedef void (*Kernel_Entry)(int zero, int machine_id, void *fdt_addr);
static void __jump_kernel(u32 volatile optee_entry, u32 dtb_base, u32 kernel_addr)
{
	Kernel_Entry kernel_entry = NULL;

	kernel_entry = (Kernel_Entry)(ulong)kernel_addr;

	enable_smp_kernel();

	if (optee_entry) {
		pr_emerg("optee to Linux (%x)...,dtb (%x)\n",
		       (unsigned int)kernel_entry, (unsigned int)dtb_base);
		uint r0 = 0;
		uint r1 = 0xffffffff;
		asm volatile("mov r0, %0" : : "r"(r0) : "memory");
		asm volatile("mov r1, %0" : : "r"(r1) : "memory");
		asm volatile("mov r2, %0" : : "r"(dtb_base) : "memory");
		asm volatile("mov lr, %0" : : "r"(kernel_entry) : "memory");
		asm volatile("bx      %0" : : "r"(optee_entry) : "memory");
	} else {
		pr_emerg("Jump to Linux (%x)...,dtb (%x)\n",
		       (unsigned int)kernel_entry, (unsigned int)dtb_base);
#ifdef CFG_BOOT0_WIRTE_RTC_TO_ISP
		rtc_set_isp_flag();
#endif
		kernel_entry(0, 0, (u32 *)dtb_base);
	}

	printf("fail\n");
}

void boot0_jump_kernel(u32 dtb_base, u32 kernel_addr)
{
	__jump_kernel(0, dtb_base, kernel_addr);
}

void optee_jump_kernel(u32 optee_entry, u32 dtb_base, u32 kernel_addr)
{
	__jump_kernel(optee_entry, dtb_base, kernel_addr);
}
