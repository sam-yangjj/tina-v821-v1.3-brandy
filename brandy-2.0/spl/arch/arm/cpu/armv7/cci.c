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
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <common.h>
// #include <arch_helpers.h>
// #include <common/debug.h>
#include <asm/cci.h>
// #include <lib/mmio.h>

#define ENABLE_ASSERTIONS 1

#define MAKE_CCI_PART_NUMBER(hi, lo) (((hi) << 8) | (lo))
#define CCI_PART_LO_MASK	     U(0xff)
#define CCI_PART_HI_MASK	     U(0xf)

/* CCI part number codes read from Peripheral ID registers 0 and 1 */
#define CCI400_PART_NUM 0x420
#define CCI500_PART_NUM 0x422
#define CCI550_PART_NUM 0x423

#define CCI400_SLAVE_PORTS 5
#define CCI500_SLAVE_PORTS 7
#define CCI550_SLAVE_PORTS 7

static uintptr_t cci_base;
static const int *cci_slave_if_map;

#if ENABLE_ASSERTIONS
static unsigned int max_master_id;
static int cci_num_slave_ports;

static bool validate_cci_map(const int *map)
{
	unsigned int valid_cci_map = 0U;
	int slave_if_id;
	unsigned int i;

	/* Validate the map */
	for (i = 0U; i <= max_master_id; i++) {
		slave_if_id = map[i];

		if (slave_if_id < 0)
			continue;

		if (slave_if_id >= cci_num_slave_ports) {
			printf("Slave interface ID is invalid\n");
			return false;
		}

		if ((valid_cci_map & (1UL << slave_if_id)) != 0U) {
			printf("Multiple masters are assigned same slave interface ID\n");
			return false;
		}
		valid_cci_map |= 1UL << slave_if_id;
	}

	if (valid_cci_map == 0U) {
		printf("No master is assigned a valid slave interface\n");
		return false;
	}

	return true;
}

/*
 * Read CCI part number from Peripheral ID registers
 */
static unsigned int read_cci_part_number(uintptr_t base)
{
	unsigned int part_lo, part_hi;

	part_lo = readl(base + PERIPHERAL_ID0) & CCI_PART_LO_MASK;
	part_hi = readl(base + PERIPHERAL_ID1) & CCI_PART_HI_MASK;

	return MAKE_CCI_PART_NUMBER(part_hi, part_lo);
}

/*
 * Identify a CCI device, and return the number of slaves. Return -1 for an
 * unidentified device.
 */
static int get_slave_ports(unsigned int part_num)
{
	int num_slave_ports = -1;

	switch (part_num) {
	case CCI400_PART_NUM:
		num_slave_ports = CCI400_SLAVE_PORTS;
		break;
	case CCI500_PART_NUM:
		num_slave_ports = CCI500_SLAVE_PORTS;
		break;
	case CCI550_PART_NUM:
		num_slave_ports = CCI550_SLAVE_PORTS;
		break;
	default:
		/* Do nothing in default case */
		break;
	}

	return num_slave_ports;
}
#endif /* ENABLE_ASSERTIONS */

void __init cci_init(uintptr_t base, const int *map,
		     unsigned int num_cci_masters)
{
	assert(map != NULL);
	assert(base != 0U);

	cci_base	 = base;
	cci_slave_if_map = map;

#if ENABLE_ASSERTIONS
	/*
	 * Master Id's are assigned from zero, So in an array of size n
	 * the max master id is (n - 1).
	 */
	max_master_id	    = num_cci_masters - 1U;
	cci_num_slave_ports = get_slave_ports(read_cci_part_number(base));
	assert(cci_num_slave_ports >= 0);
#endif

	assert(validate_cci_map(map));
}

void cci_enable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id = cci_slave_if_map[master_id];

	assert(master_id <= max_master_id);
	assert((slave_if_id < cci_num_slave_ports) && (slave_if_id >= 0));
	assert(cci_base != 0U);

	/*
	 * Enable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore
	 */
	writel(DVM_EN_BIT | SNOOP_EN_BIT,
	       cci_base + SLAVE_IFACE_OFFSET(slave_if_id) + SNOOP_CTRL_REG);

	/*
	 * Wait for the completion of the write to the Snoop Control Register
	 * before testing the change_pending bit
	 */
	dsbish();

	/* Wait for the dust to settle down */
	while ((readl(cci_base + STATUS_REG) & CHANGE_PENDING_BIT) != 0U)
		;
}

void cci_disable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id = cci_slave_if_map[master_id];

	assert(master_id <= max_master_id);
	assert((slave_if_id < cci_num_slave_ports) && (slave_if_id >= 0));
	assert(cci_base != 0U);

	/*
	 * Disable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore.
	 */
	writel(DVM_EN_BIT | SNOOP_EN_BIT,
	       cci_base + SLAVE_IFACE_OFFSET(slave_if_id) + SNOOP_CTRL_REG);

	/*
	 * Wait for the completion of the write to the Snoop Control Register
	 * before testing the change_pending bit
	 */
	dsbish();

	/* Wait for the dust to settle down */
	while ((readl(cci_base + STATUS_REG) & CHANGE_PENDING_BIT) != 0U)
		;
}
