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
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/gpio.h>
#include <arch/gpio_new.h>
#include <spare_head.h>
#include <libfdt.h>

void sunxi_gpio_set_cfgbank(struct sunxi_gpio *pio, int bank_offset, u32 val)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);

	clrsetbits_le32(&pio->cfg[0] + index, 0xf << offset, val << offset);
}

void sunxi_gpio_set_cfgpin(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	sunxi_gpio_set_cfgbank(pio, pin, val);
}

int sunxi_gpio_get_one_pin_data(u32 port, u32 pin)
{
	volatile u32 *tmp_group_data_addr;
	u32 tmp_group_data_data;

	tmp_group_data_addr = PIO_REG_DATA(port);
	tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);

	return (tmp_group_data_data >> pin) & 0x1;
}

int sunxi_gpio_get_cfgbank(struct sunxi_gpio *pio, int bank_offset)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);
	u32 cfg;

	cfg = readl(&pio->cfg[0] + index);
	cfg >>= offset;

	return cfg & 0xf;
}

int sunxi_gpio_get_cfgpin(u32 pin)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	return sunxi_gpio_get_cfgbank(pio, pin);
}

int sunxi_gpio_set_drv(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_DRV_INDEX(pin);
	u32 offset = GPIO_DRV_OFFSET(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	clrsetbits_le32(&pio->drv[0] + index, 0x3 << offset, val << offset);

	return 0;
}

int sunxi_gpio_set_pull(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_PULL_INDEX(pin);
	u32 offset = GPIO_PULL_OFFSET(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);
	clrsetbits_le32(&pio->pull[0] + index, 0x3 << offset, val << offset);

	return 0;
}

__weak int get_group_bit_offset(enum pin_e port_group)
{
	return -1;
}

#if CFG_SUNXI_GPIO_POWER_VOL_MODE
void io_pow_mode_disable(enum pin_e port_group)
{
	uint32_t reg;
	uint8_t group_bit_offset = get_group_bit_offset(port_group);
	if (group_bit_offset < 0)
		return;
	//disable auto mode
	reg = readl(PIOC_REG_POW_MS_CTL);
	reg &= ~(1 << group_bit_offset);
	reg |= (PIOC_CTL_Px_DISABLE << group_bit_offset);
	writel(reg, PIOC_REG_POW_MS_CTL);

}

enum io_pow_mode_e io_get_volt_val(enum pin_e port_group)
{
	uint32_t reg;
	uint8_t group_bit_offset = get_group_bit_offset(port_group);
	if (group_bit_offset < 0)
		return IO_MODE_DEFAULT;

	reg = readl(PIOC_REG_POW_VAL);
	return ((reg & (1 << group_bit_offset)) != PIOC_VAL_Px_3_3V_VOL)
		   ? IO_MODE_1_8_V
		   : IO_MODE_3_3_V;
}

void io_set_pow_mode(enum pin_e port_group, enum io_pow_mode_e volt_mode)
{
	uint32_t reg;
	uint8_t group_bit_offset = get_group_bit_offset(port_group);
	if (group_bit_offset < 0)
		return;
	if (volt_mode == IO_MODE_DEFAULT) {
		//default vaule in spec
		reg = readl(PIOC_REG_POW_MS_CTL);
		reg &= ~(1 << group_bit_offset);
		reg |= (PIOC_CTL_Px_DEFUALT << group_bit_offset);
		writel(reg, PIOC_REG_POW_MS_CTL);

		reg = readl(PIOC_REG_POW_MOD_SEL);
		reg &= ~(1 << group_bit_offset);
		reg |= (PIOC_SEL_Px_DEFAULT << group_bit_offset);
		writel(reg, PIOC_REG_POW_MOD_SEL);
	} else if (volt_mode == IO_MODE_AUTO) {
		//one specific combination is auto
		reg = readl(PIOC_REG_POW_MS_CTL);
		reg &= ~(1 << group_bit_offset);
		reg |= (PIOC_CTL_Px_ENABLE << group_bit_offset);
		writel(reg, PIOC_REG_POW_MS_CTL);

		reg = readl(PIOC_REG_POW_MOD_SEL);
		reg &= ~(1 << group_bit_offset);
		reg |= (PIOC_SEL_Px_3_3V_VOL << group_bit_offset);
		writel(reg, PIOC_REG_POW_MOD_SEL);
	} else {
		if (volt_mode == IO_MODE_1_8_V) {
			reg = readl(PIOC_REG_POW_MOD_SEL);
			reg &= ~(1 << group_bit_offset);
			reg |= (PIOC_SEL_Px_1_8V_VOL << group_bit_offset);
			writel(reg, PIOC_REG_POW_MOD_SEL);
		} else {
			reg = readl(PIOC_REG_POW_MOD_SEL);
			reg &= ~(1 << group_bit_offset);
			reg |= (PIOC_SEL_Px_3_3V_VOL << group_bit_offset);
			writel(reg, PIOC_REG_POW_MOD_SEL);
		}
	}
}

void sunxi_io_set_pow_mode_on_actual_val(enum pin_e port_group)
{
	io_pow_mode_disable(port_group);

	io_set_pow_mode(port_group, io_get_volt_val(port_group));
}

void sunxi_io_set_pow_mode_to_default(enum pin_e port_group)
{
	io_set_pow_mode(port_group, IO_MODE_DEFAULT);
}

#endif// CFG_SUNXI_GPIO_POWER_VOL_MODE

int boot_set_gpio(void  *user_gpio_list, u32 group_count_max, int set_gpio)
{
	normal_gpio_set_t *tmp_user_gpio_data, *gpio_list;
	u32 first_port;
	u32 tmp_group_func_data;
	u32 tmp_group_pull_data;
	u32 tmp_group_dlevel_data;
	u32 tmp_group_data_data;
	u32 data_change = 0;
	u32 port, port_num, port_num_func, port_num_pull, port_num_dlevel;
	u32 pre_port = 0, pre_port_num_func, pre_port_num_dlevel;
	u32 pre_port_num_pull;
	volatile u32 *tmp_group_func_addr, *tmp_group_pull_addr;
	volatile u32 *tmp_group_dlevel_addr, *tmp_group_data_addr;
	int i, tmp_val;

	gpio_list = (normal_gpio_set_t *)user_gpio_list;
	/* printf("gpio: conut=%d, set gpio = %d\n", group_count_max, set_gpio); */
	for (first_port = 0; first_port < group_count_max; first_port++) {
		tmp_user_gpio_data = gpio_list + first_port;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;

		if (!port) {
			continue;
		}
#if CFG_SUNXI_GPIO_POWER_VOL_MODE
		if (port != pre_port) {
			sunxi_io_set_pow_mode_on_actual_val(port - 1);
		}
#endif
		/*32-bit register */
		port_num_func = (port_num /(32/CFG_BIT_WIDTH));
		port_num_pull = (port_num /(32/PULL_BIT_WIDTH));
		port_num_dlevel = (port_num /(32/DRV_BIT_WIDTH));

		if (port < 12) {
			tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
			tmp_group_data_addr = PIO_REG_DATA(port);
		} else {
			tmp_group_func_addr = R_PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = R_PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = R_PIO_REG_DLEVEL(port, port_num_dlevel);
			tmp_group_data_addr = R_PIO_REG_DATA(port);
		}

		tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
		tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
		tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
		tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);

		pre_port = port;
		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;
		pre_port_num_dlevel = port_num_dlevel;

		/* update funtion */
		tmp_val = (port_num % (32/CFG_BIT_WIDTH)) * CFG_BIT_WIDTH;
		tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
		if (set_gpio) {
			tmp_group_func_data |=
			    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK) << tmp_val;
		}
		/* update pull  */
		tmp_val = (port_num % (32/PULL_BIT_WIDTH)) * PULL_BIT_WIDTH;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}
		/* update driver level */
		tmp_val = (port_num % (32/DRV_BIT_WIDTH)) * DRV_BIT_WIDTH;
		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(PIO_DRV_MASK << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & PIO_DRV_MASK) << tmp_val;
		}
		/* update data */
		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}

		break;
	}

	if (first_port >= group_count_max) {
		return -1;
	}

	for (i = first_port + 1; i < group_count_max; i++) {
		tmp_user_gpio_data = gpio_list + i;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		if (!port) {
			break;
		}
		port_num_func = (port_num /(32/CFG_BIT_WIDTH));
		port_num_pull = (port_num /(32/PULL_BIT_WIDTH));
		port_num_dlevel = (port_num /(32/DRV_BIT_WIDTH));

		/* The same register is not written before and after */
		if ((port_num_pull != pre_port_num_pull) ||
			(port != pre_port)) {
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
			GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
			GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
			if (data_change) {
				data_change = 0;
				GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
			}

			if (port < 12) {
				tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
				tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
				tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
				tmp_group_data_addr = PIO_REG_DATA(port);
			} else {
				tmp_group_func_addr = R_PIO_REG_CFG(port, port_num_func);
				tmp_group_pull_addr = R_PIO_REG_PULL(port, port_num_pull);
				tmp_group_dlevel_addr = R_PIO_REG_DLEVEL(port, port_num_dlevel);
				tmp_group_data_addr = R_PIO_REG_DATA(port);
			}

			tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
			tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
			tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
			tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);
		} else if (pre_port_num_func != port_num_func ||
					pre_port_num_dlevel != port_num_dlevel) {

			if (pre_port_num_func != port_num_func) {
				GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
				if (port < 12) {
					tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
				} else {
					tmp_group_func_addr = R_PIO_REG_CFG(port, port_num_func);
				}
				tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
			}
			if (pre_port_num_dlevel != port_num_dlevel) {
				GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
				if (port < 12) {
					tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
				} else {
					tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
				}
				tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
			}
		}

		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;
		pre_port_num_dlevel = port_num_dlevel;
		pre_port = port;

		/* write to the same register */
		tmp_val = (port_num % (32/CFG_BIT_WIDTH)) * CFG_BIT_WIDTH;
		if (tmp_user_gpio_data->mul_sel >= 0) {
			tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
			if (set_gpio) {
				tmp_group_func_data |=
				    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK)
				    << tmp_val;
			}
		}

		tmp_val = (port_num % (32/PULL_BIT_WIDTH)) * PULL_BIT_WIDTH;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}

		tmp_val = (port_num % (32/DRV_BIT_WIDTH)) * DRV_BIT_WIDTH;
		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(PIO_DRV_MASK << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & PIO_DRV_MASK) << tmp_val;
		}

		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}
	} /* for */

	/* The last set of data written */
	if (tmp_group_func_addr) {
		GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
		GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
		GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);

		if (data_change) {
			GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
		}
	}

	return 0;
}


#ifdef CFG_SET_GPIO_NEW
/* Due to the incompatibility of port in the code, this function is redefined*/
int boot_set_gpio_new(void *user_gpio_list, u32 group_count_max, int set_gpio)
{
	normal_gpio_set_t *tmp_user_gpio_data, *gpio_list;
	u32 first_port;
	u32 tmp_group_func_data;
	u32 tmp_group_pull_data;
	u32 tmp_group_dlevel_data;
	u32 tmp_group_data_data;
	u32 data_change = 0;
	u32 port, port_num, port_num_func, port_num_pull, port_num_dlevel;
	u32 pre_port, pre_port_num_func, pre_port_num_dlevel;
	u32 pre_port_num_pull;
	volatile u32 *tmp_group_func_addr, *tmp_group_pull_addr;
	volatile u32 *tmp_group_dlevel_addr, *tmp_group_data_addr;
	int i, tmp_val;

	gpio_list = (normal_gpio_set_t *)user_gpio_list;
	/* printf("gpio: conut=%d, set gpio = %d\n", group_count_max, set_gpio); */
	for (first_port = 0; first_port < group_count_max; first_port++) {
		tmp_user_gpio_data = gpio_list + first_port;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		if (port == PORT_NO_USE || port_num == PORT_NUM_NO_USE) {
			continue;
		}

		/*32-bit register */
		port_num_func = (port_num /(32/CFG_BIT_WIDTH));
		port_num_pull = (port_num /(32/PULL_BIT_WIDTH));
		port_num_dlevel = (port_num /(32/DRV_BIT_WIDTH));

		if (port < 12) {
			tmp_group_func_addr = NEW_PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = NEW_PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
			tmp_group_data_addr = NEW_PIO_REG_DATA(port);
		} else {
			tmp_group_func_addr = R_NEW_PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = R_NEW_PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = R_NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
			tmp_group_data_addr = R_NEW_PIO_REG_DATA(port);
		}

		tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
		tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
		tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
		tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);

		pre_port = port;
		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;
		pre_port_num_dlevel = port_num_dlevel;

		/* update funtion */
		tmp_val = (port_num % (32/CFG_BIT_WIDTH)) * CFG_BIT_WIDTH;
		tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
		if (set_gpio) {
			tmp_group_func_data |=
			    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK) << tmp_val;
		}
		/* update pull  */
		tmp_val = (port_num % (32/PULL_BIT_WIDTH)) * PULL_BIT_WIDTH;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}
		/* update driver level */
		tmp_val = (port_num % (32/DRV_BIT_WIDTH)) * DRV_BIT_WIDTH;
		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(PIO_DRV_MASK << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & PIO_DRV_MASK) << tmp_val;
		}
		/* update data */
		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}

		break;
	}

	if (first_port >= group_count_max) {
		return -1;
	}

	for (i = first_port + 1; i < group_count_max; i++) {
		tmp_user_gpio_data = gpio_list + i;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		if (port == PORT_NO_USE || port_num == PORT_NUM_NO_USE) {
			continue;
		}
		port_num_func = (port_num /(32/CFG_BIT_WIDTH));
		port_num_pull = (port_num /(32/PULL_BIT_WIDTH));
		port_num_dlevel = (port_num /(32/DRV_BIT_WIDTH));

		/* The same register is not written before and after */
		if ((port_num_pull != pre_port_num_pull) ||
			(port != pre_port)) {
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
			GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
			GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
			if (data_change) {
				data_change = 0;
				GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
			}

			if (port < 12) {
				tmp_group_func_addr = NEW_PIO_REG_CFG(port, port_num_func);
				tmp_group_pull_addr = NEW_PIO_REG_PULL(port, port_num_pull);
				tmp_group_dlevel_addr = NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
				tmp_group_data_addr = NEW_PIO_REG_DATA(port);
			} else {
				tmp_group_func_addr = R_NEW_PIO_REG_CFG(port, port_num_func);
				tmp_group_pull_addr = R_NEW_PIO_REG_PULL(port, port_num_pull);
				tmp_group_dlevel_addr = R_NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
				tmp_group_data_addr = R_NEW_PIO_REG_DATA(port);
			}

			tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
			tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
			tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
			tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);
		} else if (pre_port_num_func != port_num_func ||
					pre_port_num_dlevel != port_num_dlevel) {

			if (pre_port_num_func != port_num_func) {
				GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
				if (port < 12) {
					tmp_group_func_addr = NEW_PIO_REG_CFG(port, port_num_func);
				} else {
					tmp_group_func_addr = R_NEW_PIO_REG_CFG(port, port_num_func);
				}
				tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
			}
			if (pre_port_num_dlevel != port_num_dlevel) {
				GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
				if (port < 12) {
					tmp_group_dlevel_addr = NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
				} else {
					tmp_group_dlevel_addr = NEW_PIO_REG_DLEVEL(port, port_num_dlevel);
				}
				tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
			}
		}

		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;
		pre_port_num_dlevel = port_num_dlevel;
		pre_port = port;

		/* write to the same register */
		tmp_val = (port_num % (32/CFG_BIT_WIDTH)) * CFG_BIT_WIDTH;
		if (tmp_user_gpio_data->mul_sel >= 0) {
			tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
			if (set_gpio) {
				tmp_group_func_data |=
				    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK)
				    << tmp_val;
			}
		}

		tmp_val = (port_num % (32/PULL_BIT_WIDTH)) * PULL_BIT_WIDTH;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}

		tmp_val = (port_num % (32/DRV_BIT_WIDTH)) * DRV_BIT_WIDTH;
		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(PIO_DRV_MASK << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & PIO_DRV_MASK) << tmp_val;
		}

		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}
	} /* for */

	/* The last set of data written */
	if (tmp_group_func_addr) {
		GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
		GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
		GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);

		if (data_change) {
			GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
		}
	}

	return 0;
}

#endif

#ifdef CFG_SUNXI_FDT
int fdt_get_one_gpio(const char *node_path, const char *prop_name,
		     normal_gpio_cfg *gpio_list)
{
	int nodeoffset; /* node offset from libfdt */
	int ret;
	u32 data[10];

	memset(data, 0, sizeof(data));
	//get property vaule by handle
	nodeoffset = fdt_path_offset(working_fdt, node_path);
	if (nodeoffset < 0) {
		printf("fdt err returned %s\n", fdt_strerror(nodeoffset));
		return -1;
	}

	ret = fdt_getprop_u32(working_fdt, nodeoffset, prop_name, data);
	if (ret < 0) {
		printf("%s :%s|%s err returned %s\n", __func__, node_path,
		       prop_name, fdt_strerror(ret));
		return -1;
	}

	// strcpy(gpio_list->gpio_name,prop_name);

	gpio_list->port	     = data[1]; //0: PA
	gpio_list->port_num  = data[2];
	gpio_list->mul_sel   = data[3];
	gpio_list->pull	     = data[4];
	gpio_list->drv_level = data[5];
	gpio_list->data	     = data[6];

	boot_info(
		"port = %x,portnum=%x,mul_sel=%x,pull=%x drive= %x, data=%x\n",
		gpio_list->port, gpio_list->port_num, gpio_list->mul_sel,
		gpio_list->pull, gpio_list->drv_level, gpio_list->data);
	return 0;
}
#endif
