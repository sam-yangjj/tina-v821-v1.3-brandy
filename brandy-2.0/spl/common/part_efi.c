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
#include <arch/spinor.h>
#include <part_efi.h>

static void *gpt_data_point;

static int is_gpt_init;
#if GPT_DEBUG
static int gpt_show_partition_info(char *buf)
{
	int i, j;

	char   char8_name[PARTNAME_SZ] = {0};
	gpt_header     *gpt_head  = (gpt_header *)(buf + GPT_HEAD_OFFSET);
	gpt_entry      *entry  = (gpt_entry *)(buf + GPT_ENTRY_OFFSET);

	printf("---------------------------\n");
	for (i = 0; i < gpt_head->num_partition_entries; i++) {
		for (j = 0; j < PARTNAME_SZ; j++) {
			char8_name[j] = (char)(entry[i].partition_name[j]);
		}
		printf("GPT:%s: 0x%lx  0x%lx\n", char8_name, (long)entry[i].starting_lba, (long)entry[i].ending_lba);
	}
	printf("---------------------------\n");

	return 0;
}
#endif

static int is_gpt_valid(void *buffer)
{
	gpt_header *gpt_head = (gpt_header *)(buffer + GPT_HEAD_OFFSET);

	if (gpt_head->signature != GPT_HEADER_SIGNATURE) {
		printf("gpt magic error, %llx != %llx\n", gpt_head->signature, GPT_HEADER_SIGNATURE);
		return 0;
	}
#if GPT_DEBUG
	gpt_show_partition_info(buffer);
#endif
	return 1;
}

int get_part_info_by_name(char *name, uint *start_sector, uint *sectors)
{
	if (!gpt_data_point) {
		printf("Still not initialized gpt,please initialize A first\n");
		return -1;
	}

	int i, j;

	char   char8_name[PARTNAME_SZ] = {0};
	gpt_header     *gpt_head  = (gpt_header *)(gpt_data_point + GPT_HEAD_OFFSET);
	gpt_entry      *entry  = (gpt_entry *)(gpt_data_point + GPT_ENTRY_OFFSET);

	if (gpt_head->signature != GPT_HEADER_SIGNATURE) {
		printf("gpt magic error, %llx != %llx\n", gpt_head->signature, GPT_HEADER_SIGNATURE);
		return -1;
	}

	for (i = 0; i < gpt_head->num_partition_entries; i++) {
		for (j = 0; j < PARTNAME_SZ; j++) {
			char8_name[j] = (char)(entry[i].partition_name[j]);
		}
		if (!strcmp(name, char8_name)) {
			*start_sector = (uint)entry[i].starting_lba;
			*sectors = ((uint)entry[i].ending_lba + 1) - (uint)entry[i].starting_lba;
			return 0;
		}
	}

	/* can't find the part */
	printf("can't find the part %s\n", name);
	return -1;
}

int init_gpt(void)
{
	void  *buffer = NULL;

	if (is_gpt_init) {
		//already init before
		return 0;
	}

	buffer = malloc(SUNXI_MBR_SIZE);
	if (!buffer) {
		printf("GPT: malloc faile\n");
		return -1;
	}
	memset(buffer, 0, SUNXI_MBR_SIZE);

	if (load_gpt(buffer)) {
		printf("load gpt fail\n");
		goto error;
	}

	if (is_gpt_valid(buffer)) {
		gpt_data_point = buffer;
	} else {
		printf("gpt data not valid\n");
		ndump(buffer, 1024);
		goto error;
	}

	is_gpt_init = 1;
	return 0;

error:
	free(buffer);
	return -1;
}

int deinit_gpt(void)
{
	if (gpt_data_point) {
		memset(gpt_data_point, 0, SUNXI_MBR_SIZE);
		free(gpt_data_point);
		gpt_data_point = NULL;
	}
	return 0;
}

