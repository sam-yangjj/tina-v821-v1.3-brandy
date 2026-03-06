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
#ifndef _PRIVATE_TEE_HEAD_
#define _PRIVATE_TEE_HEAD_

struct HASH_INFO {
	uint8_t name[16];
	uint8_t hash[32];
};

#define SUNXI_SECURE_BOOT_INFO_MAGIC "BootInfo"
#define OPTEE_HAED_HASH_INFO_MAX (10)
struct sunxi_secure_boot_head {
	unsigned char magic[8]; /* ="BootInfo"  */
	unsigned int version;
	unsigned int hash_info_count;
	struct HASH_INFO toc1_hash_info[OPTEE_HAED_HASH_INFO_MAX];
	unsigned int reserved[131]; /* pad to 1020*/
};

struct spare_optee_head {
	unsigned int jump_instruction;
	unsigned char magic[8];
	unsigned int dram_size;
	unsigned int drm_size;
	unsigned int length;
	unsigned int optee_length;
	unsigned char version[8];
	unsigned char platform[8];
	unsigned int dram_para[32];
	unsigned char expand_magic[8];
	unsigned char expand_version[8];
	unsigned int vdd_cpua;
	unsigned int vdd_cpub;
	unsigned int vdd_sys;
	unsigned int vcc_pll;
	unsigned int vcc_io;
	unsigned int vdd_res1;
	unsigned int vdd_res2;
	unsigned int pmu_count;
	unsigned int pmu_port;
	unsigned int pmu_para;
	unsigned char pmu_id[4];
	unsigned char pmu_addr[4];
	unsigned char chipinfo[8];
	unsigned int placeholder[6]; /* see optee source code for specific functions*/
	unsigned char uart_port;
	unsigned char reserved1_char[3];
	unsigned int reserved[700];   /* pad to 3072, leave 1020 for secure boot info*/
	struct sunxi_secure_boot_head secure_boot_info;
};

#endif
