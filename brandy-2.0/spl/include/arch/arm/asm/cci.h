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
#ifndef CCI_H
#define CCI_H
#include <asm/arch.h>
// #include <lib/utils_def.h>

/* Slave interface offsets from PERIPHBASE */
#define SLAVE_IFACE6_OFFSET		UL(0x7000)
#define SLAVE_IFACE5_OFFSET		UL(0x6000)
#define SLAVE_IFACE4_OFFSET		UL(0x5000)
#define SLAVE_IFACE3_OFFSET		UL(0x4000)
#define SLAVE_IFACE2_OFFSET		UL(0x3000)
#define SLAVE_IFACE1_OFFSET		UL(0x2000)
#define SLAVE_IFACE0_OFFSET		UL(0x1000)
#define SLAVE_IFACE_OFFSET(index)	(SLAVE_IFACE0_OFFSET + \
					(UL(0x1000) * (index)))

/* Slave interface event and count register offsets from PERIPHBASE */
#define EVENT_SELECT7_OFFSET		UL(0x80000)
#define EVENT_SELECT6_OFFSET		UL(0x70000)
#define EVENT_SELECT5_OFFSET		UL(0x60000)
#define EVENT_SELECT4_OFFSET		UL(0x50000)
#define EVENT_SELECT3_OFFSET		UL(0x40000)
#define EVENT_SELECT2_OFFSET		UL(0x30000)
#define EVENT_SELECT1_OFFSET		UL(0x20000)
#define EVENT_SELECT0_OFFSET		UL(0x10000)
#define EVENT_OFFSET(index)		(EVENT_SELECT0_OFFSET + \
					(UL(0x10000) * (index)))

/* Control and ID register offsets */
#define CTRL_OVERRIDE_REG		U(0x0)
#define SECURE_ACCESS_REG		U(0x8)
#define STATUS_REG			U(0xc)
#define IMPRECISE_ERR_REG		U(0x10)
#define PERFMON_CTRL_REG		U(0x100)
#define IFACE_MON_CTRL_REG		U(0x104)

/* Component and peripheral ID registers */
#define PERIPHERAL_ID0			U(0xFE0)
#define PERIPHERAL_ID1			U(0xFE4)
#define PERIPHERAL_ID2			U(0xFE8)
#define PERIPHERAL_ID3			U(0xFEC)
#define PERIPHERAL_ID4			U(0xFD0)
#define PERIPHERAL_ID5			U(0xFD4)
#define PERIPHERAL_ID6			U(0xFD8)
#define PERIPHERAL_ID7			U(0xFDC)

#define COMPONENT_ID0			U(0xFF0)
#define COMPONENT_ID1			U(0xFF4)
#define COMPONENT_ID2			U(0xFF8)
#define COMPONENT_ID3			U(0xFFC)
#define COMPONENT_ID4			U(0x1000)
#define COMPONENT_ID5			U(0x1004)
#define COMPONENT_ID6			U(0x1008)
#define COMPONENT_ID7			U(0x100C)

/* Slave interface register offsets */
#define SNOOP_CTRL_REG			U(0x0)
#define SH_OVERRIDE_REG			U(0x4)
#define READ_CHNL_QOS_VAL_OVERRIDE_REG	U(0x100)
#define WRITE_CHNL_QOS_VAL_OVERRIDE_REG	U(0x104)
#define MAX_OT_REG			U(0x110)

/* Snoop Control register bit definitions */
#define DVM_EN_BIT			BIT_32(1)
#define SNOOP_EN_BIT			BIT_32(0)
#define SUPPORT_SNOOPS			BIT_32(30)
#define SUPPORT_DVM			BIT_32(31)

/* Status register bit definitions */
#define CHANGE_PENDING_BIT		BIT_32(0)

/* Event and count register offsets */
#define EVENT_SELECT_REG		U(0x0)
#define EVENT_COUNT_REG			U(0x4)
#define COUNT_CNTRL_REG			U(0x8)
#define COUNT_OVERFLOW_REG		U(0xC)

/* Slave interface monitor registers */
#define INT_MON_REG_SI0			U(0x90000)
#define INT_MON_REG_SI1			U(0x90004)
#define INT_MON_REG_SI2			U(0x90008)
#define INT_MON_REG_SI3			U(0x9000C)
#define INT_MON_REG_SI4			U(0x90010)
#define INT_MON_REG_SI5			U(0x90014)
#define INT_MON_REG_SI6			U(0x90018)

/* Master interface monitor registers */
#define INT_MON_REG_MI0			U(0x90100)
#define INT_MON_REG_MI1			U(0x90104)
#define INT_MON_REG_MI2			U(0x90108)
#define INT_MON_REG_MI3			U(0x9010c)
#define INT_MON_REG_MI4			U(0x90110)
#define INT_MON_REG_MI5			U(0x90114)

#define SLAVE_IF_UNUSED			-1

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Function declarations */

/*
 * The ARM CCI driver needs the following:
 * 1. Base address of the CCI product
 * 2. An array  of map between AMBA 4 master ids and ACE/ACE lite slave
 *    interfaces.
 * 3. Size of the array.
 *
 * SLAVE_IF_UNUSED should be used in the map to represent no AMBA 4 master exists
 * for that interface.
 */
void cci_init(uintptr_t base, const int *map, unsigned int num_cci_masters);

void cci_enable_snoop_dvm_reqs(unsigned int master_id);
void cci_disable_snoop_dvm_reqs(unsigned int master_id);

#define __init

#define _DEFINE_COPROCR_READ_FUNC(_name, coproc, opc1, CRn, CRm, opc2)	\
static inline  unsigned long read_ ## _name(void)				\
{									\
	unsigned long v;							\
	__asm__ volatile ("mrc "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : "=r" (v));\
	return v;							\
}

/* Define read function for coproc register */
#define DEFINE_COPROCR_READ_FUNC(_name, ...) 				\
	_DEFINE_COPROCR_READ_FUNC(_name, __VA_ARGS__)

DEFINE_COPROCR_READ_FUNC(mpidr, MPIDR)

/* Define function for system instruction with type specifier */
#define DEFINE_SYSOP_TYPE_FUNC(_op, _type)		\
static inline void _op ## _type(void)			\
{							\
	__asm__ (#_op " " #_type : : : "memory");			\
}

DEFINE_SYSOP_TYPE_FUNC(dsb, ish)

void __init plat_arm_interconnect_init(void);
void plat_arm_interconnect_enter_coherency(void);
void plat_arm_interconnect_exit_coherency(void);

#endif /* __ASSEMBLER__ */
#endif /* CCI_H */
