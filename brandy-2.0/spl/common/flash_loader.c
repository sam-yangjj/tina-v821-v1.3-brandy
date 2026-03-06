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
#include <part_efi.h>

#ifdef CFG_BOOT0_LOAD_ISPPARM
/*
 * CFG_BOOT0_LOAD_FLASH=y
 * CFG_BOOT0_LOAD_ISPPARM=y
 * CFG_ISPPARAM_LOAD_ADDR=0x43BFFE00
 * CFG_ISPPARAM_SIZE=0x8    #byte
 * CFG_SPINOR_ISPPARAM_OFFSET=CFG_SPINOR_UBOOT_OFFSET - 16
 */
#ifdef CFG_SUNXI_SPINOR
#if !defined(CFG_ISPPARAM_PARTITION)
#if !defined(CFG_ISPPARAM_LOAD_ADDR) || !defined(CFG_ISPPARAM_SIZE) ||         \
	!defined(CFG_SPINOR_ISPPARAM_OFFSET)
#error CFG_ISPPARAM_LOAD_ADDR CFG_ISPPARAM_SIZE CFG_ISPPARAM_SIZE \
CFG_SPINOR_ISPPARAM_OFFSET required for load ispparm
#endif
#endif

#define ISP_PARAM_OFFSET CFG_SPINOR_ISPPARAM_OFFSET
#endif /* CFG_SUNXI_SPINOR */

#ifdef CFG_SUNXI_SDMMC
#if !defined(CFG_ISPPARAM_PARTITION)
#if !defined(CFG_ISPPARAM_LOAD_ADDR) || !defined(CFG_ISPPARAM_SIZE) ||         \
	!defined(CFG_MMC_ISPPARAM_OFFSET)
#error CFG_ISPPARAM_LOAD_ADDR CFG_ISPPARAM_SIZE CFG_ISPPARAM_SIZE \
CFG_MMC_ISPPARAM_OFFSET required for load ispparm
#endif
#endif

#define ISP_PARAM_OFFSET CFG_MMC_ISPPARAM_OFFSET
#endif /* CFG_SUNXI_SDMMC */

#ifdef CFG_SUNXI_SPINAND
#if !defined(CFG_ISPPARAM_LOAD_ADDR) || !defined(CFG_NAND_ISPPARAM_START) ||   \
	!defined(CFG_ISPPARAM_SIZE)
#error CFG_ISPPARAM_LOAD_ADDR CFG_NAND_ISPPARAM_START CFG_ISPPARAM_SIZE required for load ispparm
#endif

#define ISP_PARAM_OFFSET CFG_NAND_ISPPARAM_START
#endif /* CFG_SUNXI_SPINAND */

#if !defined(CFG_SUNXI_SPINOR) && !defined(CFG_SUNXI_SDMMC) &&                 \
	!defined(CFG_SUNXI_SPINAND)
#error Only supports mmc, nand and nor
#endif

#if defined(CFG_ISPPARAM_PARTITION) && defined(CFG_SUNXI_GPT)
int load_isp_param(int (*flash_read)(uint, uint, void *))
{
	int load_flash_success = -1;
	uint start_sector;
	uint sector_num;

	if (init_gpt()) {
		printf("init GPT fail\n");
		return -1;
	}
	if (get_part_info_by_name("isp_param", &start_sector, &sector_num)) {
		return -1;
	}

	printf("from_flash = 0x%x, sector_num = %d, isp parm size = 0x%x, addr = 0x%x\n",
	       start_sector, sector_num, CFG_ISPPARAM_SIZE,
	       (void *)(phys_addr_t)CFG_ISPPARAM_LOAD_ADDR);

	load_flash_success =
		spl_flash_read(start_sector, CFG_ISPPARAM_SIZE,
			       (void *)(phys_addr_t)CFG_ISPPARAM_LOAD_ADDR);

#if defined(CFG_BOOT0_LOAD_ISP_IQ_FILE)
	if (init_gpt()) {
		printf("init GPT fail\n");
		return -1;
	}
	if (get_part_info_by_name("isp_iq_bin", &start_sector, &sector_num)) {
		return -1;
	}

	printf("from_flash = 0x%x, sector_num = %d, isp_iq_bin size = 0x%x, addr = 0x%x\n",
	       start_sector, sector_num, CFG_ISP_IQ_FILE_SIZE,
	       (void *)(phys_addr_t)CFG_ISP_IQ_FILE_LOAD_ADDR);

	load_flash_success =
		spl_flash_read(start_sector, CFG_ISP_IQ_FILE_SIZE,
			       (void *)(phys_addr_t)CFG_ISP_IQ_FILE_LOAD_ADDR);
#endif

	return load_flash_success;
}
#else
int load_isp_param(int (*flash_read)(uint, uint, void *))
{
	int load_flash_success = -1;

	printf("from_flash = 0x%x, isp parm size = 0x%x,addr = 0x%x\n",
	       ISP_PARAM_OFFSET, CFG_ISPPARAM_SIZE,
	       (void *)(phys_addr_t)CFG_ISPPARAM_LOAD_ADDR);
	load_flash_success =
		flash_read(ISP_PARAM_OFFSET / 512, CFG_ISPPARAM_SIZE / 512,
			   (void *)(phys_addr_t)CFG_ISPPARAM_LOAD_ADDR);

	return load_flash_success;
}
#endif
#endif