/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Copyright (C) 2010 Shawn Lin (nobuhiro@andestech.com)
 * Copyright (C) 2011 Macpaul Lin (macpaul@andestech.com)
 * Copyright (C) 2017 Rick Chen (rick@andestech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef __ASM_RISCV_STRING_H
#define __ASM_RISCV_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#undef __HAVE_ARCH_STRRCHR
#undef __HAVE_ARCH_STRCHR

#if CONFIG_IS_ENABLED(USE_ARCH_MEMCPY)
#define __HAVE_ARCH_MEMCPY
#endif
extern void * memcpy(void *, const void *, __kernel_size_t);

#if CONFIG_IS_ENABLED(USE_ARCH_MEMMOVE)
#define __HAVE_ARCH_MEMMOVE
#endif
extern void * memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMCHR
#undef __HAVE_ARCH_MEMZERO

#if CONFIG_IS_ENABLED(USE_ARCH_MEMSET)
#define __HAVE_ARCH_MEMSET
#endif
extern void * memset(void *, int, __kernel_size_t);

#ifdef CONFIG_MARCO_MEMSET
#define memset(_p, _v, _n)	\
	(typeof(_p) (p) = (_p); \
	 typeof(_v) (v) = (_v); \
	 typeof(_n) (n) = (_n); \
	 {								\
		if ((n) != 0) {						\
			if (__builtin_constant_p((v)) && (v) == 0)	\
				__memzero((p), (n));			\
			else						\
				memset((p), (v), (n));			\
		}							\
		(p);							\
	})

#define memzero(_p, _n) \
	(typeof(_p) (p) = (_p); \
	 typeof(_n) (n) = (_n); \
	 { if ((n) != 0) __memzero((p), (n)); (p); })
#endif

#endif /* __ASM_RISCV_STRING_H */
