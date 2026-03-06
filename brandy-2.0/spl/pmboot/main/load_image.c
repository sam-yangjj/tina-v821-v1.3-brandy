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
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_tee.h>
#include <private_atf.h>
#include <u-boot/zlib.h>
#include <lzma/LzmaTools.h>
#include <u-boot/lz4.h>
#include <boot_param.h>

//#define BOOT_DEBUG

#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
#include <libfdt.h>
#include <cache_align.h>
#endif
#endif

#define TOC1_HEAD_NAME "sunxi-secure"

static unsigned int card_work_mode;

typedef struct _sunxi_image_head {
	unsigned int jump_instruction;
	unsigned char magic[MAGIC_SIZE];
	unsigned int res1;
	unsigned int res2;
	unsigned int res3;
	unsigned int res4;
	unsigned char res5[8];
	unsigned char res6[8];
	int run_addr;
} sunxi_image_head;

__attribute__((section(".data"))) struct fdt_header *working_fdt;

extern const boot0_file_head_t BT0_head;

static void update_opensbi_param(void *image_base)
{
	struct private_atf_head *opensbi_head =
		(struct private_atf_head *)image_base;

	if (strncmp((const char *)opensbi_head->magic, "opensbi", 7) == 0) {
		memcpy(opensbi_head->platform, &BT0_head.prvt_head.jtag_gpio[4],
		       8);
	}
}

int toc1_flash_read(u32 start_sector, u32 blkcnt, void *buff)
{
	void __iomem *addr =
		sunxi_get_iobase(CONFIG_BOOTPKG_BASE + 512 * start_sector);
	memcpy(buff, (addr), 512 * blkcnt);

	return blkcnt;
}

phys_addr_t toc1_get_image_addr(u32 start_sector)
{
	sunxi_image_head *image_head = (sunxi_image_head *)(sunxi_get_iobase(
		CONFIG_BOOTPKG_BASE + 512 * start_sector));

	return image_head->run_addr;
}

unsigned int get_card_work_mode(void)
{
	return card_work_mode;
}

static void set_card_work_mode(unsigned int mode)
{
	card_work_mode = mode;
}

int load_image(phys_addr_t *uboot_base, phys_addr_t *optee_base,
	       phys_addr_t *monitor_base, phys_addr_t *rtos_base,
	       phys_addr_t *opensbi_base, phys_addr_t *cpus_rtos_base,
	       phys_addr_t *dtb_base)
{
	int i;
	//int len;
	__maybe_unused void *dram_para_addr =
		(void *)sunxi_bootparam_get_dram_buf();
	phys_addr_t image_base;
	void __iomem *bootpkg_base = sunxi_get_iobase(CONFIG_BOOTPKG_BASE);

	struct sbrom_toc1_head_info *toc1_head = NULL;
	struct sbrom_toc1_item_info *item_head = NULL;

	struct sbrom_toc1_item_info *toc1_item = NULL;

	toc1_head = (struct sbrom_toc1_head_info *)bootpkg_base;
	item_head =
		(struct sbrom_toc1_item_info
			 *)(bootpkg_base + sizeof(struct sbrom_toc1_head_info));
#ifdef BOOT_DEBUG
	printf("*******************TOC1 Head Message*************************\n");
	printf("Toc_name          = %s\n", toc1_head->name);
	printf("Toc_magic         = 0x%x\n", toc1_head->magic);
	printf("Toc_add_sum       = 0x%x\n", toc1_head->add_sum);

	printf("Toc_serial_num    = 0x%x\n", toc1_head->serial_num);
	printf("Toc_status        = 0x%x\n", toc1_head->status);

	printf("Toc_items_nr      = 0x%x\n", toc1_head->items_nr);
	printf("Toc_valid_len     = 0x%x\n", toc1_head->valid_len);
	printf("TOC_MAIN_END      = 0x%x\n", toc1_head->end);
	printf("***************************************************************\n\n");
#endif
	//init
	toc1_item = item_head;
	set_card_work_mode(NOUSE_CARDMODE);
	if (!strncmp(toc1_head->name, TOC1_HEAD_NAME, strlen(TOC1_HEAD_NAME))) {
		//only nboot + secure toc1
		set_card_work_mode(WORK_MODE_CARD_PRODUCT);
	}

	for (i = 0; i < toc1_head->items_nr; i++, toc1_item++) {
#ifdef BOOT_DEBUG
		printf("\n*******************TOC1 Item Message*************************\n");
		printf("Entry_name        = %s\n", toc1_item->name);
		printf("Entry_data_offset = 0x%x\n", toc1_item->data_offset);
		printf("Entry_data_len    = 0x%x\n", toc1_item->data_len);

		printf("encrypt           = 0x%x\n", toc1_item->encrypt);
		printf("Entry_type        = 0x%x\n", toc1_item->type);
		printf("run_addr          = 0x%x\n", toc1_item->run_addr);
		printf("index             = 0x%x\n", toc1_item->index);
		printf("Entry_end         = 0x%x\n", toc1_item->end);
		printf("***************************************************************\n\n");
#else
		printf("Entry_name        = %s\n", toc1_item->name);
#endif

		if (toc1_item->type != 3) {
			//skip
			continue;
		}

		image_base = toc1_get_image_addr(toc1_item->data_offset / 512);
		if (strncmp(toc1_item->name, ITEM_UBOOT_NAME,
			    strlen(toc1_item->name)) == 0) {
			*uboot_base = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
		}
#ifdef CFG_SUNXI_GUNZIP
		else if ((strncmp(toc1_item->name, ITEM_UBOOT_GZ_NAME,
				  strlen(toc1_item->name)) == 0)) {
			void *dst =
				(void *)0x40000000; /* hardcode here, as we can't get image_base from gz header */

			int dstlen =
				*(unsigned long *)((unsigned char *)
							   CONFIG_BOOTPKG_BASE +
						   toc1_item->data_offset +
						   toc1_item->data_len - 4);
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			unsigned long *lenp  = &srclen;
			unsigned int l, m, h;
			int ret = gunzip(dst, dstlen, src, lenp);
			if (ret) {
				printf("Error: gunzip returned %d\n", ret);
				return -1;
			}

			sunxi_image_head *image_head = (sunxi_image_head *)dst;
			image_base		     = image_head->run_addr;
			/* dst cant in [image_base - dstlen, image_base + dstlen] */
			l = (long)(image_base - dstlen);
			m = (long)dst;
			h = (long)(image_base + dstlen);
			if ((m > l) && (m < h)) {
				memcpy((void *)(image_base + dstlen), dst,
				       dstlen);
				memcpy((void *)image_base,
				       (void *)(image_base + dstlen), dstlen);
			} else {
				memcpy((void *)image_base, dst, dstlen);
			}
			*uboot_base = image_base;
		}
#endif
#ifdef CFG_SUNXI_LZ4
		else if ((strncmp(toc1_item->name, ITEM_UBOOT_LZ4_NAME,
				  strlen(toc1_item->name)) == 0)) {
			void *dst =
				(void *)0x40000000; /* hardcode here, as we can't get image_base from lz4 header */
			unsigned int dstlen = 0x800000;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
			unsigned int l, m, h;
			if (ret) {
				printf("Error: ulz4fn returned %d\n", ret);
				return -1;
			}

			sunxi_image_head *image_head = (sunxi_image_head *)dst;
			image_base		     = image_head->run_addr;
			printf("uboot run addr:%x \n", (void *)image_base);
			/* dst cant in [image_base - dstlen, image_base + dstlen] */
			l = (long)(image_base - dstlen);
			m = (long)dst;
			h = (long)(image_base + dstlen);
			if ((m > l) && (m < h)) {
				memcpy((void *)(image_base + dstlen), dst,
				       dstlen);
				memcpy((void *)image_base,
				       (void *)(image_base + dstlen), dstlen);
			} else {
				memcpy((void *)image_base, dst, dstlen);
			}
			*uboot_base = image_base;

		}
#endif
#ifdef CFG_SUNXI_LZMA
		else if (strncmp(toc1_item->name, ITEM_UBOOT_LZMA_NAME,
				 strlen(toc1_item->name)) == 0) {
			void *dst =
				(void *)0x40000000; /* hardcode here, as we can't get image_base from gz header */
			size_t srclen = ~0U, dstlen = ~0U;
			srclen = toc1_item->data_len;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned int l, m, h;
			int ret = lzmaBuffToBuffDecompress(dst, &dstlen, src,
							   srclen);
			if (ret) {
				printf("Error: lzmaBuffToBuffDecompress returned %d\n",
				       ret);
				return -1;
			}

			sunxi_image_head *image_head = (sunxi_image_head *)dst;
			image_base		     = image_head->run_addr;
			printf("uboot run addr:%x \n", (void *)image_base);
			/* dst cant in [image_base - dstlen, image_base + dstlen] */
			l = (long)(image_base - dstlen);
			m = (long)dst;
			h = (long)(image_base + dstlen);
			if ((m > l) && (m < h)) {
				memcpy((void *)(image_base + dstlen), dst,
				       dstlen);
				memcpy((void *)image_base,
				       (void *)(image_base + dstlen), dstlen);
			} else {
				memcpy((void *)image_base, dst, dstlen);
			}
			*uboot_base = image_base;
		}
#endif
		else if (strncmp(toc1_item->name, ITEM_OPTEE_NAME,
				 sizeof(ITEM_OPTEE_NAME)) == 0) {
			*optee_base = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
			struct spare_optee_head *tee_head =
				(struct spare_optee_head *)image_base;
			memcpy(tee_head->dram_para,
			       sunxi_bootparam_get_dram_buf(),
			       32 * sizeof(int));
			memcpy(tee_head->chipinfo,
			       &BT0_head.prvt_head.jtag_gpio[4], 8);
		} else if (strncmp(toc1_item->name, ITEM_MONITOR_NAME,
				   sizeof(ITEM_MONITOR_NAME)) == 0) {
			*monitor_base = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
			struct private_atf_head *atf_head =
				(struct private_atf_head *)image_base;
			memcpy(atf_head->dram_para,
			       sunxi_bootparam_get_dram_buf(),
			       32 * sizeof(int));
			memcpy(atf_head->platform,
			       &BT0_head.prvt_head.jtag_gpio[4], 8);
#ifdef CFG_LOAD_DTB_FROM_KERNEL
			atf_head->dtb_base = CFG_SUNXI_FDT_ADDR;
#endif
		} else if (strncmp(toc1_item->name, ITEM_SCP_NAME,
				   sizeof(ITEM_SCP_NAME)) == 0) {
#ifdef SCP_SRAM_BASE
#ifndef CFG_EARLY_FASTBOOT
#ifdef SCP_DTS_BASE
			struct sbrom_toc1_item_info *toc1_item_scp_dts =
				item_head;
			int scp_j;
			for (scp_j = 0; scp_j < toc1_head->items_nr;
			     scp_j++, toc1_item_scp_dts++) {
				if (strncmp(toc1_item_scp_dts->name,
					    ITEM_DTB_NAME,
					    sizeof(ITEM_DTB_NAME)) == 0) {
					if (toc1_item_scp_dts->data_len >
					    SCP_DTS_SIZE) {
						printf("error: dtb size larger than scp dts size\n");
					} else {
						toc1_flash_read(
							toc1_item_scp_dts->data_offset /
								512,
							(toc1_item_scp_dts
								 ->data_len +
							 511) / 512,
							(void *)SCP_DTS_BASE);
					}
					break;
				}
			}
			if (scp_j == toc1_head->items_nr)
				printf("error: dtb not found for scp\n");
#endif
#endif
#ifdef SCP_DEASSERT_BY_MONITOR
			toc1_flash_read(toc1_item->data_offset / 512,
					(SCP_SRAM_SIZE + SCP_DRAM_SIZE + 511) /
						512,
					(void *)SCP_TEMP_STORE_BASE);
#else
			sunxi_assert_arisc();
			toc1_flash_read(toc1_item->data_offset / 512,
					(SCP_SRAM_SIZE + 511) / 512,
					(void *)SCP_SRAM_BASE);
			toc1_flash_read((toc1_item->data_offset +
					 SCP_CODE_DRAM_OFFSET) /
						512,
					(SCP_DRAM_SIZE + 511) / 512,
					(void *)SCP_DRAM_BASE);
			memcpy((void *)(SCP_SRAM_BASE + HEADER_OFFSET +
					SCP_DRAM_PARA_OFFSET),
			       dram_para_addr, SCP_DARM_PARA_NUM * sizeof(int));
			sunxi_deassert_arisc();
#endif
#endif
		} else if (strncmp(toc1_item->name, ITEM_DTB_NAME,
				   sizeof(toc1_item->name)) == 0) {
#ifndef CFG_LOAD_DTB_FROM_KERNEL
			/* note , uboot must be less than 2M */
#ifdef CFG_SUNXI_FDT_ADDR
			image_base = CFG_SUNXI_FDT_ADDR;
#else
			image_base = *uboot_base + 2 * 1024 * 1024;
#endif
			working_fdt = (struct fdt_header *)image_base;
			struct private_atf_head *atf_head =
				(struct private_atf_head *)(sunxi_get_iobase(
					*monitor_base));
			atf_head->dtb_base = image_base;
			*dtb_base	   = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
			u32 fdt_ext_size      = ALIGN(CFG_RESERVE_FDT_SIZE, 32);
			u32 new_fdt_totalsize = fdt_totalsize(working_fdt);
			if (new_fdt_totalsize > fdt_ext_size) {
				printf("new fdt(0x%08x) biger than now fdt(0x%08x)\n",
				       new_fdt_totalsize, fdt_ext_size);
			} else {
				fdt_set_totalsize(working_fdt, fdt_ext_size);
			}
#endif
#endif
#endif //CFG_LOAD_DTB_FROM_KERNEL
		}
#ifdef CFG_SUNXI_LZMA
		else if (strncmp(toc1_item->name, ITEM_DTB_LZMA_NAME,
				 strlen(ITEM_DTB_LZMA_NAME)) == 0) {
			image_base    = *uboot_base + 2 * 1024 * 1024;
			size_t srclen = ~0U, dstlen = ~0U;
			srclen = toc1_item->data_len;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			struct private_atf_head *atf_head =
				(struct private_atf_head *)(*monitor_base);
			atf_head->dtb_base = image_base;
			printf("boot0 lzma dtb\n");
			int ret = lzmaBuffToBuffDecompress(
				(void *)image_base, &dstlen, src, srclen);
			if (ret) {
				printf("Error: lzmaBuffToBuffDecompress returned %d\n",
				       ret);
				return -1;
			}
		}
#endif

#ifdef CFG_SUNXI_LZ4
		else if ((strncmp(toc1_item->name, ITEM_DTB_LZ4_NAME,
				  strlen(ITEM_DTB_LZ4_NAME)) == 0)) {
			image_base	    = *uboot_base + 2 * 1024 * 1024;
			unsigned int dstlen = 0x800000;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			int ret = ulz4fn(src, srclen, (void *)image_base,
					 (size_t *)&dstlen);
			if (ret) {
				printf("Error: ulz4fn returned %d\n", ret);
				return -1;
			}
		}
#endif

#ifdef CFG_SUNXI_GUNZIP
		else if ((strncmp(toc1_item->name, ITEM_DTB_GZ_NAME,
				  strlen(ITEM_DTB_GZ_NAME)) == 0)) {
			image_base = *uboot_base + 2 * 1024 * 1024;
			int dstlen =
				*(unsigned long *)((unsigned char *)
							   CONFIG_BOOTPKG_BASE +
						   toc1_item->data_offset +
						   toc1_item->data_len - 4);
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			unsigned long *lenp  = &srclen;
			int ret = gunzip((void *)image_base, dstlen, src, lenp);
			if (ret) {
				printf("Error: gunzip returned %d\n", ret);
				return -1;
			}
		}
#endif
		else if (strncmp(toc1_item->name, ITEM_DTBO_NAME,
				 sizeof(ITEM_DTBO_NAME)) == 0) {
			image_base = *uboot_base + 1 * 1024 * 1024;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
#ifdef CFG_BOOT0_LOGO_TO_KERNEL
		} else if (strncmp(toc1_item->name, ITEM_LOGO_NAME,
				   sizeof(ITEM_LOGO_NAME)) == 0) {
#if defined(CFG_SUNXI_FDT_ADDR) && defined(CFG_BOOTLOGO_LOAD_ADDR)
			image_base = CFG_BOOTLOGO_LOAD_ADDR;
#else
			image_base = *uboot_base + 3 * 1024 * 1024;
#endif
			*(uint *)(image_base) = toc1_item->data_len;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)(image_base));
			printf("load logo to kernel\n");
#else
		} else if (strncmp(toc1_item->name, ITEM_LOGO_NAME,
				   sizeof(ITEM_LOGO_NAME)) == 0) {
			image_base	      = *uboot_base + 3 * 1024 * 1024;
			*(uint *)(image_base) = toc1_item->data_len;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)(image_base + 16));
			set_uboot_func_mask(UBOOT_FUNC_MASK_BIT_BOOTLOGO);
#endif
		} else if (strncmp(toc1_item->name, ITEM_OPENSBI_NAME,
				   sizeof(ITEM_OPENSBI_NAME)) == 0) {
			*opensbi_base = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
			update_opensbi_param((void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_RTOS_NAME,
				   sizeof(ITEM_RTOS_NAME)) == 0 ||
			   strncmp(toc1_item->name, ITEM_MELIS_NAME,
				   sizeof(ITEM_MELIS_NAME)) == 0) {
			*rtos_base = image_base;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_MELIS_CONFIG_NAME,
				   sizeof(ITEM_MELIS_CONFIG_NAME)) == 0) {
			image_base =
				0x43000000; /* hardcode here, as we can't get image_base from gz header */
			printf("image_base:%x\n", image_base);
			memcpy((void *)image_base, &toc1_item->data_len,
			       sizeof(toc1_item->data_len));
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base + 512);
#if defined(CFG_RISCV_E907) || defined(CFG_RISCV_E906) ||                      \
	defined(CFG_RISCV_C906)
		} else if (strncmp(toc1_item->name, ITEM_MELIS_ELF_NAME,
				   sizeof(ITEM_MELIS_ELF_NAME)) == 0) {
#ifdef CFG_MELISELF_LOAD_ADDR
			image_base =
				CFG_MELISELF_LOAD_ADDR; /* hardcode here, as we can't get image_base from gz header */
#else
			image_base = *uboot_base + 4 * 1024 * 1024;
#endif
			/*printf("rv_base:%x\n", image_base);*/
			*cpus_rtos_base = image_base;
			//memcpy((void *)image_base, &toc1_item->data_len, sizeof(toc1_item->data_len));
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
#endif
#ifdef CFG_VIDEO_BIN
		} else if (strncmp(toc1_item->name, ITEM_VIDEO_DATA_NAME,
				   sizeof(ITEM_VIDEO_DATA_NAME)) == 0) {
#ifdef CFG_VIDEO_DATA_LOAD_ADDR
			image_base = CFG_VIDEO_DATA_LOAD_ADDR;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
#else
			printf("no find video data addr !!!\n");
#endif
#endif
#if defined(CFG_ISPPARAM_LOAD_ADDR)
		} else if (strncmp(toc1_item->name, ITEM_ISP_PARA,
				   sizeof(ITEM_ISP_PARA)) == 0) {
			image_base = CFG_ISPPARAM_LOAD_ADDR;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)image_base);
#endif
		}
#ifdef CFG_SUNXI_GUNZIP
		else if ((strncmp(toc1_item->name, ITEM_MELIS_GZ_NAME,
				  sizeof(ITEM_MELIS_GZ_NAME)) == 0) ||
			 (strncmp(toc1_item->name, ITEM_RTOS_GZ_NAME,
				  sizeof(ITEM_RTOS_GZ_NAME)) == 0)) {
			image_base =
				0x40000000; /* hardcode here, as we can't get image_base from gz header */
			*rtos_base = image_base;
			void *dst  = (void *)image_base;
			int dstlen =
				*(unsigned long *)((unsigned char *)
							   CONFIG_BOOTPKG_BASE +
						   toc1_item->data_offset +
						   toc1_item->data_len - 4);
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			unsigned long *lenp  = &srclen;
			int ret		     = gunzip(dst, dstlen, src, lenp);
			if (ret) {
				printf("Error: gunzip returned %d\n", ret);
				return -1;
			}
			update_opensbi_param(dst);
		}
#endif
#ifdef CFG_SUNXI_LZ4
		else if ((strncmp(toc1_item->name, ITEM_MELIS_LZ4_NAME,
				  sizeof(ITEM_MELIS_LZ4_NAME)) == 0) ||
			 (strncmp(toc1_item->name, ITEM_RTOS_LZ4_NAME,
				  sizeof(ITEM_RTOS_LZ4_NAME)) == 0)) {
			image_base =
				0x40000000; /* hardcode here, as we can't get image_base from lz4 header */
			*rtos_base	    = image_base;
			void *dst	    = (void *)image_base;
			unsigned int dstlen = 0x800000;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
			if (ret) {
				printf("Error: ulz4fn returned %d\n", ret);
				return -1;
			}
			update_opensbi_param(dst);
		}
#endif
#ifdef CFG_SUNXI_LZMA
		else if (strncmp(toc1_item->name, ITEM_MELIS_LZMA_NAME,
				 sizeof(ITEM_MELIS_LZMA_NAME)) == 0 ||
			 (strncmp(toc1_item->name, ITEM_RTOS_LZMA_NAME,
				  sizeof(ITEM_RTOS_LZMA_NAME)) == 0)) {
			image_base =
				0x40000000; /* hardcode here, as we can't get image_base from gz header */
			*rtos_base     = image_base;
			size_t src_len = ~0U, dst_len = ~0U;
			void *dst = (void *)image_base;
			unsigned char *src =
				(unsigned char *)(CONFIG_BOOTPKG_BASE) +
				toc1_item->data_offset;
			int ret = lzmaBuffToBuffDecompress(dst, &src_len, src,
							   dst_len);
			if (ret) {
				printf("Error: lzmaBuffToBuffDecompress returned %d\n",
				       ret);
				return -1;
			}
			update_opensbi_param(dst);
		}
#endif
	}

	return 0;
}
