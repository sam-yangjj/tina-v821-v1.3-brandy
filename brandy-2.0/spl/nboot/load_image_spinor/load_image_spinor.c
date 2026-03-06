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
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <arch/spinor.h>
#include <sunxi_flashmap.h>
#ifdef CFG_SUNXI_GPT
#include <part_efi.h>
#endif

extern const boot0_file_head_t  BT0_head;
static int flash_inited;

#ifdef CFG_SUNXI_ENV
int spl_flash_init(void)
{
       return 0;
}

int spl_flash_read(uint start, uint sector_cnt, void *buffer)
{
#ifdef CFG_SUNXI_LOGIC_OFFSET
	return spinor_read(start + CFG_SUNXI_LOGIC_OFFSET, sector_cnt, buffer);
#else
	return spinor_read(start, sector_cnt, buffer);
#endif
}
#endif

#ifdef CFG_SUNXI_GPT
int load_gpt(void *buffer)
{
	uint sectors;
#ifdef CFG_SUNXI_LOGIC_OFFSET
	uint start_sector = 0;
#else
	uint start_sector = CFG_SPINOR_GPT_ARD;
#endif


	if (spl_flash_init()) {
		printf("spinor init fail\n");
		return -1;
	}

	sectors = 1 * SUNXI_MBR_SIZE / 512;
	printf("GPT load from sector:0x%x, sectors: 0x%x\n",
			start_sector, sectors);
	if (spl_flash_read(start_sector, sectors, buffer)) {
		printf("load GPT from spinor fail\n");
		return -1;
	}
	return 0;

}
#endif

#ifdef CFG_SUNXI_OTA_ENABLE
int load_toc1_ota_from_spinor(void)
{
	sbrom_toc1_head_info_t	*toc1_head;
	u8	*tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint start_sector = 0;
	uint sectors_num = 0;
	uint total_size = 0;
	int  ret = -1;
	char buff[64] = {0};
	char *boot_partation = buff;

	if (!flash_inited && spinor_init(0)) {
		printf("spinor init fail\n");
		goto __load_boot1_from_spinor_fail_0;
	}
	flash_inited = 1;

	if (init_gpt()) {
		printf("init gpt fail\n");
		goto __load_boot1_from_spinor_fail_0;
	}
	printf("init_gpt OK\n");

	if (fw_env_open()) {
		printf("fw_env_open failed\n");
		printf("now use the first boot\n");
		get_part_info_by_name("bootA", &start_sector, &sectors_num);
		start_sector = start_sector + CFG_SPINOR_GPT_ARD;
	} else {
		boot_partation = fw_getenv("boot_partation");
		if (strncmp(boot_partation, "NULL", strlen("NULL"))) {
			get_part_info_by_name(boot_partation, &start_sector, &sectors_num);
			start_sector = start_sector + CFG_SPINOR_GPT_ARD;
		} else {
			start_sector = 96;
		}
		fw_env_close();
	}

	if (spinor_read(start_sector, 1, (void *)tmp_buff)) {
		printf("the first data is error\n");
		goto __load_boot1_from_spinor_fail_1;
	}
	printf("Succeed in reading toc file head.\n");

	toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
	if (toc1_head->magic != TOC_MAIN_INFO_MAGIC) {
		printf("toc1 magic error\n");
		goto __load_boot1_from_spinor_fail_1;
	}
	total_size = toc1_head->valid_len;
	printf("The size of toc is %x.\n", total_size);

	if (spinor_read(start_sector, total_size/512, (void *)tmp_buff)) {
		printf("spinor read data error\n");
		goto __load_boot1_from_spinor_fail_1;
	}
	ret = 0;

__load_boot1_from_spinor_fail_1:
	deinit_gpt();
__load_boot1_from_spinor_fail_0:
	return ret;
}
#else
int load_toc1_from_spinor(void *storage_data)
{
	sbrom_toc1_head_info_t	*toc1_head;
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint total_size = 0;
	int start_sector = sunxi_flashmap_toc1_start(FLASHMAP_SPI_NOR);

	if(!flash_inited && spinor_init(0))
	{
		printf("spinor init fail\n");
		return -1;
	}
	flash_inited = 1;

	if(spinor_read(start_sector, 1, (void *)tmp_buff ) )
	{
		printf("the first data is error\n");
		goto __load_boot1_from_spinor_fail;
	}
	printf("Succeed in reading toc file head.\n");

	toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
	if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
	{
		printf("toc1 magic error\n");
		goto __load_boot1_from_spinor_fail;
	}
	total_size = toc1_head->valid_len;
	printf("The size of toc is %x.\n", total_size );

	if(spinor_read(start_sector, total_size/512, (void *)tmp_buff ))
	{
		printf("spinor read data error\n");
		goto __load_boot1_from_spinor_fail;
	}

	return 0;

__load_boot1_from_spinor_fail:

	return -1;
}

#endif
void update_flash_para(phys_addr_t uboot_base)
{
	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) uboot_base;
	bfh->boot_data.storage_type = STORAGE_NOR;
}

int load_package(phys_addr_t *uboot_base)
{
#ifdef CFG_SUNXI_OTA_ENABLE
	return load_toc1_ota_from_spinor();
#else
	return load_toc1_from_spinor((char *)BT0_head.prvt_head.storage_data);
#endif
}

int flash_init(void)
{
	if (!flash_inited && spinor_init(0)) {
		pr_emerg("spinor init fail\n");
		return -1;
	}
	flash_inited = 1;
	return 0;
}
