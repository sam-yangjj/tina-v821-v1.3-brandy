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
#include <libfdt.h>
#include <common.h>

#ifdef CFG_SUNXI_SUPPORT_RAMDISK
#define RAMDISK_DEBUG 0

#if RAMDISK_DEBUG
#define rd_dbg(fmt, arg...) printf("%s()%d - " fmt, __func__, __LINE__, ##arg)
#else
#define rd_dbg(fmt, arg...)
#endif
static inline int fdt_setprop_uxx(void *fdt, int nodeoffset, const char *name,
				  uint64_t val, int is_u64)
{
	if (is_u64)
		return fdt_setprop_u64(fdt, nodeoffset, name, val);
	else
		return fdt_setprop_u32(fdt, nodeoffset, name, (uint32_t)val);
}

int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end)
{
	int   nodeoffset;
	int   err, j, total;
	int is_u64;
	uint64_t addr, size;

	/* just return if the size of initrd is zero */
	if (initrd_start == initrd_end)
		return 0;

	/* find or create "/chosen" node. */
	//nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	nodeoffset = fdt_path_offset(working_fdt, "/chosen");
	if (nodeoffset < 0)
		return nodeoffset;

	total = fdt_num_mem_rsv(fdt);

	/*
	 * Look for an existing entry and update it.  If we don't find
	 * the entry, we will j be the next available slot.
	 */
	for (j = 0; j < total; j++) {
		err = fdt_get_mem_rsv(fdt, j, &addr, &size);
		if (addr == initrd_start) {
			fdt_del_mem_rsv(fdt, j);
			break;
		}
	}

	err = fdt_add_mem_rsv(fdt, initrd_start, initrd_end - initrd_start);
	if (err < 0) {
		rd_dbg("fdt_initrd: %s\n", fdt_strerror(err));
		return err;
	}

	is_u64 = (fdt_address_cells(fdt, 0) == 2);

	err = fdt_setprop_uxx(fdt, nodeoffset, "linux,initrd-start",
			      (uint64_t)initrd_start, is_u64);

	if (err < 0) {
		rd_dbg("WARNING: could not set linux,initrd-start %s.\n",
		       fdt_strerror(err));
		return err;
	}

	err = fdt_setprop_uxx(fdt, nodeoffset, "linux,initrd-end",
			      (uint64_t)initrd_end, is_u64);

	if (err < 0) {
		rd_dbg("WARNING: could not set linux,initrd-end %s.\n",
		       fdt_strerror(err));

		return err;
	}

	return 0;
}

#endif
