/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom weidonghui <weidonghui@allwinnertech.com>
 */

#ifndef LIBFDT_INTERNAL_H
#define LIBFDT_INTERNAL_H
#include <fdt.h>

#define FDT_ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))
#define FDT_TAGALIGN(x) (FDT_ALIGN((x), FDT_TAGSIZE))

#define FDT_CHECK_HEADER(fdt)                                                  \
	{                                                                      \
		int err_ = fdt_check_header(fdt);                              \
		if (err_ != 0)                 				\
			return err_;                                           \
	}

int fdt_check_node_offset_(const void *fdt, int offset);
int fdt_check_prop_offset_(const void *fdt, int offset);
const char *fdt_find_string_(const char *strtab, int tabsize, const char *s);
int fdt_node_end_offset_(void *fdt, int nodeoffset);

static inline const void *fdt_offset_ptr_(const void *fdt, int offset)
{
	return (const char *)fdt + fdt_off_dt_struct(fdt) + offset;
}

static inline void *fdt_offset_ptr_w_(void *fdt, int offset)
{
	return (void *)(uintptr_t)fdt_offset_ptr_(fdt, offset);
}

static inline const struct fdt_reserve_entry *fdt_mem_rsv_(const void *fdt,
							   int n)
{
	const struct fdt_reserve_entry *rsv_table =
		(const struct fdt_reserve_entry *)((const char *)fdt +
						   fdt_off_mem_rsvmap(fdt));

	return rsv_table + n;
}
static inline struct fdt_reserve_entry *fdt_mem_rsv_w_(void *fdt, int n)
{
	return (void *)(uintptr_t)fdt_mem_rsv_(fdt, n);
}

#define FDT_SW_MAGIC (~FDT_MAGIC)

#endif /* LIBFDT_INTERNAL_H */
