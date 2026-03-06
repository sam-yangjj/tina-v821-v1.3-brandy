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
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom wangwei <wangwei@allwinnertech.com>
 */

#ifndef _SYS_COMMON_H_
#define _SYS_COMMON_H_

#include <linux/types.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <config.h>
#include <arch/cpu.h>
#include <private_toc.h>

#define ESPACE(x)			_x

#define ARRAY_SIZE(x)			(sizeof(x) / sizeof((x)[0]))

#ifdef BOOT_DEBUG
#define _DEBUG				1
#define boot_info(fmt, args...)		printf(fmt, ##args)
#else
#define _DEBUG				0
#define boot_info(fmt, args...)
#endif

/* APIs that print out function (and line) */
#define pr_debug(fmt, args...)		boot_info("%s() %d: " fmt, __func__, __LINE__, ##args)
#define pr_info(fmt, args...)		printf("%s(): " fmt, __func__, ##args)
#define pr_err(fmt, args...)		printf("%s(): ERROR: " fmt, __func__, ##args)

#ifdef CFG_FPGA_PLATFORM
#define FPGA_PLATFORM
#endif

#define	SUNXI_CRYPT_SOFTWARE		(0xF1)
#define	SUNXI_CRYPT_HW			(0xF2)

#define BIT(nr)				(1UL << (nr))

void udelay(unsigned long);
void mdelay(unsigned long);
void sdelay(unsigned long loops);
u32 timer_get_us(void);
u32 get_sys_ticks(void);

void print_sys_tick(void);
int sunxi_set_printf_debug_mode(u8 debug_level, u8 to_uboot);
u8 sunxi_get_printf_debug_mode(void);
u8 sunxi_get_debug_mode_for_uboot(void);
void puts(const char *s);
int printf(const char *fmt, ...);
int printf_no_timestamp(const char *fmt, ...);
int pr_emerg(const char *fmt, ...);
int raw_printf(const char *fmt, ...);
void ndump(u8 *buf, int count);
void hexdump(const char *addr, unsigned int size, const char *name);
void __assert_fail(const char *assertion, const char *file, unsigned line,
		   const char *function);
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })



void mmu_enable(u32 dram_size);
void mmu_disable(void);
void v7_flush_dcache_all(void);
void flush_dcache_all(void);
void dcache_enable(void);
void dcache_disable(void);
void flush_dcache_range(unsigned long start, unsigned long end);
void invalidate_dcache_range(unsigned long start, unsigned long end);
void data_sync_barrier(void);

int sunxi_board_init(void);
int sunxi_nsi_init(void);
int sunxi_board_exit(void);

void boot0_jmp_monitor(phys_addr_t addr);
void boot0_jmp_optee(phys_addr_t optee, phys_addr_t uboot);
void boot0_jmp(phys_addr_t addr);
#ifdef CFG_ARCH_RISCV
void boot0_jmp_opensbi(phys_addr_t opensbi, phys_addr_t uboot);
void boot0_jmp_opensbi_v1x(unsigned long hartid, phys_addr_t dtb_base, phys_addr_t opensbi_base, phys_addr_t uboot_base);
#endif

int axp_init(u8 power_mode);
int axp_reg_write(u8 addr, u8 val);
int axp_reg_read(u8 addr, u8 *val);
int set_ddr_voltage(int set_vol);
int set_sys_voltage(int set_vol);
int set_sys_voltage_ext(char *name, int set_vol);
int close_sys_voltage_ext(char *name, int set_vol);
int set_camera_voltage_ext(char *name, int set_vol);
int set_ddr_voltage_ext(char *name, int set_vol, int on);
int set_voltage_ext(char *name, int set_vol, int on);
int set_pll_voltage(int set_vol);
int set_power_reset(void);
int probe_power_key(void);
int get_power_mode(void);
int get_power_source(void);
int get_pmu_exist(void);
void i2c_init(ulong i2c_base, int speed, int slaveaddr);
void i2c_exit(void);

int tcs4838_tcs_init(u8 power_mode);
int tcs4838_set_pll_voltage(int set_vol);
int sy8827g_ext_init(u8 power_mode);
int sy8827g_set_pll_voltage(int set_vol);

int check_update_key(u16 *key_input);
int check_update_gpiokey(u16 *key_input);

void __iomem *sunxi_get_iobase (unsigned int base);
unsigned int sunxi_get_lw32_addr(void *base);

char* strcpy(char *dest,const char *src);
char* strncpy(char *dest,const char *src,size_t count);
char* strcat(char *dest, const char *src);
char* strncat(char *dest, const char *src, size_t count);
int strcmp(const char *cs,const char *ct);
int strncmp(const char *cs,const char *ct,size_t count);
char* strchr(const char *s, int c);
size_t strlen(const char *s);
char* strrchr(const char *s, int c);
size_t strnlen(const char *s, size_t count);
size_t strspn(const char *s, const char *accept);
extern void* memset0(void *s, int c, size_t count);
void* memset(void *s, int c, size_t count);
void* memcpy0(void *dest, const void *src, size_t n);
void* memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *cs,const void *ct,size_t count);
void *memmove(void *dest, const void *src, size_t count);
void* memscan(void *addr, int c, size_t size);
char* strstr(const char *s1,const char *s2);
void* memchr(const void *s, int c, size_t n);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);


int malloc_init(u32 start, u32 size);
void* malloc(u32 size);
void* realloc(void *p, u32 size);
void  free(void *p);

int flash_init(void);
int load_package(phys_addr_t *uboot_base);
int load_image(phys_addr_t *uboot_base, phys_addr_t *optee_base, \
		phys_addr_t *monitor_base, phys_addr_t *rtos_base, phys_addr_t *opensbi_base, \
		 phys_addr_t *cpus_rtos_base, phys_addr_t *dtb_base);
int load_comp_image(phys_addr_t *uboot_base, phys_addr_t *optee_base, \
					phys_addr_t *monitor_base, phys_addr_t *rtos_base, \
					 phys_addr_t *opensbi_base, phys_addr_t *cpus_rtos_base, \
					  phys_addr_t *dtb_base, struct sbrom_toc1_item_info  *toc1_item, \
					   void __iomem *bootpkg_base);
unsigned int get_card_work_mode(void);
void update_flash_para(phys_addr_t uboot_base);
int verify_addsum(void *mem_base, u32 size);
u32 g_mod( u32 dividend, u32 divisor, u32 *quot_p);
char get_uart_input(void);

int sunxi_assert_arisc(void);
int sunxi_deassert_arisc(void);
void handler_super_standby(void);
int super_standby_mode(void);
void set_uboot_func_mask(uint8_t mask);
uint8_t get_uboot_func_mask(uint8_t mask);
uint8_t sunxi_board_late_init(void);
int sunxi_board_init_early(void);
int sunxi_fes_board_init_early(void);
int sunxi_pmboot_board_init_early(void);

void pattern_end(uint32_t pass);

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);
u32 get_boot_dram_training_enable_flag(const uint32_t *dram_para);

phys_addr_t elf_get_entry_addr(phys_addr_t base);
int load_elf_image(phys_addr_t img_addr);
void *elf_find_segment_offset(phys_addr_t elf_addr, const char *seg_name);
void *elf_find_segment_addr(phys_addr_t elf_addr, const char *seg_name);

void neon_enable(void);

int verify_addsum_for_kernel(void *mem_base, u32 size, u32 src_sum);
u32 sunxi_generate_checksum(void *buffer, uint length, uint div, uint src_sum);
int sunxi_verify_checksum(void *buffer, uint length, uint src_sum);
//kernel code is not from first byte of image
#define KERNEL_CODE_OFFSET_IN_UIMAGE 0x40
int load_uimage(u32 from_flash, u32 to_ddr, u32 image_size_KB,
		       int (*flash_read)(uint, uint, void *), u32 *kernel_addr);
#define KERNEL_CODE_OFFSET_IN_BIMAGE 0x800
int load_bimage(u32 from_flash, u32 to_ddr,
		int (*flash_read)(uint, uint, void *), u32 *kernel_addr);
void optee_jump_kernel(u32 optee_entry, u32 dtb_base, u32 kernel_addr);
void boot0_jump_kernel(u32 dtb_base, u32 kernel_addr);

/* mmc*/
#if defined(CFG_SUNXI_MMC) || defined(CFG_SUNXI_SDMMC)
int mmc_bread_ext(uint, uint, void *);
int mmc_bread_ext_async(uint, uint, void *);
int mmc_bread_count(void);
int mmc_wait_bread_ext_async_done(void);
int sunxi_mmc_init_ext(void);
int sunxi_mmc_cpu_read(uint start, uint blkcnt, void *dst);
#endif
/* spinor*/
int spinor_read(uint, uint, void *);
int spinor_read_async(uint start, uint sector_cnt, void *buffer);
int spinor_get_read_cnt(void);

int load_isp_param(int (*flash_read)(uint, uint, void *));
unsigned char read_switch_flag_from_kernel(void);
int exist_uboot_jmp_cardproduct(phys_addr_t uboot_base);
int load_and_run_fastboot(u32 uboot_base, u32 optee_base, u32 monitor_base,
		u32 rtos_base, u32 opensbi_base, u32 cpus_rtos_base);
void load_and_run_kernel(u32 optee_base, u32 opensbi_base, u32 monitor_base);

#ifdef CFG_EARLY_FASTBOOT
int gpt_env_init(void);
int early_fastboot_init(void);
int early_fastboot_load_dtb(void);
#ifdef CFG_EARLY_LOAD_MODEL
int early_fastboot_load_model(void);
#endif
#endif
#ifdef CFG_EARLY_BOOT_RISCV
int early_fastboot_boot_riscv(char *partition_name);
#endif

extern struct fdt_header *working_fdt;  /* Pointer to the working fdt */
#ifdef CFG_SUNXI_ENV
#define ENV_DEBUG 0
int fw_env_open(void);
int fw_env_close(void);
char *fw_getenv(char *name);
int fw_env_write(char *name, char *value);
int fw_env_flush(void);
void fw_env_dump(void);

int spl_flash_init(void);
int spl_flash_read(uint start, uint sector_cnt, void *buffer);
#endif

#ifdef CFG_SUNXI_GPT
#define GPT_DEBUG 0
int load_gpt(void *buffer);
#endif
extern bool flash_disable_dma;

void sunxi_dump(void *addr, unsigned int size);
void aw_set_hosc_freq(u32 hosc_freq);
u32 aw_get_hosc_freq(void);

#ifdef CFG_RECORD_BOOTTIME
#ifndef CFG_RECORD_BOOTTIME_MAX
#define CFG_RECORD_BOOTTIME_MAX			(32)
#endif

extern uint32_t boottime_pos;
extern uint32_t boottime[CFG_RECORD_BOOTTIME_MAX];
extern const char *boottime_desc[CFG_RECORD_BOOTTIME_MAX];

static inline void record_time(const char *desc)
{
	if (boottime_pos == CFG_RECORD_BOOTTIME_MAX)
		return;
#ifdef CFG_BOOTTIME_PREP_REG
	{
		u32 tmp = readl(CFG_BOOTTIME_PREP_REG);
		tmp |= (1 << CFG_BOOTTIME_PREP_BIT);
		writel(tmp, CFG_BOOTTIME_PREP_REG);
	}
#endif
	boottime[boottime_pos] = readl(CFG_BOOTTIME_BASE);
	boottime_desc[boottime_pos] = desc;
	boottime_pos++;
}

static inline void dump_record_time(void)
{
	int i;
	u32 freq = aw_get_hosc_freq();

	pr_emerg("timer freq: %u MHz\n", freq);
	for (i = 0; i < boottime_pos; i++)
		pr_emerg("\t%s: %u us\n", boottime_desc[i], boottime[i] / freq);
}
#else
static inline void record_time(const char *desc) { }
static inline void dump_record_time(void) { }
#endif /* CFG_RECORD_BOOTTIME */
#endif
