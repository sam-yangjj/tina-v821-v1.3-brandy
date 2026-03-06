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

#include <boot_param.h>
#include <common.h>
#include <lzma/LzmaTools.h>
#include <nand_boot0.h>
#include <private_atf.h>
#include <private_boot0.h>
#include <private_tee.h>
#include <private_toc.h>
#include <private_uboot.h>
#include <spare_head.h>
#include <u-boot/lz4.h>
#include <u-boot/zlib.h>

//#define BOOT_DEBUG

#ifdef CFG_SUNXI_FDT_ADDR
#ifdef CFG_RESERVE_FDT_SIZE
#include <cache_align.h>
#include <libfdt.h>
#endif
#endif

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

#if defined(CFG_SUNXI_GUNZIP) || defined(CFG_SUNXI_LZ4) ||                     \
    defined(CFG_SUNXI_LZMA)
static void update_opensbi_param(void *image_base)
{
    struct private_atf_head *opensbi_head =
	(struct private_atf_head *)image_base;

    if (strncmp((const char *)opensbi_head->magic, "opensbi", 7) == 0) {
	memcpy(opensbi_head->platform, &BT0_head.prvt_head.jtag_gpio[4], 8);
    }
}

static void *decomp_malloc_temp_buf(void)
{
#ifdef CONFIG_DECOMP_BUF_BY_MALLOC
    return malloc(CONFIG_DECOMPRESS_BUF_SIZE);
#else
    return (void *)0x40000000; /* hardcode here, as we can't get image_base from
				  gz header */
#endif
}

static void decomp_free_temp_buf(void *addr)
{
#ifdef CONFIG_DECOMP_BUF_BY_MALLOC
    return free(addr);
#endif
}
#endif

int load_comp_image(phys_addr_t *uboot_base, phys_addr_t *optee_base,
		    phys_addr_t *monitor_base, phys_addr_t *rtos_base,
		    phys_addr_t *opensbi_base, phys_addr_t *cpus_rtos_base,
		    phys_addr_t *dtb_base,
		    struct sbrom_toc1_item_info *toc1_item,
		    void __iomem *bootpkg_base)
{
    int ret = -1;
    if (toc1_item == NULL || bootpkg_base == NULL) {
	printf("[erro] input toc1_item or bootpkg_base is null \n");
	return -1;
    }
#ifdef CFG_SUNXI_GUNZIP
    else if ((strncmp(toc1_item->name, ITEM_UBOOT_GZ_NAME,
		      strlen(toc1_item->name)) == 0)) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	int dstlen = *(unsigned long *)((unsigned char *)bootpkg_base +
					toc1_item->data_offset +
					toc1_item->data_len - 4);
	unsigned char *src =
	    (unsigned char *)(bootpkg_base) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	unsigned long *lenp = &srclen;
	unsigned int l, m, h;
	phys_addr_t image_base;

	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = gunzip(dst, dstlen, src, lenp);
	if (ret) {
	    printf("Error: gunzip returned %d\n", ret);

	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}

	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*uboot_base = image_base;
	if (dst)
	    decomp_free_temp_buf(dst);

	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZ4
    else if ((strncmp(toc1_item->name, ITEM_UBOOT_LZ4_NAME,
		      strlen(toc1_item->name)) == 0)) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	unsigned int dstlen = 0x800000;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	unsigned int l, m, h;
	phys_addr_t image_base;
	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
	if (ret) {
	    printf("Error: ulz4fn returned %d\n", ret);
	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}

	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	printf("uboot run addr:%x \n", (void *)image_base);
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*uboot_base = image_base;
	if (dst)
	    decomp_free_temp_buf(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZMA
    else if (strncmp(toc1_item->name, ITEM_UBOOT_LZMA_NAME,
		     strlen(toc1_item->name)) == 0) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	size_t srclen = ~0U, dstlen = ~0U;
	srclen = toc1_item->data_len;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned int l, m, h;
	phys_addr_t image_base;
	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = lzmaBuffToBuffDecompress(dst, &dstlen, src, srclen);
	if (ret) {
	    printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}

	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	printf("uboot run addr:%x \n", (void *)image_base);
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*uboot_base = image_base;
	if (dst)
	    decomp_free_temp_buf(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZMA
    else if (strncmp(toc1_item->name, ITEM_DTB_LZMA_NAME,
		     strlen(ITEM_DTB_LZMA_NAME)) == 0) {
	phys_addr_t image_base = *uboot_base + 2 * 1024 * 1024;
	size_t srclen = ~0U, dstlen = ~0U;
	srclen = toc1_item->data_len;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	struct private_atf_head *atf_head =
	    (struct private_atf_head *)(*monitor_base);
	atf_head->dtb_base = image_base;
	printf("boot0 lzma dtb\n");
	int ret =
	    lzmaBuffToBuffDecompress((void *)image_base, &dstlen, src, srclen);
	if (ret) {
	    printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
	    return -1;
	}
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZ4
    else if ((strncmp(toc1_item->name, ITEM_DTB_LZ4_NAME,
		      strlen(ITEM_DTB_LZ4_NAME)) == 0)) {
	phys_addr_t image_base = *uboot_base + 2 * 1024 * 1024;
	unsigned int dstlen = 0x800000;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	int ret = ulz4fn(src, srclen, (void *)image_base, (size_t *)&dstlen);
	if (ret) {
	    printf("Error: ulz4fn returned %d\n", ret);
	    return -1;
	}
	return 0;
    }
#endif
#ifdef CFG_SUNXI_GUNZIP
    else if ((strncmp(toc1_item->name, ITEM_DTB_GZ_NAME,
		      strlen(ITEM_DTB_GZ_NAME)) == 0)) {
	phys_addr_t image_base = *uboot_base + 2 * 1024 * 1024;
	int dstlen = *(unsigned long *)((unsigned char *)CONFIG_BOOTPKG_BASE +
					toc1_item->data_offset +
					toc1_item->data_len - 4);
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	unsigned long *lenp = &srclen;
	int ret = gunzip((void *)image_base, dstlen, src, lenp);
	if (ret) {
	    printf("Error: gunzip returned %d\n", ret);
	    return -1;
	}
	return 0;
    }
#endif
#ifdef CFG_SUNXI_GUNZIP
    else if (strncmp(toc1_item->name, ITEM_OPENSBI_GZ_NAME,
		     sizeof(toc1_item->name)) == 0) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	int dstlen = *(unsigned long *)((unsigned char *)CONFIG_BOOTPKG_BASE +
					toc1_item->data_offset +
					toc1_item->data_len - 4);
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	unsigned long *lenp = &srclen;
	unsigned int l, m, h;
	phys_addr_t image_base;
	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = gunzip(dst, dstlen, src, lenp);
	if (ret) {
	    printf("Error: gunzip returned %d\n", ret);
	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}
	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	printf("opensbi run addr:%x \n", (void *)image_base);
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*opensbi_base = image_base;
	update_opensbi_param((void *)opensbi_base);
	if (dst)
	    decomp_free_temp_buf(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZ4
    else if (strncmp(toc1_item->name, ITEM_OPENSBI_LZ4_NAME,
		     sizeof(toc1_item->name)) == 0) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	unsigned int l, m, h;
	unsigned int dstlen = 0x800000;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	phys_addr_t image_base;
	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
	if (ret) {
	    printf("Error: ulz4fn returned %d\n", ret);
	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}
	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	printf("opensbi run addr:%x \n", (void *)image_base);
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*opensbi_base = image_base;
	update_opensbi_param((void *)opensbi_base);
	if (dst)
	    decomp_free_temp_buf(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZMA
    else if (strncmp(toc1_item->name, ITEM_OPENSBI_LZMA_NAME,
		     sizeof(toc1_item->name)) == 0) {
	void *dst =
	    decomp_malloc_temp_buf(); // malloc tmp memory for decompression
	size_t srclen = ~0U, dstlen = ~0U;
	unsigned int l, m, h;
	srclen = toc1_item->data_len;
	phys_addr_t image_base;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;

	if (!dst) {
	    printf("decompression dst addr is null \n");
	    return -1;
	}

	int ret = lzmaBuffToBuffDecompress((void *)dst, &dstlen, src, srclen);
	if (ret) {
	    printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
	    if (dst)
		decomp_free_temp_buf(dst);
	    return -1;
	}
	sunxi_image_head *image_head = (sunxi_image_head *)dst;
	image_base = image_head->run_addr;
	printf("opensbi run addr:%x \n", (void *)image_base);
	/* dst cant in [image_base - dstlen, image_base + dstlen] */
	l = (long)(image_base - dstlen);
	m = (long)dst;
	h = (long)(image_base + dstlen);
	if ((m > l) && (m < h)) {
	    memcpy((void *)(image_base + dstlen), dst, dstlen);
	    memcpy((void *)image_base, (void *)(image_base + dstlen), dstlen);
	} else {
	    memcpy((void *)image_base, dst, dstlen);
	}
	*opensbi_base = image_base;
	update_opensbi_param((void *)opensbi_base);
	if (dst)
	    decomp_free_temp_buf(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_GUNZIP
    else if ((strncmp(toc1_item->name, ITEM_MELIS_GZ_NAME,
		      sizeof(ITEM_MELIS_GZ_NAME)) == 0) ||
	     (strncmp(toc1_item->name, ITEM_RTOS_GZ_NAME,
		      sizeof(ITEM_RTOS_GZ_NAME)) == 0)) {
	phys_addr_t image_base = 0x40000000; /* hardcode here, as we can't get
						image_base from gz header */
	*rtos_base = image_base;
	void *dst = (void *)image_base;
	int dstlen = *(unsigned long *)((unsigned char *)CONFIG_BOOTPKG_BASE +
					toc1_item->data_offset +
					toc1_item->data_len - 4);
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	unsigned long *lenp = &srclen;
	int ret = gunzip(dst, dstlen, src, lenp);
	if (ret) {
	    printf("Error: gunzip returned %d\n", ret);
	    return -1;
	}
	update_opensbi_param(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZ4
    else if ((strncmp(toc1_item->name, ITEM_MELIS_LZ4_NAME,
		      sizeof(ITEM_MELIS_LZ4_NAME)) == 0) ||
	     (strncmp(toc1_item->name, ITEM_RTOS_LZ4_NAME,
		      sizeof(ITEM_RTOS_LZ4_NAME)) == 0)) {
	phys_addr_t image_base = 0x40000000; /* hardcode here, as we can't get
						image_base from lz4 header */
	*rtos_base = image_base;
	void *dst = (void *)image_base;
	unsigned int dstlen = 0x800000;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	unsigned long srclen = toc1_item->data_len;
	int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
	if (ret) {
	    printf("Error: ulz4fn returned %d\n", ret);
	    return -1;
	}
	update_opensbi_param(dst);
	return 0;
    }
#endif
#ifdef CFG_SUNXI_LZMA
    else if (strncmp(toc1_item->name, ITEM_MELIS_LZMA_NAME,
		     sizeof(ITEM_MELIS_LZMA_NAME)) == 0 ||
	     (strncmp(toc1_item->name, ITEM_RTOS_LZMA_NAME,
		      sizeof(ITEM_RTOS_LZMA_NAME)) == 0)) {
	phys_addr_t image_base = 0x40000000; /* hardcode here, as we can't get
						image_base from gz header */
	*rtos_base = image_base;
	size_t src_len = ~0U, dst_len = ~0U;
	void *dst = (void *)image_base;
	unsigned char *src =
	    (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
	int ret = lzmaBuffToBuffDecompress(dst, &src_len, src, dst_len);
	if (ret) {
	    printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
	    return -1;
	}
	update_opensbi_param(dst);
	return 0;
    }
#endif
    return ret;
}
