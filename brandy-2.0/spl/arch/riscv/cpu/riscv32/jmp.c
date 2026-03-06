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
#include "private_opensbi.h"

void boot0_jmp(phys_addr_t addr)
{
	asm volatile("jr a0");
}

void boot0_jmp_optee(phys_addr_t optee, phys_addr_t uboot)
{
	phys_addr_t dtb_entry = uboot + 2 * 1024 * 1024;

	asm volatile("mv s1, %0" : : "r"(optee) : "memory");
	asm volatile("mv a0, %0" : : "r"(dtb_entry) : "memory");
	asm volatile("mv ra, %0" : : "r"(uboot) : "memory");
	asm volatile("jr     s1");
}

void boot0_jmp_monitor(phys_addr_t monitor_base)
{
#ifdef CONFIG_MONITOR
	asm volatile("jr a0");
__LOOP:
	asm volatile("WFI");
	goto __LOOP;
#endif
}

/* For OpenSBI-0.6 only.
 * This function is neglected. Don't use this any more!
 */
void boot0_jmp_opensbi(phys_addr_t opensbi_base, phys_addr_t uboot_base)
{
	asm volatile("jr a0");
__LOOP:
	printf("Failed to jump to opensbi\n");
	asm volatile("WFI");
	goto __LOOP;
}

/*
 * For OpenSBI-0.6:
 * - No register is used for passing arguments. Just jump to @opensbi_base
 *
 * For OpenSBI-1.0 and OpenSBI-1.4:
 * - a0: @hartid
 * - a1: @dtb_base. Must be aligned to 8 bytes.
 *
 * For backwards compatibility, we arrange the args of this function carefully.
 * Don't change the form of this funtion!
 */
void boot0_jmp_opensbi_v1x(unsigned long hartid, phys_addr_t dtb_base, phys_addr_t opensbi_base, phys_addr_t uboot_base)
{
	/* Jump to @opensbi_base */
	asm volatile("jr a2");

	/* @uboot_base is not used. It's hard-coded in opensbi as FW_JUMP_ADDR */
	(void)uboot_base;
__LOOP:
	printf("Failed to jump to opensbi v1x\n");
	asm volatile("WFI");
	goto __LOOP;
}

static void __jump_kernel(u32 opensbi_entry, u32 dtb_base, u32 kernel_addr)
{
	if (opensbi_entry) {
		printf("opensbi to Linux (%x)...,dtb (%x)\n",
		       (unsigned int)kernel_addr, (unsigned int)dtb_base);
		asm volatile("mv s1, %0" ::"r"(opensbi_entry) : "memory");
		asm volatile("mv a0, %0" ::"r"(0) : "memory");
		asm volatile("mv a1, %0" ::"r"(dtb_base) : "memory");
		asm volatile("mv ra, %0" ::"r"(kernel_addr) : "memory");
		asm volatile("jr s1");
	} else {
		asm volatile("mv s1, %0" ::"r"(kernel_addr) : "memory");
		asm volatile("mv a1, %0" ::"r"(dtb_base) : "memory");
		asm volatile("jr     s1");
	}
}

void boot0_jump_kernel(u32 dtb_base, u32 kernel_addr)
{
	__jump_kernel(0, dtb_base, kernel_addr);
}

void optee_jump_kernel(u32 opensbi_entry, u32 dtb_base, u32 kernel_addr)
{
	__jump_kernel(opensbi_entry, dtb_base, kernel_addr);
}

