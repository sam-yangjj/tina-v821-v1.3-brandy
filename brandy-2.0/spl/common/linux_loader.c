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
#include <spare_head.h>
#include <android_image.h>
#include <private_uboot.h>
#include <u-boot/lz4.h>
#include <lzma/LzmaTools.h>
#include <arch/rtc.h>
#include <arch/clock.h>
#include <linux/errno.h>
#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
#include <libfdt.h>
#endif
#endif

#if 0
#define kernel_dbg(fmt, arg...) printf(fmt, ##arg)
#else
#define kernel_dbg(fmt, arg...)
#endif

//#define CALC_ANDROID_BOOT_IMAGE_CRC32
//#define CALC_LINUX_KERNEL_IMAGE_CRC32

#define ALIGN(x, a) __ALIGN_MASK((x), (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define SUNXI_FLASH_READ_FIRST_SIZE (4 * 1024)

int exist_uboot_jmp_cardproduct(phys_addr_t uboot_base)
{
	if (uboot_base) {
		uboot_head_t *header = (uboot_head_t *)uboot_base;
		/* fastboot scheme, uboot must jump to the card for mass production*/
		printf("workmode 0x%x\n", header->boot_data.work_mode);
		if (header->boot_data.work_mode == WORK_MODE_CARD_PRODUCT)
			boot0_jmp(uboot_base);
	}
	return 0;
}

#ifdef CFG_SUNXI_SUPPORT_RAMDISK
int android_image_get_ramdisk(const struct andr_img_hdr *hdr, ulong *rd_data,
			      ulong *rd_len)
{
	if (!hdr->ramdisk_size)
		return -1;
	*rd_data = (unsigned long)hdr;
	*rd_data += hdr->page_size;
	*rd_data += ALIGN(hdr->kernel_size, hdr->page_size);

	*rd_len = hdr->ramdisk_size;
	return 0;
}
#endif

#ifdef CFG_LOAD_DTB_FROM_KERNEL
extern __attribute__((section(".data"))) struct fdt_header *working_fdt;
int android_image_get_dtb(const struct andr_img_hdr *hdr,
			      ulong *dtb_data, ulong *dtb_len)
{
	if (!hdr->dtb_size)
		return -1;

	*dtb_data = (unsigned long)hdr;
	*dtb_data += hdr->page_size;
	*dtb_data += ALIGN(hdr->kernel_size, hdr->page_size);
	*dtb_data += ALIGN(hdr->ramdisk_size, hdr->page_size);
	*dtb_data += ALIGN(hdr->second_size, hdr->page_size);
	*dtb_data += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);

	*dtb_len = hdr->dtb_size;
	return 0;
}
#endif

int sunxi_mem_info(unsigned long mem_addr, unsigned long mem_len,
		   unsigned long other_mem_addr, unsigned long other_mem_len)
{
	/*
     *          |       access target          |
     *    | case 1|
     *                   |case 2|
     *                                     |case 3|
     *                                            |case 4|
     * |case 5|
     *  |                  case 6                     |
     * only 4 & 5 is ok
     */
	int ret = 0;
	if ((mem_addr >= other_mem_addr) && /*case 2 + case3*/
	    (mem_addr < other_mem_addr + other_mem_len)) {
		ret = -1;
	} else if ((mem_addr < other_mem_addr) && /*case 1*/
		   (mem_addr + mem_len >= other_mem_addr)) {
		ret = -1;
	} else if ((mem_addr < other_mem_addr) &&
		   (mem_addr + mem_len >=
		    other_mem_addr + other_mem_len)) { /*case 6*/
		ret = -1;
	}

	return ret;
}

#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT) && defined(CFG_SUNXI_SPIF)
extern void spif_unlock(void);
#endif

#if defined(CALC_ANDROID_BOOT_IMAGE_CRC32) || defined(CALC_LINUX_KERNEL_IMAGE_CRC32)
extern uint32_t crc32(uint32_t crc, const uint8_t *buf, uint len);
#endif

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
static uint32_t g_boot_img_addr, g_boot_img_size, g_boot_img_crc32;
#endif

#ifdef CALC_LINUX_KERNEL_IMAGE_CRC32
static uint32_t g_kernel_addr, g_kernel_size, g_kernel_crc32;
#endif

#define GZIP_MAGIC 0x8b1f
#define LZ4_MAGIC  0x184d2204
#define LZMA_MAGIC  0x005d

#ifndef CFG_PARALLEL_DECOMP_KERNEL
int load_bimage(u32 from_flash, u32 to_ddr,
		int (*flash_read)(uint, uint, void *), u32 *kernel_addr)
{
	int ret;
	u32 rbytes	= 0;
	u32 rblock	= 0;
	u32 start_block = from_flash;
	struct andr_img_hdr *fb_hdr;

	void *addr = (void *)(phys_addr_t)to_ddr;

	if (flash_read(start_block, (SUNXI_FLASH_READ_FIRST_SIZE / 512),
		       addr)) {
		printf("Fail to load kernel head\n");
		return -1;
	}

	fb_hdr = (struct andr_img_hdr *)addr;

	if (!memcmp(fb_hdr->magic, ANDR_BOOT_MAGIC, 8)) {
		kernel_dbg("kernel magic is ok\n");
		kernel_dbg("kernel_size = 0x%x\n", fb_hdr->kernel_size);
		kernel_dbg("ramdisk_size = 0x%x\n", fb_hdr->ramdisk_size);
		rbytes = fb_hdr->kernel_size + fb_hdr->ramdisk_size +
			 fb_hdr->second_size + 4 * 1024 + 511;
	} else {
		printf("kernel magic is error\n");
		return -1;
	}
	printf("kernel_addr:%x\n", fb_hdr->kernel_addr);

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
	g_boot_img_addr = to_ddr;
	g_boot_img_size = fb_hdr->page_size;
	g_boot_img_size += ALIGN(fb_hdr->kernel_size, fb_hdr->page_size);
	g_boot_img_size += ALIGN(fb_hdr->ramdisk_size, fb_hdr->page_size);
	g_boot_img_size += ALIGN(fb_hdr->second_size, fb_hdr->page_size);
	g_boot_img_size += ALIGN(fb_hdr->recovery_dtbo_size, fb_hdr->page_size);
	g_boot_img_size += ALIGN(fb_hdr->dtb_size, fb_hdr->page_size);
	rblock = g_boot_img_size / 512 - SUNXI_FLASH_READ_FIRST_SIZE / 512;
#else
	rblock = rbytes / 512 - SUNXI_FLASH_READ_FIRST_SIZE / 512;
#endif
	start_block += SUNXI_FLASH_READ_FIRST_SIZE / 512;
	addr = (void *)(phys_addr_t)((to_ddr) + SUNXI_FLASH_READ_FIRST_SIZE);
	kernel_dbg("rblock=%d, start=%d\n", rblock, start_block);

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
	printf("begin read android boot image: addr=0x%08x, size=%u(0x%08x)\n",
		g_boot_img_addr, g_boot_img_size, g_boot_img_size);
#endif

	ret = flash_read(start_block, rblock, addr);
	if (ret) {
		printf("load boot_image fail\n");
		return -1;
	}
	record_time("read_kernel");
	kernel_dbg("boot:%x %x\n", to_ddr, rbytes);
#ifdef CFG_KERNEL_CHECKSUM
	ret = verify_addsum_for_kernel((void *)to_ddr +
					       KERNEL_CODE_OFFSET_IN_BIMAGE,
				       fb_hdr->kernel_size, fb_hdr->checksum);
#endif
	if (ret == 0) {
#ifndef CFG_EARLY_FASTBOOT
#ifdef CFG_LOAD_DTB_FROM_KERNEL
		kernel_dbg("dtb_size = 0x%x\n", fb_hdr->dtb_size);
		kernel_dbg("dtb_addr = 0x%x\n", fb_hdr->dtb_addr);
		working_fdt = (struct fdt_header *)CFG_SUNXI_FDT_ADDR;
		if (fb_hdr->dtb_size) {
			ulong dtb_data = 0, dtb_len = 0;
			android_image_get_dtb(fb_hdr, &dtb_data, &dtb_len);
			ret = flash_read(from_flash + (dtb_data - (unsigned long)fb_hdr)/512,
					ALIGN(fb_hdr->dtb_size, 512)/512, (void *)CFG_SUNXI_FDT_ADDR);
			if (ret) {
				printf("load dtb fail\n");
				return -1;
			}
			record_time("load_dtb");
#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
			u32 fdt_ext_size = ALIGN(CFG_RESERVE_FDT_SIZE, 32);
			u32 new_fdt_totalsize = fdt_totalsize(working_fdt);
			if (new_fdt_totalsize > fdt_ext_size) {
				printf("new fdt(0x%08x) biger than now fdt(0x%08x)\n",
					new_fdt_totalsize, fdt_ext_size);
			} else {
				fdt_set_totalsize(working_fdt, fdt_ext_size);
			}
#endif
#endif
		}
#endif
#endif

#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT) && defined(CFG_SUNXI_SPIF)
		spif_unlock();
#endif

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
		rtc_set_data(CFG_FLASHFLAG_RTC_INDEX, rtc_read_data(CFG_FLASHFLAG_RTC_INDEX) | (1 << CFG_FLASHFLAG_RTC_BIT));
#endif
		if (fb_hdr->kernel_addr !=
		    to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE) {
			if (sunxi_mem_info((ulong)fb_hdr, rbytes,
					   fb_hdr->kernel_addr,
					   fb_hdr->kernel_size) < 0) {
				printf("Warning:Please check memory !!!\n");
				printf("boot.img:0x%lx-0x%lx\n", (ulong)fb_hdr,
				       (ulong)((ulong)fb_hdr + rbytes));
				printf("kernel_code:0x%lx-0x%lx\n",
				       (ulong)fb_hdr->kernel_addr,
				       (ulong)(fb_hdr->kernel_addr +
					       fb_hdr->kernel_size));
			}
#ifdef CFG_SUNXI_GUNZIP
			if ((*(unsigned int *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE)
				& 0xffff) == GZIP_MAGIC) {
				int ret = 0;
				unsigned long len = fb_hdr->kernel_size;
				unsigned long *lenp = &len;
				ret = gunzip((char *)(phys_addr_t)fb_hdr->kernel_addr,
					(fb_hdr->kernel_size * 3),
			(unsigned char *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE),
									lenp);
				if (ret) {
					printf("Error: gunzip returned %d\n", ret);
					return -1;
				}
				record_time("unzip_kernel");
			} else
#elif CFG_SUNXI_LZ4
			if (*(unsigned int *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE)
				 == LZ4_MAGIC) {
				int ret = 0;
				/* kernel size after uncompression must be less than len */
				unsigned long len = 0xa00000;
				unsigned long *lenp = &len;
				ret = ulz4fn((unsigned char *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE),
					fb_hdr->kernel_size,
					(char *)(phys_addr_t)fb_hdr->kernel_addr,
					(size_t *)lenp);
				if (ret) {
					printf("Error: ulz4 returned %d, len:0x%lx\n", ret, *lenp);
					return -1;
				}
				record_time("unlz4_kernel");

#ifdef CALC_LINUX_KERNEL_IMAGE_CRC32
				g_kernel_size = len;
#endif
			} else
#elif CFG_SUNXI_LZMA
			if (*(unsigned int *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE)
				 == LZMA_MAGIC) {
				int ret = 0;
				size_t dstlen = ~0U;

				ret = lzmaBuffToBuffDecompress((void *)(phys_addr_t)fb_hdr->kernel_addr, &dstlen,
					 (unsigned char *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE),
								  fb_hdr->kernel_size);
				if (ret) {
				    printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
				    return -1;
				}
				record_time("unlzma_kernel");
			} else
#endif
			{
				memcpy((void *)(phys_addr_t)fb_hdr->kernel_addr,
				       (void *)(phys_addr_t)(
					       to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE),
				       fb_hdr->kernel_size);
				record_time("move_kernel");
			}
			printf("kernel:%x %x\n", fb_hdr->kernel_addr,
				   fb_hdr->kernel_size);
		}

#ifdef CFG_SUNXI_SUPPORT_RAMDISK
		ulong rd_data, rd_len;
#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
		uint32_t bak_ramdisk_addr = 0;
		bak_ramdisk_addr = fb_hdr->ramdisk_addr;
#endif
		fb_hdr->ramdisk_addr = (ulong)CFG_RAMDISK_ADDR;

		if (fb_hdr->ramdisk_size) {
			android_image_get_ramdisk(fb_hdr, &rd_data, &rd_len);
			if (sunxi_mem_info((ulong)fb_hdr, rbytes,
					   fb_hdr->ramdisk_addr,
					   fb_hdr->ramdisk_size) < 0) {
				printf("Warning:Please check memory !!!\n");
				printf("boot.img:0x%lx-0x%lx\n", (ulong)fb_hdr,
				       (ulong)((ulong)fb_hdr + rbytes));
				printf("ramdisk:0x%lx-0x%lx\n",
				       (ulong)fb_hdr->ramdisk_addr,
				       (ulong)(fb_hdr->ramdisk_addr +
					       fb_hdr->ramdisk_size));
			}
			pr_emerg("load ramdisk from %0x to %x\n", rd_data, fb_hdr->ramdisk_addr);
			memcpy((void *)(long)fb_hdr->ramdisk_addr,
			       (const void *)rd_data, rd_len);
			record_time("load_ramdisk");
			/* ramdisk need to update dts, so load ramdisk after dtb*/
			int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end);
			ret = fdt_initrd((void *)CFG_SUNXI_FDT_ADDR, (ulong)fb_hdr->ramdisk_addr,
						(ulong)(fb_hdr->ramdisk_addr + rd_len));
			if (ret) {
				pr_emerg("update initrd fail\n");
				return -1;
			}
			record_time("update_initrd");
		}
#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
		fb_hdr->ramdisk_addr = bak_ramdisk_addr;
#endif
#endif
	}

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
	g_boot_img_crc32 = crc32(0, (uint8_t *)g_boot_img_addr, g_boot_img_size);
	printf("Android boot image: addr=0x%08x, size=%u(0x%08x), crc32=0x%08x\n",
		g_boot_img_addr, g_boot_img_size, g_boot_img_size, g_boot_img_crc32);
#endif

#ifdef CALC_LINUX_KERNEL_IMAGE_CRC32
	g_kernel_addr = fb_hdr->kernel_addr;
	g_kernel_crc32 = crc32(0, (uint8_t *)g_kernel_addr, g_kernel_size);
	printf("Linux kernel image: addr=0x%08x, size=%u(0x%08x), crc32=0x%08x\n",
		g_kernel_addr, g_kernel_size, g_kernel_size, g_kernel_crc32);
#endif

	*kernel_addr = fb_hdr->kernel_addr;
	return ret == 0 ? 0 : -1;
}
#else /* CFG_PARALLEL_DECOMP_KERNEL */

//#define LOAD_ALL_DATA_BEFORE_PARALLEL_DEC

#ifdef LOAD_ALL_DATA_BEFORE_PARALLEL_DEC
#define DEC_SIZE_ARR_LEN 23
static uint32_t g_dec_size_arr[DEC_SIZE_ARR_LEN] = {
	65536,
	65536,
	65536,
	65536,
	65536,
	65536,
	65536,
	65536,
	65536,
	131072,
	131072,
	131072,
	131072,
	131072,
	196608,
	196608,
	196608,
	196608,
	262144,
	262144,
	262144,
	327680,
	92160,
};
#endif

//#define RECORD_PARA_DEC_INFO

#ifdef RECORD_PARA_DEC_INFO
#define MAX_DATA_RECORD_CNT 32
typedef struct data_record {
	const char *name;
	uint32_t data_index;
	uint32_t data_buf[MAX_DATA_RECORD_CNT];
} data_record_t;

static void record_data(data_record_t *dr, uint32_t data)
{
	uint32_t *index, *buf;

	index = &dr->data_index;
	if (*index >= MAX_DATA_RECORD_CNT) {
		printf("'%s' record buf is full!\n", dr->name);
		return;
	}

	buf = dr->data_buf;
	buf[*index] = data;
	*index = *index + 1;
}

static void dump_data(data_record_t *dr)
{
	uint32_t i;

	printf("Dump recorder '%s', data_cnt=%u\n", dr->name, dr->data_index);
	for (i = 0; i < dr->data_index; i++) {
		printf("index: %u, data: %u\n", i, dr->data_buf[i]);
	}
}

static data_record_t g_dec_size_record = {
	.name = "decompress size",
};

static void record_dec_size(uint32_t data)
{
	record_data(&g_dec_size_record, data);
}

static void dump_dec_size(void)
{
	dump_data(&g_dec_size_record);
}
#endif

typedef struct compressed_kernel_image {
	uint32_t addr;
	uint32_t size;
	uint32_t kernel_load_addr;
} compressed_kernel_image_t;

typedef struct flash_read_ops {
	int (*read_async)(uint, uint, void *);
	int (*read_cnt)(void);
	int (*read_done)(void);
} flash_read_ops_t;

int para_dec_lz4_kernel_image(compressed_kernel_image_t *image, flash_read_ops_t *ops)
{
	int ret = 0;
	/* kernel size after uncompression must be less than len */
	unsigned long len = 0xa00000;
	uint32_t part_dec_cnt, part_dec_size, decompressed_size;
	lz4_part_dec_context_t ctx;

	lz4_part_dec_init(&ctx, (void *)image->addr, image->size,
		(void *)image->kernel_load_addr, (size_t *)&len);

	part_dec_cnt = 1;

	LZ4_DEBUG_LOG("before lz4_part_decompress");
	dump_lz4_part_dec_context(&ctx);

	ret = lz4_part_decompress(&ctx, SUNXI_FLASH_READ_FIRST_SIZE - KERNEL_CODE_OFFSET_IN_BIMAGE);

	LZ4_DEBUG_LOG("after lz4_part_decompress, ret: %d", ret);
	dump_lz4_part_dec_context(&ctx);

	if (ret < 0) {
		decompressed_size = lz4_part_dec_get_decompressed_size(&ctx);
		printf("lz4_part_decompress failed, ret: %d, dec_cnt: %d, decompressed size: %u(0x%08x)\n",
			ret, part_dec_cnt, decompressed_size, decompressed_size);
		while (1) {
		}
		return -1;
	}

#ifdef LOAD_ALL_DATA_BEFORE_PARALLEL_DEC
	while (1) {
		part_dec_cnt++;
		if ((part_dec_cnt - 1) > DEC_SIZE_ARR_LEN) {
			printf("dec_cnt(%u) reach the up limit! decompress exit\n", part_dec_cnt);
			break;
		}
		part_dec_size = g_dec_size_arr[part_dec_cnt - 2];

		printf("part_dec_size: %u\n", part_dec_size);

		LZ4_DEBUG_LOG("before lz4_part_decompress");
		dump_lz4_part_dec_context(&ctx);

		ret = lz4_part_decompress(&ctx, part_dec_size);

		LZ4_DEBUG_LOG("after lz4_part_decompress, ret: %d", ret);
		dump_lz4_part_dec_context(&ctx);

		if (ret < 0) {
			decompressed_size = lz4_part_dec_get_decompressed_size(&ctx);
			printf("lz4_part_decompress failed, ret: %d, dec_cnt: %d, decompressed size: %u(0x%08x)\n",
				ret, part_dec_cnt, decompressed_size, decompressed_size);
			while (1) {
			}
			return -1;
		}

		if (is_lz4_part_dec_finish(&ctx)) {
			printf("lz4 part decompress finish\n");
			break;
		}
	}

#else /* LOAD_ALL_DATA_BEFORE_PARALLEL_DEC */
	int tmp, read_cnt;

	tmp = 0;
	read_cnt = 0;
	while (1) {
		tmp = ops->read_cnt();
		if (tmp == -ENODATA) {
			printf("read done\r\n");
			break;
		}
		if (tmp < 0) {
			printf("flash_read_async fail, err=%d\n", tmp);
			return -1;
		}
		if (likely(tmp < image->size))
			tmp = tmp & ~(64 * 1024 - 1);
		else if (ops->read_done)
			ops->read_done();

		if (tmp <= read_cnt)
			continue;

		part_dec_cnt++;
		part_dec_size = tmp - read_cnt;
#ifdef RECORD_PARA_DEC_INFO
		record_dec_size(part_dec_size);
#endif
		ret = lz4_part_decompress(&ctx, part_dec_size);
		if (ret < 0) {
			decompressed_size = lz4_part_dec_get_decompressed_size(&ctx);
			printf("lz4_part_decompress failed, ret: %d, dec_cnt: %d, decompressed size: %u(0x%08x)\n",
				ret, part_dec_cnt, decompressed_size, decompressed_size);
			while (1) {
			}
			return -1;
		}
		read_cnt = tmp;
		if (is_lz4_part_dec_finish(&ctx))
			break;
	}
#endif /* LOAD_ALL_DATA_BEFORE_PARALLEL_DEC */

	record_time("unlz4_kernel");

#ifdef RECORD_PARA_DEC_INFO
	dump_dec_size();
#endif

	decompressed_size = lz4_part_dec_get_decompressed_size(&ctx);
	printf("dec_cnt: %d, decompressed size: %u(0x%08x)\n",
		part_dec_cnt, decompressed_size, decompressed_size);

#ifdef CALC_LINUX_KERNEL_IMAGE_CRC32
	g_kernel_size = len;
#endif

	return 0;
}

int load_bimage(u32 from_flash, u32 to_ddr,
		int (*flash_read)(uint, uint, void *), u32 *kernel_addr)
{
	int ret = 0;
	u32 rbytes	= 0;
	u32 rblock	= 0;
	u32 start_block = from_flash;
	struct andr_img_hdr *fb_hdr;
	__attribute__((unused)) flash_read_ops_t ops;
	compressed_kernel_image_t image;

#ifdef CFG_SUNXI_SPINOR
	ops.read_async = spinor_read_async;
	ops.read_cnt = spinor_get_read_cnt;
	ops.read_done = NULL;
#elif CFG_SUNXI_SDMMC
	ops.read_async = mmc_bread_ext_async;
	ops.read_cnt = mmc_bread_count;
	ops.read_done = mmc_wait_bread_ext_async_done;
#else
#error "CFG_PARALLEL_DECOMP_KERNEL unsupport"
#endif

	void *addr = (void *)(phys_addr_t)to_ddr;

	if (flash_read(start_block, (SUNXI_FLASH_READ_FIRST_SIZE / 512),
		       addr)) {
		printf("Fail to load kernel head\n");
		return -1;
	}

	fb_hdr = (struct andr_img_hdr *)addr;

	if (!memcmp(fb_hdr->magic, ANDR_BOOT_MAGIC, 8)) {
		kernel_dbg("kernel magic is ok\n");
		kernel_dbg("kernel_size = 0x%x\n", fb_hdr->kernel_size);
		kernel_dbg("ramdisk_size = 0x%x\n", fb_hdr->ramdisk_size);
		rbytes = fb_hdr->page_size;
		rbytes += ALIGN(fb_hdr->kernel_size, fb_hdr->page_size);
		rbytes += ALIGN(fb_hdr->ramdisk_size, fb_hdr->page_size);
		rbytes += ALIGN(fb_hdr->second_size, fb_hdr->page_size);
		rbytes += ALIGN(fb_hdr->recovery_dtbo_size, fb_hdr->page_size);
		rbytes += ALIGN(fb_hdr->dtb_size, fb_hdr->page_size);
	} else {
		printf("kernel magic is error\n");
		return -1;
	}
	printf("kernel_addr:%x\n", fb_hdr->kernel_addr);

	rblock = rbytes / 512 - SUNXI_FLASH_READ_FIRST_SIZE / 512;
	start_block += SUNXI_FLASH_READ_FIRST_SIZE / 512;
	addr = (void *)(phys_addr_t)((to_ddr) + SUNXI_FLASH_READ_FIRST_SIZE);
	kernel_dbg("rblock=%d, start=%d\n", rblock, start_block);

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
	g_boot_img_addr = to_ddr;
	g_boot_img_size = rbytes;
	printf("begin read android boot image: addr=0x%08x, size=%u(0x%08x)\n",
		g_boot_img_addr, g_boot_img_size, g_boot_img_size);
#endif

#ifdef LOAD_ALL_DATA_BEFORE_PARALLEL_DEC
	ret = flash_read(start_block, rblock, addr);
#else
	ret = ops.read_async(start_block, rblock, addr);
#endif
	if (ret) {
		printf("load boot_image fail\n");
		return -1;
	}

#ifdef CFG_AMP_DELEG_SPIF_IRQ
	rtc_set_data(CFG_DELEG_SPIF_IRQ_RTC_IDX, rtc_read_data(CFG_DELEG_SPIF_IRQ_RTC_IDX) & ~(1 << CFG_DELEG_SPIF_IRQ_RTC_BIT));
#endif
	record_time("read_kernel");
	kernel_dbg("boot image:%x %x\n", to_ddr, rbytes);

	image.addr = to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE;
	image.size = fb_hdr->kernel_size;
	image.kernel_load_addr = fb_hdr->kernel_addr;

	if (ret == 0) {
		if (fb_hdr->kernel_addr !=
		    to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE) {
			if (sunxi_mem_info((ulong)fb_hdr, rbytes,
					   fb_hdr->kernel_addr,
					   fb_hdr->kernel_size) < 0) {
				printf("Warning:Please check memory !!!\n");
				printf("boot.img:0x%lx-0x%lx\n", (ulong)fb_hdr,
				       (ulong)((ulong)fb_hdr + rbytes));
				printf("kernel_code:0x%lx-0x%lx\n",
				       (ulong)fb_hdr->kernel_addr,
				       (ulong)(fb_hdr->kernel_addr +
					       fb_hdr->kernel_size));
			}
#ifdef CFG_SUNXI_GUNZIP
			if ((*(unsigned int *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE)
				& 0xffff) == GZIP_MAGIC) {
				printf("Error: parallel gunzip unsupport\n");
				return -1;
			} else
#elif CFG_SUNXI_LZ4
			if (*(unsigned int *)(phys_addr_t)(to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE)
				 == LZ4_MAGIC) {
				ret = para_dec_lz4_kernel_image(&image, &ops);
				if (ret)
					printf("lz4 kernel image parallel decompress failed, ret: %d\n", ret);
			} else
#endif
			{
				memcpy((void *)(phys_addr_t)fb_hdr->kernel_addr,
				       (void *)(phys_addr_t)(
					       to_ddr + KERNEL_CODE_OFFSET_IN_BIMAGE),
				       fb_hdr->kernel_size);
				record_time("move_kernel");
			}
			printf("kernel:%x %x\n", fb_hdr->kernel_addr,
				   fb_hdr->kernel_size);
		}

#ifndef CFG_EARLY_FASTBOOT
#ifdef CFG_LOAD_DTB_FROM_KERNEL
		kernel_dbg("dtb_size = 0x%x\n", fb_hdr->dtb_size);
		kernel_dbg("dtb_addr = 0x%x\n", fb_hdr->dtb_addr);
		working_fdt = (struct fdt_header *)CFG_SUNXI_FDT_ADDR;
		if (fb_hdr->dtb_size) {
			ulong dtb_data = 0, dtb_len = 0;
			android_image_get_dtb(fb_hdr, &dtb_data, &dtb_len);
			memcpy((void *)CFG_SUNXI_FDT_ADDR, (const void *)dtb_data, dtb_len);
			record_time("load_dtb");
#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
			u32 fdt_ext_size = ALIGN(CFG_RESERVE_FDT_SIZE, 32);
			u32 new_fdt_totalsize = fdt_totalsize(working_fdt);
			if (new_fdt_totalsize > fdt_ext_size) {
				printf("new fdt(0x%08x) biger than now fdt(0x%08x)\n",
					new_fdt_totalsize, fdt_ext_size);
			} else {
				fdt_set_totalsize(working_fdt, fdt_ext_size);
			}
#endif
#endif
		}
#endif
#endif

#if defined(CFG_SPIF_HWLOCK) && defined(CFG_SUNXI_PMBOOT) && defined(CFG_SUNXI_SPIF)
		spif_unlock();
#endif

#ifdef CFG_BOOT0_WIRTE_RTC_TO_FLASH
		rtc_set_data(CFG_FLASHFLAG_RTC_INDEX, rtc_read_data(CFG_FLASHFLAG_RTC_INDEX) | (1 << CFG_FLASHFLAG_RTC_BIT));
#endif
#ifdef CFG_SUNXI_SUPPORT_RAMDISK
		ulong rd_data, rd_len;
#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
		uint32_t bak_ramdisk_addr = 0;
		bak_ramdisk_addr = fb_hdr->ramdisk_addr;
#endif
		fb_hdr->ramdisk_addr = (ulong)CFG_RAMDISK_ADDR;

		if (fb_hdr->ramdisk_size) {
			android_image_get_ramdisk(fb_hdr, &rd_data, &rd_len);
			if (sunxi_mem_info((ulong)fb_hdr, rbytes,
					   fb_hdr->ramdisk_addr,
					   fb_hdr->ramdisk_size) < 0) {
				printf("Warning:Please check memory !!!\n");
				printf("boot.img:0x%lx-0x%lx\n", (ulong)fb_hdr,
				       (ulong)((ulong)fb_hdr + rbytes));
				printf("ramdisk:0x%lx-0x%lx\n",
				       (ulong)fb_hdr->ramdisk_addr,
				       (ulong)(fb_hdr->ramdisk_addr +
					       fb_hdr->ramdisk_size));
			}
			pr_emerg("load ramdisk from %0x to %x\n", rd_data, fb_hdr->ramdisk_addr);
			memcpy((void *)(long)fb_hdr->ramdisk_addr,
			       (const void *)rd_data, rd_len);
			record_time("load_ramdisk");
			/* ramdisk need to update dts, so load ramdisk after dtb*/
			int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end);
			ret = fdt_initrd((void *)CFG_SUNXI_FDT_ADDR, (ulong)fb_hdr->ramdisk_addr,
						(ulong)(fb_hdr->ramdisk_addr + rd_len));
			if (ret) {
				pr_emerg("update initrd fail\n");
				return -1;
			}
			record_time("update_initrd");
		}
#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
		fb_hdr->ramdisk_addr = bak_ramdisk_addr;
#endif
#endif
	}

#ifdef CALC_ANDROID_BOOT_IMAGE_CRC32
	g_boot_img_crc32 = crc32(0, (uint8_t *)g_boot_img_addr, g_boot_img_size);
	printf("Android boot image: addr=0x%08x, size=%u(0x%08x), crc32=0x%08x\n",
		g_boot_img_addr, g_boot_img_size, g_boot_img_size, g_boot_img_crc32);
#endif

#ifdef CALC_LINUX_KERNEL_IMAGE_CRC32
	g_kernel_addr = fb_hdr->kernel_addr;
	g_kernel_crc32 = crc32(0, (uint8_t *)g_kernel_addr, g_kernel_size);
	printf("Linux kernel image: addr=0x%08x, size=%u(0x%08x), crc32=0x%08x\n",
		g_kernel_addr, g_kernel_size, g_kernel_size, g_kernel_crc32);
#endif

	*kernel_addr = fb_hdr->kernel_addr;
	return ret == 0 ? 0 : -1;
}
#endif /* !CFG_PARALLEL_DECOMP_KERNEL */

#ifdef CFG_KERNEL_UIMAGE
int load_uimage(u32 from_flash, u32 to_ddr, u32 image_size_KB,
		int (*flash_read)(uint, uint, void *), u32 *kernel_addr)
{
	int ret;
	u32 rbytes	= (image_size_KB * 1024);
	u32 rblock	= 0;
	u32 start_block = from_flash;

	void *addr = (void *)(phys_addr_t)(to_ddr);

	if (flash_read(start_block, (SUNXI_FLASH_READ_FIRST_SIZE / 512),
		       addr)) {
		printf("Fail to load kernel head\n");
		return -1;
	}

	rblock = rbytes / 512 - SUNXI_FLASH_READ_FIRST_SIZE / 512;
	start_block += SUNXI_FLASH_READ_FIRST_SIZE / 512;
	addr = (void *)(phys_addr_t)((to_ddr) + SUNXI_FLASH_READ_FIRST_SIZE);
	kernel_dbg("rblock=%d, start=%d\n", rblock, start_block);

	ret = flash_read(start_block, rblock, addr);

	*kernel_addr = to_ddr + KERNEL_CODE_OFFSET_IN_UIMAGE;
	return ret == 0 ? 0 : -1;
}
#endif
