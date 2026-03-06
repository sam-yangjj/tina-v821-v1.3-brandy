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
 * (C) Copyright 2018
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <private_tee.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/rtc.h>
#include <arch/gpio.h>
#include <board_helper.h>
#include <config.h>
#include <boot_param.h>

__u8 uboot_backup;

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u16 pmu_type, u16 uart_input, u16 key_input);
static int boot0_clear_env(void);
__maybe_unused int load_kernel_from_spinor(u32 *);
__maybe_unused void startup_kernel(u32, u32);

static void print_commit_log(void)
{
#ifndef CFG_SHORT_COMMIT_LOG
	printf("HELLO! BOOT0 is starting!\n");
	printf("BOOT0 commit : %s\n", BT0_head.hash);
#else
	/* directly use sunxi_serial_putc to optimize startup speed */
	u32 time = get_sys_ticks();
	sunxi_serial_putc('[');
	sunxi_serial_putc('0' + (time % 100) / 10);
	sunxi_serial_putc('0' + time % 10);
	sunxi_serial_putc(']');
	sunxi_serial_putc('c');
	sunxi_serial_putc('o');
	sunxi_serial_putc('m');
	sunxi_serial_putc(':');
	sunxi_serial_putc(BT0_head.hash[0]);
	sunxi_serial_putc(BT0_head.hash[1]);
	sunxi_serial_putc(BT0_head.hash[2]);
	sunxi_serial_putc(BT0_head.hash[3]);
	sunxi_serial_putc('\r');
	sunxi_serial_putc('\n');
#endif
	sunxi_set_printf_debug_mode(BT0_head.prvt_head.debug_mode, 0);
}

void main(void)
{
	int dram_size;
	int status;
	phys_addr_t  uboot_base = 0, optee_base = 0, monitor_base = 0, \
				rtos_base = 0, opensbi_base = 0, cpus_rtos_base = 0, dtb_base = 0;
	u16 pmu_type = 0, key_input = 0; /* TODO: set real value */

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
	rtc_clear_data(CFG_FLASHFLAG_RTC_INDEX);
#endif

#if defined(CFG_SUNXI_SIMPLEBOOT) && defined(CFG_RECORD_BOOTTIME) && defined(CFG_BOOTTIME_RTC_IDX)
	/* copy simpleboot time data */
	if (rtc_read_data(CFG_BOOTTIME_RTC_IDX) != 0) {
		uint32_t *time = (uint32_t *)rtc_read_data(CFG_BOOTTIME_RTC_IDX);

		rtc_write_data(CFG_BOOTTIME_RTC_IDX, 0);
		boottime[0] = time[0];
		boottime[1] = time[1];
		boottime[2] = time[2];
		boottime[3] = time[3];
		boottime[4] = time[4];
		boottime[5] = time[5];
		boottime_desc[0] = "brom";
		boottime_desc[1] = "print";
		boottime_desc[2] = "board_init";
		boottime_desc[3] = "dram";
		boottime_desc[4] = "copy";
		boottime_desc[5] = "flush";
		boottime_pos = 6;
	}
#endif
	record_time("boot0");
	sunxi_board_init_early();
	record_time("bootinit early");
	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6);
	record_time("serial_init");
	print_commit_log();
	record_time("commit_log");

	status = sunxi_board_init();
	if (status)
		goto _BOOT_ERROR;
	record_time("board_init");

	if (rtc_probe_fel_flag()) {
		rtc_clear_fel_flag();
		goto _BOOT_ERROR;
#ifdef CFG_SUNXI_PHY_KEY
#ifdef CFG_LRADC_KEY
	} else if (check_update_key(&key_input)) {
		goto _BOOT_ERROR;
#elif CFG_GPIO_KEY
	} else if (check_update_gpiokey(&key_input)) {
		goto _BOOT_ERROR;
#endif
#endif
	}
	record_time("probe_fel_flag");

	if (BT0_head.prvt_head.enable_jtag) {
		printf("enable_jtag\n");
		boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 5, 1);
	}

	char uart_input_value = get_uart_input(); /* Prevent DRAM jamming */
	if (uart_input_value == '2') {
		printf("detected_f user input 2\n");
		goto _BOOT_ERROR;
	}
	record_time("serial_init");
	sunxi_bootparam_load();
	record_time("bootparam_load");
#ifndef CFG_SUNXI_SIMPLEBOOT
#ifdef FPGA_PLATFORM
	dram_size = sunxi_fpga_dram_init((void *)sunxi_bootparam_get_dram_buf());
#else
	dram_size = init_DRAM(0, (void *)sunxi_bootparam_get_dram_buf());
#endif
	if (!dram_size) {
		printf("init dram fail\n");
		goto _BOOT_ERROR;
	} else {
		if (BT0_head.dram_size > 0)
			dram_size = BT0_head.dram_size;
		record_time("init_DRAM");
		printf("dram size =%d\n", dram_size);
	}
#else
	dram_size = BT0_head.dram_size;
	printf("dram size =%d\n", dram_size);
#endif /* CFG_SUNXI_SIMPLEBOOT */
#ifdef CFG_SUNXI_STANDBY_WORKAROUND
	handler_super_standby();
#endif

	uart_input_value = get_uart_input();
	if (uart_input_value == '2') {
		sunxi_set_printf_debug_mode(3, 0);
		printf("detected_r user input 2\n");
		goto _BOOT_ERROR;
	} else if (uart_input_value == 'd') {
		sunxi_set_printf_debug_mode(8, 1);
		printf("detected user input d\n");
	} else if (uart_input_value == 'q') {
		printf("detected user input q\n");
		sunxi_set_printf_debug_mode(0, 1);
	}

	mmu_enable(dram_size);
	record_time("mmu_enable");
	malloc_init(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE);
	record_time("malloc_init");
	if (flash_init())
		goto _BOOT_ERROR;
	record_time("flash_init");
	status = sunxi_board_late_init();
	if (status)
		goto _BOOT_ERROR;
	record_time("board_late_init");

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
	rtc_set_data(CFG_FLASHFLAG_RTC_INDEX,
					rtc_read_data(CFG_FLASHFLAG_RTC_INDEX) & ~(1 << CFG_FLASHFLAG_RTC_BIT));
#endif
	status = load_package(&uboot_base);
	record_time("load_package");
	if (status == 0)
		load_image(&uboot_base, &optee_base, &monitor_base, &rtos_base, &opensbi_base, &cpus_rtos_base, &dtb_base);
	else
		goto _BOOT_ERROR;
	record_time("load_image");

	update_uboot_info(uboot_base, optee_base, monitor_base, rtos_base, dram_size,
			pmu_type, uart_input_value, key_input);
	record_time("update_uboot_info");

	if (load_and_run_fastboot(uboot_base, optee_base, monitor_base, rtos_base, opensbi_base, cpus_rtos_base))
		goto _BOOT_ERROR;

	mmu_disable();

	//hexdump((char *)(unsigned long)dtb_base, 0x40, "DTB");

#ifdef CFG_ARCH_RISCV
	if (opensbi_base) {
		printf("Jump to OpenSBI: opensbi_base = 0x%x, dtb_base = 0x%x, uboot_base = 0x%x\n",
			opensbi_base, dtb_base, uboot_base);
		record_time("jump_opensbi");
		dump_record_time();
		boot0_jmp_opensbi_v1x(0, dtb_base, opensbi_base, uboot_base);
	} else
#endif
	if (monitor_base) {
		struct spare_monitor_head *monitor_head =
			(struct spare_monitor_head *)((phys_addr_t)monitor_base);
		monitor_head->secureos_base = optee_base;
		monitor_head->nboot_base = uboot_base;
		printf("Jump to ATF: monitor_base = 0x%x, uboot_base = 0x%x, optee_base = 0x%x\n",
			monitor_base, uboot_base, optee_base);
		boot0_jmp_monitor(monitor_base);
	} else if (optee_base) {
		printf("Jump to OPTEE: optee_base = 0x%x, uboot_base = 0x%x\n", optee_base, uboot_base);
		boot0_jmp_optee(optee_base, uboot_base);
	} else if (rtos_base) {
		printf("Jump to RTOS: rtos_base = 0x%x\n", rtos_base);
		boot0_jmp(rtos_base);
	} else {
		printf("Jump to U-Boot: uboot_base = 0x%x\n", uboot_base);
		boot0_jmp(uboot_base);
	}

	printf("Jump to second boot failed! Stay in the loop...\n");
	while(1);

_BOOT_ERROR:
	printf("Failed preparing the second boot\n");
	boot0_clear_env();
	boot0_jmp(FEL_BASE);
}

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u16 pmu_type, u16 uart_input, u16 key_input)
{
	if (rtos_base)
		return;

	uboot_head_t  *header = (uboot_head_t *) uboot_base;
	struct sbrom_toc1_head_info *toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_BASE;

	if (uboot_base) {

#ifdef CFG_SUNXI_BOOT_PARAM
		sunxi_bootparam_format_and_transfer(
			(void *)(uboot_base + SUNXI_BOOTPARAM_OFFSET));
#endif

		header->boot_data.boot_package_size = toc1_head->valid_len;
		header->boot_data.dram_scan_size = dram_size;
		memcpy((void *)header->boot_data.dram_para,
			(void *)sunxi_bootparam_get_dram_buf(), 32 * sizeof(int));

		if (monitor_base)
			header->boot_data.monitor_exist = 1;

		if (optee_base) {
			struct spare_optee_head *tee_head =
				(struct spare_optee_head *)optee_base;
			header->boot_data.secureos_exist = 1;
			tee_head->dram_size		 = dram_size;
			tee_head->drm_size = BT0_head.secure_dram_mbytes;
			tee_head->uart_port = BT0_head.prvt_head.uart_port;
		}

#ifndef CONFIG_RISCV
		header->boot_data.func_mask |= get_uboot_func_mask(UBOOT_FUNC_MASK_ALL);
#endif
		update_flash_para(uboot_base);

		header->boot_data.uart_port = BT0_head.prvt_head.uart_port;
		memcpy((void *)header->boot_data.uart_gpio, BT0_head.prvt_head.uart_ctrl, 2*sizeof(normal_gpio_cfg));
		header->boot_data.pmu_type = pmu_type;
		header->boot_data.uart_input = uart_input;
		header->boot_data.key_input = key_input;
		header->boot_data.debug_mode = sunxi_get_debug_mode_for_uboot();
		if (get_card_work_mode() != NOUSE_CARDMODE)
			header->boot_data.work_mode = get_card_work_mode();
		header->boot_data.uboot_backup = uboot_backup;
	}
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mmu_disable();
	mdelay(10);

	return 0;
}

