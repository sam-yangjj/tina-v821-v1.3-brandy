/* Glue between u-boot and upstream zlib */
#ifndef __GLUE_ZLIB_H__
#define __GLUE_ZLIB_H__

#include <common.h>
#include <linux/compiler.h>
#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/generic.h>
//#include <watchdog.h>
#include "u-boot/zlib.h"

#define get_unaligned  __get_unaligned_le
#define put_unaligned  __put_unaligned_le

/* avoid conflicts */
#undef OFF
#undef ASMINF
#undef POSTINC
#undef NO_GZIP
#define GUNZIP
#undef STDC
#undef NO_ERRNO_H

#endif
