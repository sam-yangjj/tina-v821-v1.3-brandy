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
#include <u-boot/zlib.h>
#include <part_efi.h>

#if ENV_DEBUG
#define env_dbg(fmt, arg...) printf(fmt, ##arg)
#else
#define env_dbg(fmt, arg...)
#endif

#define ENV_NAME "env"
#define ENV_NAME_2 "env-redund"

#define O_RDONLY    (00000000)
#define O_WRONLY    (00000001)
#define O_RDWR      (00000002)

static uint env_start_sector;
static uint env_start_sector_2;
static uint env_sectors;
static uint env_sectors_2;
static uint cur_envsize;

#ifdef CFG_SUNXI_HAVE_REDUNDENV
static unsigned char active_flag = 1;
/* obsolete_flag must be 0 to efficiently set it on NOR flash without erasing */
static unsigned char obsolete_flag;
#endif

static unsigned long usable_envsize;
#define ENV_SIZE      usable_envsize

static int dev_current;

static int s_is_env_opened;

struct env_image_single {
	uint32_t crc;		/* CRC32 over data bytes    */
	char data[];
};

struct env_image_redundant {
	uint32_t crc;		/* CRC32 over data bytes    */
	unsigned char flags;	/* active or obsolete */
	char data[];
};

enum flag_scheme {
	FLAG_NONE,
	FLAG_BOOLEAN,
	FLAG_INCREMENTAL,
};

struct environment {
	void *image;
	uint32_t *crc;
	unsigned char *flags;
	char *data;
	enum flag_scheme flag_scheme;
};

static struct environment environment = {
	.flag_scheme = FLAG_NONE,
};

static int flash_io(int mode);

/*
 * s1 is either a simple 'name', or a 'name=value' pair.
 * s2 is a 'name=value' pair.
 * If the names match, return the value of s2, else NULL.
 */
static char *envmatch(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return s2;
	if (*s1 == '\0' && *(s2 - 1) == '=')
		return s2;
	return NULL;
}

/**
 * Search the environment for a variable.
 * Return the value, if found, or NULL, if not found.
 */
char *fw_getenv(char *name)
{
	char *env, *nxt;

	for (nxt = environment.data; !(*nxt); ++nxt) {
		if (nxt >= &environment.data[ENV_SIZE]) {
			env_dbg("## Error: environment not terminated\n");
			return NULL;
		}
	}

	for (env = nxt; *env; env = nxt + 1) {
		char *val;

		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				env_dbg("## Error: environment not terminated\n");
				return NULL;
			}
		}
		val = envmatch(name, env);
		if (!val)
			continue;
		return val;
	}
	return NULL;
}

#if ENV_DEBUG
void fw_env_dump(void)
{
	char *env, *nxt;

	for (nxt = environment.data; !(*nxt); ++nxt) {
		if (nxt >= &environment.data[ENV_SIZE])
			env_dbg("## Error: environment not terminated\n");
	}

	for (env = nxt; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE])
				env_dbg("## Error: environment not terminated\n");
		}
		env_dbg("env: %s\n", env);
	}
}
#endif

#if 0
/*
 * Set/Clear a single variable in the environment.
 * This is called in sequence to update the environment
 * in RAM without updating the copy in flash after each set
 */
int fw_env_write(char *name, char *value)
{
	int len;
	char *env, *nxt;
	char *oldval = NULL;
	int deleting, creating, overwriting;

	/*
	 * search if variable with this name already exists
	 */
	for (nxt = env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				env_dbg("## Error: "
					"environment not terminated\n");
				/* errno = EINVAL; */
				return -1;
			}
		}
		oldval = envmatch(name, env);
		if (oldval)
			break;
	}

	deleting = (oldval && !(value && strlen(value)));
	creating = (!oldval && (value && strlen(value)));
	overwriting = (oldval && (value && strlen(value)));

	if (deleting) {
		env_dbg("env: delting\n");
	} else if (overwriting) {
		env_dbg("env: overwriting\n");
	} else if (creating) {
		env_dbg("env: creating\n");
	} else {
		env_dbg("env: nothing\n");
		return 0;
	}

	if (deleting || overwriting) {
		if (*++nxt == '\0') {
			*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	/* Delete only ? */
	if (!value || !strlen(value))
		return 0;

	/*
	 * Append new definition at the end
	 */
	for (env = environment.data; *env || *(env + 1); ++env)
		;
	if (env > environment.data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > cur_envsize - (env-environment)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	len += strlen(value) + 1;

	if (len > (&environment.data[ENV_SIZE] - env)) {
		env_dbg("Error: environment overflow, \"%s\" deleted\n", name);
		return -1;
	}

	while ((*env = *name++) != '\0')
		env++;
	*env = '=';
	while ((*++env = *value++) != '\0')
		;

	/* end is marked with double '\0' */
	*++env = '\0';

	return 0;
}

int fw_env_flush(void)
{
	/*
	 * Update CRC
	 */
#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	*environment.crc = crc32(0, (uint8_t *) environment.data, ENV_SIZE);
#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif

	/* write environment back to flash */
	if (flash_io(O_RDWR)) {
		env_dbg("Error: can't write fw_env to flash\n");
		return -1;
	}

	return 0;
}
#endif

static int flash_env_init(void)
{
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	get_part_info_by_name(ENV_NAME, &env_start_sector, &env_sectors);
	get_part_info_by_name(ENV_NAME_2, &env_start_sector_2, &env_sectors_2);
	env_dbg("env_start_sector_2=%d %d\n", env_start_sector, env_start_sector_2);
	env_dbg("env_sectors_2=%d %d\n", env_sectors, env_sectors_2);
#else
	get_part_info_by_name(ENV_NAME, &env_start_sector, &env_sectors);
	env_dbg("env_start_sector=%d\n", env_start_sector);
	env_dbg("env_sectors=%d\n", env_sectors);
#endif
	if (env_start_sector == 0 && env_start_sector_2 == 0) {
		env_dbg("%s and %s part info all can't find\n",
		 ENV_NAME, ENV_NAME_2);
		return -1;
	}
	if (env_sectors != 0 || env_sectors == env_sectors_2) {
#ifdef CFG_SUNXI_ENV_SIZE
		cur_envsize = CFG_SUNXI_ENV_SIZE;
#else
		/* for env 4k is enough, but partition size may not only 4k (maybe align to 32k or 64k) */
		cur_envsize = 4 * 1024;
#endif
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	} else {
		env_dbg("%s and %s part size diff\n", ENV_NAME, ENV_NAME_2);
		return -1;
#endif
	}
	return 0;
}

static int flash_env_read(void *buf, size_t size)
{
	int start_sector;
	if (dev_current == 0) {
		start_sector = env_start_sector;
	} else if (dev_current == 1) {
		start_sector = env_start_sector_2;
	} else {
		return -1;
	}

	if (spl_flash_init()) {
		env_dbg("flash init fail\n");
		return -1;
	}
	if (spl_flash_read(start_sector, size/512, buf)) {
		env_dbg("read flash fail\n");
		return -1;
	}
//	env_dbg("buf%x,%x,%x,%x\n",*(char*)buf,*(char*)(buf+1),*(char*)(buf+2),*(char*)(buf+3));
	return 0;
}

/*
 * Set obsolete flag at offset - NOR flash only
 */
 #if 0
static int flash_flag_obsolete(off_t offset)
{
	return -1;
}

static int flash_env_write(void *buf, size_t size)
{
	int next_env_sector;
	int current_env_sector;
	if (dev_current == 0) {
		current_env_sector = env_start_sector;
		next_env_sector = env_start_sector_2;
	} else if (dev_current == 1) {
		current_env_sector = env_start_sector_2;
		next_env_sector = env_start_sector;
	} else {
		return -1;
	}

	if (spl_flash_init()) {
		env_dbg("flash init fail\n");
		return -1;
	}

	switch (environment.flag_scheme) {
	case FLAG_NONE:
		break;
	case FLAG_INCREMENTAL:
		(*environment.flags)++;
		break;
	case FLAG_BOOLEAN:
		*environment.flags = active_flag;
		break;
	default:
		env_dbg("Unimplemented flash scheme %d\n",
			environment.flag_scheme);
		return -1;
	}

	if (spl_flash_erase(next_env_sector, size/512)) {
		env_dbg("erase flash fail\n");
		return -1;
	}

	if (spl_flash_write(next_env_sector, size/512, buf)) {
		env_dbg("write flash fail\n");
		return -1;
	}

	if (environment.flag_scheme == FLAG_BOOLEAN) {
		/* Have to set obsolete flag */
		int offset = current_env_sector * 512 +
			offsetof(struct env_image_redundant, flags);

		flash_flag_obsolete(offset);
	}

	return 0;
}
#endif
static int flash_io(int mode)
{
	void *buf;
	size_t size;
	int rc;

	if (mode == O_RDONLY) {
		buf = environment.image;
		size = cur_envsize;
		rc = flash_env_read(buf, size);
	} else {
		env_dbg("%s %d %s boot0 not support write env\n", __FILE__, __LINE__, __func__);
		return -1;
	}

	return rc;
}

/*
 * Prevent confusion if running from erased flash memory
 */
int fw_env_open(void)
{
	int crc0, crc0_ok;
	void *addr0 = NULL;

#ifdef CFG_SUNXI_HAVE_REDUNDENV
	int crc1, crc1_ok;
	unsigned char flag0;
	unsigned char flag1;
	void *addr1 = NULL;
#endif
	int ret;

	if (s_is_env_opened) {
		env_dbg("env has been opened: %d\n", s_is_env_opened);
		return 0;
	}

	if (flash_env_init()) {
		env_dbg("env init fail\n");
		return -1;
	}

	env_dbg("cur_envsize 0x%x\n", cur_envsize);

	addr0 = malloc(cur_envsize  + 1024); // more memory for fixed emmc boot memory error
	if (addr0 == NULL) {
		env_dbg("Not enough memory for environment (%ld bytes)\n",
				cur_envsize);
		ret = -1;
		goto open_cleanup;
	}

	memset(addr0, 0x00, cur_envsize);
	/* read environment from FLASH to local buffer */
	environment.image = addr0;
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	struct env_image_redundant *redundant;
	redundant = addr0;
	environment.crc = &redundant->crc;
	environment.flags = &redundant->flags;
	environment.data = redundant->data;
#else
	struct env_image_single *single;
	single = addr0;
	environment.crc = &single->crc;
	environment.flags = NULL;
	environment.data = single->data;
#endif

	usable_envsize = cur_envsize - sizeof(uint32_t);
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	usable_envsize -= sizeof(char);
#endif

	dev_current = 0;
	if (flash_io(O_RDONLY)) {
		ret = -1;
		goto open_cleanup;
	}

#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	crc0 = crc32(0, (uint8_t *)environment.data, ENV_SIZE);
#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif

	crc0_ok = (crc0 == *environment.crc);
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	flag0 = *environment.flags;

	dev_current = 1;
	env_dbg("cur_envsize 0x%x\n", cur_envsize);
	addr1 = malloc(cur_envsize + 1024);// more memory for fixed emmc boot memory error
	//env_dbg("env addr1:0x%x\n", addr1);
	if (addr1 == NULL) {
		env_dbg("Not enough memory for environment (%ld bytes)\n",
				cur_envsize);
		ret = -1;
		goto open_cleanup;
	}
	memset(addr1, 0x00, cur_envsize);
	redundant = addr1;
	/*
	 * have to set environment.image for flash_read(), careful -
	 * other pointers in environment still point inside addr0
	 */
	environment.image = addr1;
	if (flash_io(O_RDONLY)) {
		ret = -1;
		goto open_cleanup;
	}

	/* Check flag scheme compatibility */
	/* Hardcode */
	environment.flag_scheme = FLAG_INCREMENTAL;
#ifdef CFG_USE_DCACHE
	dcache_enable();
#endif
	crc1 = crc32(0, (uint8_t *)redundant->data, ENV_SIZE);
#ifdef CFG_USE_DCACHE
	dcache_disable();
#endif
	crc1_ok = (crc1 == redundant->crc);
	flag1 = redundant->flags;

	env_dbg("crc0_ok:%d, crc1_ok:%d flag0:%d flag1:%d\n", crc0_ok, crc1_ok, flag0, flag1);
	if (crc0_ok && !crc1_ok) {
		dev_current = 0;
	} else if (!crc0_ok && crc1_ok) {
		dev_current = 1;
	} else if (!crc0_ok && !crc1_ok) {
		env_dbg("Warning: Bad CRC\n");
		return -1;
	} else {
		switch (environment.flag_scheme) {
		case FLAG_BOOLEAN:
			if (flag0 == active_flag &&
					flag1 == obsolete_flag) {
				dev_current = 0;
			} else if (flag0 == obsolete_flag &&
					flag1 == active_flag) {
				dev_current = 1;
			} else if (flag0 == flag1) {
				dev_current = 0;
			} else if (flag0 == 0xFF) {
				dev_current = 0;
			} else if (flag1 == 0xFF) {
				dev_current = 1;
			} else {
				dev_current = 0;
			}
			break;
		case FLAG_INCREMENTAL:
			if (flag0 == 255 && flag1 == 0)
				dev_current = 1;
			else if ((flag1 == 255 && flag0 == 0) ||
					flag0 >= flag1)
				dev_current = 0;
			else	/* flag1 > flag0 */
				dev_current = 1;
			break;
		default:
			env_dbg("Unknown flag scheme %d\n",
					environment.flag_scheme);
			return -1;
		}
	}
	/*
	 * If we are reading, we don't need the flag and the CRC any
	 * more, if we are writing, we will re-calculate CRC and update
	 * flags before writing out
	 */
	if (dev_current) {
		environment.image = addr1;
		environment.crc = &redundant->crc;
		environment.flags = &redundant->flags;
		environment.data = redundant->data;
		free(addr0);
	} else {
		environment.image = addr0;
		/* Other pointers are already set */
		free(addr1);
	}
	env_dbg("dev_current:%d\n", dev_current);
#else
	if (!crc0_ok) {
		env_dbg("Warning: Bad CRC\n");
		return -1;
	}
#endif
	s_is_env_opened = 1;
	return 0;

open_cleanup:
	if (addr0)
		free(addr0);
#ifdef CFG_SUNXI_HAVE_REDUNDENV
	if (addr1)
		free(addr1);
#endif
	return ret;
}

/*
 * Simply free allocated buffer with environment
 */
int fw_env_close(void)
{
	if (environment.image)
		free(environment.image);

	environment.image = NULL;
	s_is_env_opened = 0;

	return 0;
}


