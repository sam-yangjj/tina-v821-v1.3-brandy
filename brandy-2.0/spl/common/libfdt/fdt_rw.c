/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom weidonghui <weidonghui@allwinnertech.com>
 */

#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

static int fdt_blocks_misordered_(const void *fdt, int mem_rsv_size,
				  int struct_size)
{
	return (fdt_off_mem_rsvmap(fdt) <
		FDT_ALIGN(sizeof(struct fdt_header), 8)) ||
	       (fdt_off_dt_struct(fdt) <
		(fdt_off_mem_rsvmap(fdt) + mem_rsv_size)) ||
	       (fdt_off_dt_strings(fdt) <
		(fdt_off_dt_struct(fdt) + struct_size)) ||
	       (fdt_totalsize(fdt) <
		(fdt_off_dt_strings(fdt) + fdt_size_dt_strings(fdt)));
}

static int fdt_rw_check_header_(void *fdt)
{
	FDT_CHECK_HEADER(fdt);

	if (fdt_version(fdt) < 17)
		return -FDT_ERR_BADVERSION;
	if (fdt_blocks_misordered_(fdt, sizeof(struct fdt_reserve_entry),
				   fdt_size_dt_struct(fdt)))
		return -FDT_ERR_BADLAYOUT;
	if (fdt_version(fdt) > 17)
		fdt_set_version(fdt, 17);

	return 0;
}

#define FDT_RW_CHECK_HEADER(fdt)                                               \
	{                                                                      \
		int err_ = fdt_rw_check_header_(fdt);                                                      \
		if (err_ != 0)                   \
			return err_;                                           \
	}

static inline int fdt_data_size_(void *fdt)
{
	return fdt_off_dt_strings(fdt) + fdt_size_dt_strings(fdt);
}

static int fdt_splice_(void *fdt, void *splicepoint, int oldlen, int newlen)
{
	char *p   = splicepoint;
	char *end = (char *)fdt + fdt_data_size_(fdt);

	if (((p + oldlen) < p) || ((p + oldlen) > end))
		return -FDT_ERR_BADOFFSET;
	if ((p < (char *)fdt) || ((end - oldlen + newlen) < (char *)fdt))
		return -FDT_ERR_BADOFFSET;
	if ((end - oldlen + newlen) > ((char *)fdt + fdt_totalsize(fdt)))
		return -FDT_ERR_NOSPACE;
	fdt_memmove(p + newlen, p + oldlen, end - p - oldlen);
	return 0;
}

static int fdt_splice_mem_rsv_(void *fdt, struct fdt_reserve_entry *p, int oldn,
			       int newn)
{
	int delta = (newn - oldn) * sizeof(*p);
	int err;
	err = fdt_splice_(fdt, p, oldn * sizeof(*p), newn * sizeof(*p));
	if (err)
		return err;
	fdt_set_off_dt_struct(fdt, fdt_off_dt_struct(fdt) + delta);
	fdt_set_off_dt_strings(fdt, fdt_off_dt_strings(fdt) + delta);
	return 0;
}

static int fdt_splice_struct_(void *fdt, void *p, int oldlen, int newlen)
{
	int delta = newlen - oldlen;
	int err = fdt_splice_(fdt, p, oldlen, newlen);

	if (err)
		return err;

	fdt_set_size_dt_struct(fdt, fdt_size_dt_struct(fdt) + delta);
	fdt_set_off_dt_strings(fdt, fdt_off_dt_strings(fdt) + delta);
	return 0;
}

static int fdt_splice_string_(void *fdt, int newlen)
{
	void *p = (char *)fdt + fdt_off_dt_strings(fdt) +
		  fdt_size_dt_strings(fdt);
	int err = fdt_splice_(fdt, p, 0, newlen);

	if (err)
		return err;

	fdt_set_size_dt_strings(fdt, fdt_size_dt_strings(fdt) + newlen);
	return 0;
}

static int fdt_find_add_string_(void *fdt, const char *s)
{
	char *strtab = (char *)fdt + fdt_off_dt_strings(fdt);
	const char *p;
	char *new;
	int len = strlen(s) + 1;
	int err;

	p = fdt_find_string_(strtab, fdt_size_dt_strings(fdt), s);
	if (p)
		/* found it */
		return (p - strtab);

	new = strtab + fdt_size_dt_strings(fdt);
	err = fdt_splice_string_(fdt, len);
	if (err)
		return err;

	memcpy(new, s, len);
	return (new - strtab);
}

int fdt_add_mem_rsv(void *fdt, uint64_t address, uint64_t size)
{
	struct fdt_reserve_entry *re;
	int err;

	FDT_RW_CHECK_HEADER(fdt);

	re  = fdt_mem_rsv_w_(fdt, fdt_num_mem_rsv(fdt));
	err = fdt_splice_mem_rsv_(fdt, re, 0, 1);
	if (err)
		return err;

	re->address = cpu_to_fdt64(address);
	re->size    = cpu_to_fdt64(size);
	return 0;
}

int fdt_del_mem_rsv(void *fdt, int n)
{
	struct fdt_reserve_entry *re = fdt_mem_rsv_w_(fdt, n);

	FDT_RW_CHECK_HEADER(fdt);

	if (n >= fdt_num_mem_rsv(fdt))
		return -FDT_ERR_NOTFOUND;

	return fdt_splice_mem_rsv_(fdt, re, 1, 0);
}

static int fdt_resize_property_(void *fdt, int nodeoffset, const char *name,
				int len, struct fdt_property **prop)
{
	int oldlen;
	int err;

	*prop = fdt_get_property_w(fdt, nodeoffset, name, &oldlen);
	if (!*prop)
		return oldlen;
	err = fdt_splice_struct_(fdt, (*prop)->data, FDT_TAGALIGN(oldlen), FDT_TAGALIGN(len));
	if (err)
		return err;

	(*prop)->len = cpu_to_fdt32(len);
	return 0;
}

static int fdt_add_property_(void *fdt, int nodeoffset, const char *name,
			     int len, struct fdt_property **prop)
{
	int proplen;
	int nextoffset = fdt_check_node_offset_(fdt, nodeoffset);
	int namestroff;
	int err;

	if (nextoffset < 0)
		return nextoffset;

	namestroff = fdt_find_add_string_(fdt, name);
	if (namestroff < 0)
		return namestroff;

	*prop   = fdt_offset_ptr_w_(fdt, nextoffset);
	proplen = sizeof(**prop) + FDT_TAGALIGN(len);

	err = fdt_splice_struct_(fdt, *prop, 0, proplen);
	if (err)
		return err;

	(*prop)->tag     = cpu_to_fdt32(FDT_PROP);
	(*prop)->nameoff = cpu_to_fdt32(namestroff);
	(*prop)->len     = cpu_to_fdt32(len);
	return 0;
}

int fdt_set_name(void *fdt, int nodeoffset, const char *name)
{
	char *namep;
	int oldlen, newlen;
	int err;

	FDT_RW_CHECK_HEADER(fdt);

	namep = (char *)(uintptr_t)fdt_get_name(fdt, nodeoffset, &oldlen);
	if (!namep)
		return oldlen;

	newlen = strlen(name);

	err = fdt_splice_struct_(fdt, namep, FDT_TAGALIGN(oldlen + 1),
				 FDT_TAGALIGN(newlen + 1));
	if (err)
		return err;

	memcpy(namep, name, newlen + 1);
	return 0;
}

int fdt_setprop_placeholder(void *fdt, int nodeoffset, const char *name,
			    int len, void **prop_data)
{
	struct fdt_property *prop;
	int err;

	FDT_RW_CHECK_HEADER(fdt);

	err = fdt_resize_property_(fdt, nodeoffset, name, len, &prop);
	if (err == -FDT_ERR_NOTFOUND)
		err = fdt_add_property_(fdt, nodeoffset, name, len, &prop);
	if (err)
		return err;

	*prop_data = prop->data;
	return 0;
}

int fdt_setprop(void *fdt, int nodeoffset, const char *name, const void *val,
		int len)
{
	void *prop_data;
	int err;

	err = fdt_setprop_placeholder(fdt, nodeoffset, name, len, &prop_data);
	if (err)
		return err;

	if (len)
		memcpy(prop_data, val, len);
	return 0;
}

int fdt_appendprop(void *fdt, int nodeoffset, const char *name, const void *val,
		   int len)
{
	struct fdt_property *prop;
	int err, oldlen, newlen;

	FDT_RW_CHECK_HEADER(fdt);

	prop = fdt_get_property_w(fdt, nodeoffset, name, &oldlen);
	if (prop) {
		newlen = len + oldlen;
		err = fdt_splice_struct_(fdt, prop->data, FDT_TAGALIGN(oldlen),
					 FDT_TAGALIGN(newlen));
		if (err)
			return err;
		prop->len = cpu_to_fdt32(newlen);
		memcpy(prop->data + oldlen, val, len);
	} else {
		err = fdt_add_property_(fdt, nodeoffset, name, len, &prop);
		if (err)
			return err;
		memcpy(prop->data, val, len);
	}
	return 0;
}

int fdt_delprop(void *fdt, int nodeoffset, const char *name)
{
	struct fdt_property *prop;
	int len, proplen;

	FDT_RW_CHECK_HEADER(fdt);

	prop = fdt_get_property_w(fdt, nodeoffset, name, &len);
	if (!prop)
		return len;

	proplen = sizeof(*prop) + FDT_TAGALIGN(len);
	return fdt_splice_struct_(fdt, prop, proplen, 0);
}

int fdt_add_subnode_namelen(void *fdt, int parentoffset, const char *name,
			    int namelen)
{
	struct fdt_node_header *nh;
	int offset, nextoffset;
	int nodelen;
	int err;
	uint32_t tag;
	fdt32_t *endtag;

	FDT_RW_CHECK_HEADER(fdt);

	offset = fdt_subnode_offset_namelen(fdt, parentoffset, name, namelen);
	if (offset >= 0)
		return -FDT_ERR_EXISTS;
	else if (offset != -FDT_ERR_NOTFOUND)
		return offset;

	/* Try to place the new node after the parent's properties */
	fdt_next_tag(fdt, parentoffset, &nextoffset); /* skip the BEGIN_NODE */
	do {
		offset = nextoffset;
		tag    = fdt_next_tag(fdt, offset, &nextoffset);
	} while ((tag == FDT_PROP) || (tag == FDT_NOP));

	nh      = fdt_offset_ptr_w_(fdt, offset);
	nodelen = sizeof(*nh) + FDT_TAGALIGN(namelen + 1) + FDT_TAGSIZE;

	err = fdt_splice_struct_(fdt, nh, 0, nodelen);
	if (err)
		return err;

	nh->tag = cpu_to_fdt32(FDT_BEGIN_NODE);
	memset(nh->name, 0, FDT_TAGALIGN(namelen + 1));
	memcpy(nh->name, name, namelen);
	endtag  = (fdt32_t *)((char *)nh + nodelen - FDT_TAGSIZE);
	*endtag = cpu_to_fdt32(FDT_END_NODE);

	return offset;
}

int fdt_add_subnode(void *fdt, int parentoffset, const char *name)
{
	return fdt_add_subnode_namelen(fdt, parentoffset, name, strlen(name));
}

int fdt_del_node(void *fdt, int nodeoffset)
{
	int endoffset;

	FDT_RW_CHECK_HEADER(fdt);

	endoffset = fdt_node_end_offset_(fdt, nodeoffset);
	if (endoffset < 0)
		return endoffset;

	return fdt_splice_struct_(fdt, fdt_offset_ptr_w_(fdt, nodeoffset),
				  endoffset - nodeoffset, 0);
}

static void fdt_packblocks_(const char *old, char *new, int mem_rsv_size,
			    int struct_size)
{
	int mem_rsv_off, struct_off, strings_off;

	mem_rsv_off = FDT_ALIGN(sizeof(struct fdt_header), 8);
	struct_off  = mem_rsv_off + mem_rsv_size;
	strings_off = struct_off + struct_size;

	fdt_memmove(new + mem_rsv_off, old + fdt_off_mem_rsvmap(old), mem_rsv_size);
	fdt_set_off_mem_rsvmap(new, mem_rsv_off);

	fdt_memmove(new + struct_off, old + fdt_off_dt_struct(old), struct_size);
	fdt_set_off_dt_struct(new, struct_off);
	fdt_set_size_dt_struct(new, struct_size);

	fdt_memmove(new + strings_off, old + fdt_off_dt_strings(old),
		fdt_size_dt_strings(old));
	fdt_set_off_dt_strings(new, strings_off);
	fdt_set_size_dt_strings(new, fdt_size_dt_strings(old));
}

int fdt_open_into(const void *fdt, void *buf, int bufsize)
{
	int err;
	int mem_rsv_size, struct_size;
	int newsize;
	const char *fdtstart = fdt;
	const char *fdtend   = fdtstart + fdt_totalsize(fdt);
	char *tmp;

	FDT_CHECK_HEADER(fdt);

	mem_rsv_size =
		(fdt_num_mem_rsv(fdt) + 1) * sizeof(struct fdt_reserve_entry);

	if (fdt_version(fdt) >= 17) {
		struct_size = fdt_size_dt_struct(fdt);
	} else {
		struct_size = 0;
		while (fdt_next_tag(fdt, struct_size, &struct_size) != FDT_END)
			;
		if (struct_size < 0)
			return struct_size;
	}

	if (!fdt_blocks_misordered_(fdt, mem_rsv_size, struct_size)) {
		/* no further work necessary */
		err = fdt_move(fdt, buf, bufsize);
		if (err)
			return err;
		fdt_set_version(buf, 17);
		fdt_set_size_dt_struct(buf, struct_size);
		fdt_set_totalsize(buf, bufsize);
		return 0;
	}

	/* Need to reorder */
	newsize = FDT_ALIGN(sizeof(struct fdt_header), 8) + mem_rsv_size +
		  struct_size + fdt_size_dt_strings(fdt);

	if (bufsize < newsize)
		return -FDT_ERR_NOSPACE;

	/* First attempt to build converted tree at beginning of buffer */
	tmp = buf;
	/* But if that overlaps with the old tree... */
	if (((tmp + newsize) > fdtstart) && (tmp < fdtend)) {
		/* Try right after the old tree instead */
		tmp = (char *)(uintptr_t)fdtend;
		if ((tmp + newsize) > ((char *)buf + bufsize))
			return -FDT_ERR_NOSPACE;
	}

	fdt_packblocks_(fdt, tmp, mem_rsv_size, struct_size);
	fdt_memmove(buf, tmp, newsize);

	fdt_set_magic(buf, FDT_MAGIC);
	fdt_set_totalsize(buf, bufsize);
	fdt_set_version(buf, 17);
	fdt_set_last_comp_version(buf, 16);
	fdt_set_boot_cpuid_phys(buf, fdt_boot_cpuid_phys(fdt));

	return 0;
}

int fdt_pack(void *fdt)
{
	int mem_rsv_size;

	FDT_RW_CHECK_HEADER(fdt);

	mem_rsv_size =
		(fdt_num_mem_rsv(fdt) + 1) * sizeof(struct fdt_reserve_entry);
	fdt_packblocks_(fdt, fdt, mem_rsv_size, fdt_size_dt_struct(fdt));
	fdt_set_totalsize(fdt, fdt_data_size_(fdt));

	return 0;
}

/**
 * fdt_getprop_u32 - Find a node and return it's property or a default
 *
 * @fdt: ptr to device tree
 * @nodeoffset:  by fdt_path_offset() function
 * @prop: property name
 * @val: return value if the property is found
 * @note return value is the num of  the u32 element
 */
int fdt_getprop_u32(const void *fdt, int nodeoffset, const char *prop,
		    uint32_t *val)
{
	int len;
	const fdt32_t *data = NULL;

	data = fdt_getprop(fdt, nodeoffset, prop, &len);
	if ((data == NULL) || (len == 0) || (len % 4 != 0)) {
		return -FDT_ERR_INTERNAL;
	}
	if (val != NULL) {
		const uint32_t *p = data;
		int j;
		for (j = 0, p = data; j < len / 4; j++) {
			*val = fdt32_to_cpu(p[j]);
			val++;
		}
	}
	return len / 4;
}