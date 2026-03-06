/*
 * (C) Copyright 2018 allwinnertech zhangyuanjing<zhangyuanjing@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define DEBUG */

#include <common.h>
#include <console.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <sunxi_board.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...)                                                  \
	do {                                                                   \
		printf("DEBUG_PRINT: %s:%d: " fmt, __FILE__, __LINE__,         \
		       ##__VA_ARGS__);                                         \
	} while (0)
#else
#define DEBUG_PRINT(fmt, ...)                                                  \
	do {                                                                   \
	} while (0)
#endif

#define WUP_IO_EN_REG (0x4a000050)
#define WUP_IO_DEB_CLK_REG (0x4a000054)
#define WUP_IO_STA_REG (0x4a000058)
#define WUP_IO_HOLD_REG (0x4a00005C)
#define WUP_IO_WUP_GEN_REG (0x4a000060)
#define WUP_IO_WUP_CYC0_REG (0x4a000064)
#define WUP_IO_WUP_CYC1_REG (0x4a000068)

#define WUP_IO_DEB_CLK_SET1_SHIFT (28)
#define WUP_IO_DEB_CLK_SET1_MASK (0xF << WUP_IO_DEB_CLK_SET1_SHIFT)
#define WUP_IO_DEB_CLK_SET0_SHIFT (24)
#define WUP_IO_DEB_CLK_SET0_MASK (0xF << WUP_IO_DEB_CLK_SET0_SHIFT)
#define WUP_IO_MODE_SHIFT (16)
#define WUP_IO_DEB_CLK_SEL_SHIFT (0)
#define WUP_IO_DEB_CYC_OFFSET (4)
#define WUP_IO_DEB_CYC_SHIFT (0)
#define WUP_IO_DEB_CYC_VMASK (0xF)
#define WUP_IO_HOLD_SHIFT (0)
#define WUP_IO_EN_SHIFT (0)
#define WUP_IO_GEN_MASK (0x00000001)

#define SUXNI_IRQ_WUPIO MAKE_IRQn(186U, 0)
#define WUP_IO_NUN (8)
#define WUP_IO_MASK (0x000000FF)

enum {
	GPIO_CFG0     = 0x00,
	GPIO_CFG1     = 0x04,
	GPIO_CFG2     = 0x08,
	GPIO_CFG3     = 0x0c,
	GPIO_DAT      = 0x10,
	GPIO_DRV0     = 0x14,
	GPIO_DRV1     = 0x18,
	GPIO_DRV2     = 0x1c,
	GPIO_DRV3     = 0x20,
	GPIO_PUL0     = 0x24,
	GPIO_PUL1     = 0x28,
	GPIO_OFFSET   = 0x30,
	GPIO_CFG_MASK = 0xf,
	GPIO_DRV_MASK = 0x3,
};

typedef enum {
	WUPIO_PIN_PL0 = 0,
	WUPIO_PIN_PL1 = 1,
	WUPIO_PIN_PL2 = 2,
	WUPIO_PIN_PL3 = 3,
	WUPIO_PIN_PL4 = 4,
	WUPIO_PIN_PL5 = 5,
	WUPIO_PIN_PL6 = 6,
	WUPIO_PIN_PL7 = 7,
	WUPIO_PIN_MAX,
} wupio_pin_t;

enum {
	GPIO_PORTA = 0,
	GPIO_PORTB,
	GPIO_PORTC,
	GPIO_PORTD,
	GPIO_PORTE,
	GPIO_PORTF,
	GPIO_PORTG,
	GPIO_PORTH,
	GPIO_PORTI,
	GPIO_PORTJ,
	GPIO_PORTK,
	GPIO_PORTL,
	GPIO_PORTM,
	GPIO_PORTN,
};

enum {
	GPIO_INPUT	  = 0,
	GPIO_OUTPUT	  = 1,
	GPIO_PERIPH_MUX2  = 2,
	GPIO_PERIPH_MUX3  = 3,
	GPIO_PERIPH_MUX4  = 4,
	GPIO_PERIPH_MUX5  = 5,
	GPIO_PERIPH_MUX6  = 6,
	GPIO_PERIPH_MUX7  = 7,
	GPIO_PERIPH_MUX8  = 8,
	GPIO_PERIPH_MUX14 = 14,
	GPIO_DISABLED	  = 0xf,
};

enum gpio_pull_t {
	GPIO_PULL_UP   = 0,
	GPIO_PULL_DOWN = 1,
	GPIO_PULL_NONE = 2,
};

typedef enum {
	WUPIO_MODE_NEGATIVE_EDGE = 0,
	WUPIO_MODE_POSITIVE_EDGE = 1,
	WUPIO_MODE_MAX,
} wupio_mode_t;

typedef enum {
	WUPIO_DEBSEL_0 = 0,
	WUPIO_DEBSEL_1 = 1,
	WUPIO_DEBSEL_MAX,
} wupio_debsel_t;

typedef struct {
	uint8_t clk_sel_0;
	uint8_t clk_sel_1;
} wupio_init_t;

typedef struct {
	wupio_mode_t mode;
	wupio_debsel_t deb_sel;
	uint8_t deb_cyc_h;
	uint8_t deb_cyc_l;
} wupio_config_t;

typedef uint32_t gpio_t;

#define hal_readl(addr) readl((void *)addr)
#define hal_writel(val, addr) writel((val), (void *)(addr))

#define PIO_NUM_IO_BITS 5
#define GPIO_PIN(x, y) (((uint32_t)(x << PIO_NUM_IO_BITS)) | y)

static inline uint32_t _port_num(gpio_t pin)
{
	return pin >> PIO_NUM_IO_BITS;
}

static uint32_t _port_base_get(gpio_t pin)
{
	uint32_t port = pin >> PIO_NUM_IO_BITS;

	/* PL PM PN in R_PIO */
	if (port >= GPIO_PORTL) {
		return SUNXI_R_PIO_BASE + (port - GPIO_PORTL) * GPIO_OFFSET;
	}
	/* PA PB PC PD PE PF PG PH PI PJ PK in PIO */
	return SUNXI_PIO_BASE + port * GPIO_OFFSET;
}

static inline uint32_t _pin_num(gpio_t pin)
{
	return (pin & ((1 << PIO_NUM_IO_BITS) - 1));
}

void sunxi_gpio_wupio_set_mux(gpio_t pin, int cfg)
{
	uint32_t port_addr = _port_base_get(pin);
	uint32_t pin_num   = _pin_num(pin);
	uint32_t addr;
	uint32_t val;

	addr = port_addr + GPIO_CFG0 + ((pin_num >> 3) << 2);
	val  = hal_readl(addr);
	val &= ~(0xf << ((pin_num & 0x7) << 2));
	val |= ((cfg & GPIO_CFG_MASK) << ((pin_num & 0x7) << 2));
	hal_writel(val, addr);

	DEBUG_PRINT(
		"GPIO: MUX pin = %d, num in bank = %d, addr = 0x%08x, val = 0x%08x, set cfg = %d\n",
		pin, pin_num, addr, hal_readl(addr), cfg);
}

void sunxi_gpio_wupio_set_pull(gpio_t pin, enum gpio_pull_t pull)
{
	uint32_t port_addr = _port_base_get(pin);
	uint32_t pin_num   = _pin_num(pin);
	uint32_t addr;
	uint32_t val, v;

	switch (pull) {
	case GPIO_PULL_UP:
		v = 0x1;
		break;

	case GPIO_PULL_DOWN:
		v = 0x2;
		break;

	case GPIO_PULL_NONE:
		v = 0x0;
		break;

	default:
		v = 0x0;
		break;
	}

	addr = port_addr + GPIO_PUL0 + ((pin_num >> 4) << 2);
	val  = hal_readl(addr);
	val &= ~(0x3 << ((pin_num & 0xf) << 1));
	val |= (v << ((pin_num & 0xf) << 1));
	hal_writel(val, addr);

	DEBUG_PRINT(
		"GPIO: PULL pin = %d, addr = 0x%08x, val = 0x%08x, set pull = %d\n",
		pin, addr, hal_readl(addr), v);
}

void wupio_config(int pin, wupio_config_t *config)
{
	uint32_t val;

	DEBUG_PRINT("wupio_config: Configuring WUPIO pin %d\n", pin);

	/* set mode */
	val = hal_readl(WUP_IO_EN_REG);
	DEBUG_PRINT("wupio_config: WUP_IO_EN_REG before mode change: 0x%08x\n",
		    val);
	val &= ~(1 << (pin + WUP_IO_MODE_SHIFT));
	val |= (config->mode << (pin + WUP_IO_MODE_SHIFT));
	DEBUG_PRINT("wupio_config: WUP_IO_EN_REG after mode change: 0x%08x\n",
		    val);
	hal_writel(val, WUP_IO_EN_REG);

	/* select debounce clock */
	val = hal_readl(WUP_IO_DEB_CLK_REG);
	DEBUG_PRINT(
		"wupio_config: WUP_IO_DEB_CLK_REG before debounce clock change: 0x%08x\n",
		val);
	val &= ~(1 << (pin + WUP_IO_DEB_CLK_SEL_SHIFT));
	val |= (config->deb_sel << (pin + WUP_IO_DEB_CLK_SEL_SHIFT));
	DEBUG_PRINT(
		"wupio_config: WUP_IO_DEB_CLK_REG after debounce clock change: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_DEB_CLK_REG);

	/* set debounce cycle for low level */
	val = hal_readl(WUP_IO_WUP_CYC0_REG);
	DEBUG_PRINT(
		"wupio_config: WUP_IO_WUP_CYC0_REG before low cycle change: 0x%08x\n",
		val);
	val &= ~(WUP_IO_DEB_CYC_VMASK
		 << (pin * WUP_IO_DEB_CYC_OFFSET + WUP_IO_DEB_CYC_SHIFT));
	val |= ((config->deb_cyc_l & WUP_IO_DEB_CYC_VMASK)
		<< (pin * WUP_IO_DEB_CYC_OFFSET + WUP_IO_DEB_CYC_SHIFT));
	DEBUG_PRINT(
		"wupio_config: WUP_IO_WUP_CYC0_REG after low cycle change: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_WUP_CYC0_REG);

	/* set debounce cycle for high level */
	val = hal_readl(WUP_IO_WUP_CYC1_REG);
	DEBUG_PRINT(
		"wupio_config: WUP_IO_WUP_CYC1_REG before high cycle change: 0x%08x\n",
		val);
	val &= ~(WUP_IO_DEB_CYC_VMASK
		 << (pin * WUP_IO_DEB_CYC_OFFSET + WUP_IO_DEB_CYC_SHIFT));
	val |= ((config->deb_cyc_h & WUP_IO_DEB_CYC_VMASK)
		<< (pin * WUP_IO_DEB_CYC_OFFSET + WUP_IO_DEB_CYC_SHIFT));
	DEBUG_PRINT(
		"wupio_config: WUP_IO_WUP_CYC1_REG after high cycle change: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_WUP_CYC1_REG);
}

static void wupio_init(wupio_init_t *init)
{
	uint32_t val;

	DEBUG_PRINT("wupio_init: Initializing WUPIO\n");

	/* clear pending */
	val = WUP_IO_MASK;
	DEBUG_PRINT(
		"wupio_init: Clearing pending interrupts, WUP_IO_STA_REG: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_STA_REG);

	/* set debounce clock */
	val = hal_readl(WUP_IO_DEB_CLK_REG);
	DEBUG_PRINT(
		"wupio_init: WUP_IO_DEB_CLK_REG before setting debounce clock: 0x%08x\n",
		val);
	val &= ~(WUP_IO_DEB_CLK_SET0_MASK | WUP_IO_DEB_CLK_SET1_MASK);
	val = (init->clk_sel_0 << WUP_IO_DEB_CLK_SET0_SHIFT) |
	      (init->clk_sel_1 << WUP_IO_DEB_CLK_SET1_SHIFT);
	DEBUG_PRINT(
		"wupio_init: WUP_IO_DEB_CLK_REG after setting debounce clock: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_DEB_CLK_REG);

	/* enbale gen interrupt */
	val = hal_readl(WUP_IO_WUP_GEN_REG);
	DEBUG_PRINT("wupio_init: WUP_IO_WUP_GEN_REG before setting: 0x%08x\n",
		    val);
	val |= WUP_IO_GEN_MASK;
	hal_writel(val, WUP_IO_WUP_GEN_REG);
	DEBUG_PRINT("wupio_init: WUP_IO_WUP_GEN_REG after setting: 0x%08x\n",
		    val);
}

static void wupio_set_hold(int pin, uint32_t en)
{
	uint32_t val;

	DEBUG_PRINT("wupio_set_hold: Setting hold for pin %d to %d\n", pin, en);

	en  = (en ? 1 : 0);
	val = hal_readl(WUP_IO_HOLD_REG);
	DEBUG_PRINT(
		"wupio_set_hold: WUP_IO_HOLD_REG before hold change: 0x%08x\n",
		val);
	val &= ~(1 << (pin + WUP_IO_HOLD_SHIFT));
	val |= (en << (pin + WUP_IO_HOLD_SHIFT));
	DEBUG_PRINT(
		"wupio_set_hold: WUP_IO_HOLD_REG after hold change: 0x%08x\n",
		val);
	hal_writel(val, WUP_IO_HOLD_REG);
	/* must delay 3 cycles of 32k clock */
	udelay(125);
}

static void wupio_enable(int pin, uint32_t en)
{
	uint32_t val;

	DEBUG_PRINT("wupio_enable: Enabling WUPIO pin %d with enable %d\n", pin,
		    en);

	en = (en ? 1 : 0);

	/* set wakeup detect */
	val = hal_readl(WUP_IO_EN_REG);
	DEBUG_PRINT(
		"wupio_enable: WUP_IO_EN_REG before enable change: 0x%08x\n",
		val);
	val &= ~(1 << (pin + WUP_IO_EN_SHIFT));
	val |= (en << (pin + WUP_IO_EN_SHIFT));
	DEBUG_PRINT("wupio_enable: WUP_IO_EN_REG after enable change: 0x%08x\n",
		    val);
	hal_writel(val, WUP_IO_EN_REG);
}

void sunxi_wupio_set(int wupio_pin, int wupio_pin_mode)
{
	wupio_init_t init;
	wupio_config_t config;

	wupio_set_hold(wupio_pin, 0);

	sunxi_gpio_wupio_set_mux(GPIO_PIN(GPIO_PORTL, wupio_pin), GPIO_INPUT);
	sunxi_gpio_wupio_set_pull(GPIO_PIN(GPIO_PORTL, wupio_pin),
				  GPIO_PULL_NONE);

	/* flush gpio hold */
	wupio_set_hold(wupio_pin, 1);

	/* init and set wupio clock period in 16ms (2^9*32.24us) */
	init.clk_sel_0 = 9;
	init.clk_sel_1 = 9;
	wupio_init(&init);

	/* config wupio */
	config.mode	 = wupio_pin_mode;
	config.deb_sel	 = WUPIO_DEBSEL_0;
	config.deb_cyc_h = 7;
	config.deb_cyc_l = 7;
	wupio_config(wupio_pin, &config);

	/* enable io wakeup */
	wupio_enable(wupio_pin, 1);
}

int do_sunxi_wupio(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int wupio_pin;
	int wupio_pin_mode;

	if (argc != 3) {
		DEBUG_PRINT(
			"param error\n\tsunxi_wupio <wupio_index> <wupio_pin_mode>\n");
		DEBUG_PRINT(
			"Usage: <wupio_index> is the pin index for the WUPIO, 0 to %d.\n",
			WUPIO_PIN_MAX);
		DEBUG_PRINT(
			"       <wupio_pin_mode> is the pin mode, 0:NEGATIVE_EDGE, 1:POSITIVE_EDGE.\n");
		DEBUG_PRINT("Example: sunxi_wupio 3 1\n");
		return -1;
	}

	if (argv[1]) {
		wupio_pin = simple_strtol(argv[1], NULL, 10);
		if (wupio_pin < 0 || wupio_pin > WUPIO_PIN_MAX) {
			DEBUG_PRINT("Invalid wupio_index: %d\n", wupio_pin);
			return -1;
		}
	} else {
		DEBUG_PRINT("Invalid argument: <wupio_index> is missing\n");
		return -1;
	}

	if (argv[2]) {
		wupio_pin_mode = simple_strtol(argv[2], NULL, 10);
		if (wupio_pin_mode < 0 || wupio_pin_mode > WUPIO_MODE_MAX) {
			DEBUG_PRINT(
				"Invalid wupio_pin_mode: %d, 0:NEGATIVE_EDGE, 1:POSITIVE_EDGE\n",
				wupio_pin_mode);
			return -1;
		}
	} else {
		DEBUG_PRINT("Invalid argument: <wupio_pin_mode> is missing\n");
		return -1;
	}

	sunxi_wupio_set(wupio_pin, wupio_pin_mode);
	return 0;
}

U_BOOT_CMD(sunxi_wupio, CONFIG_SYS_MAXARGS, 1, do_sunxi_wupio,
	   "sunxi wupio set for uboot", "<wupio_index> <wupio_pin_mode>");
