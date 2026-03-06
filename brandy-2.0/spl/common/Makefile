# Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
#
# Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
# the people's Republic of China and other countries.
# All Allwinner Technology Co.,Ltd. trademarks are used with permission.
#
# DISCLAIMER
# THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
# IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
# IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
# ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
# ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
# COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
# YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
#
# THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
# PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
# WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
# THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
# OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
# IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

include $(TOPDIR)/mk/config.mk
LIB	=  libcommon.o

COBJS	+= string.o
COBJS	+= printf.o
COBJS   += boot_utils.o
COBJS   += eabi_compat.o
COBJS   += malloc.o
COBJS   += iobase_sunxi.o
ifdef CFG_SUNXI_MEMOP
COBJS   += memset_sunxi.o
COBJS   += memcpy_sunxi.o
endif
COBJS   += debug.o

ifdef_any_of = $(filter-out undefined,$(foreach v,$(1),$(origin $(v))))
ifndef_any_of = $(filter undefined,$(foreach v,$(1),$(origin $(v))))

ifdef CFG_SUNXI_GPT
COBJS   += part_efi.o
endif

ifdef CFG_SUNXI_ENV
COBJS   += env.o
endif

ifneq ($(call ifdef_any_of,CFG_SUNXI_ENV CFG_SUNXI_GUNZIP CFG_VERIFY_RTOS_DIGEST_CRC32),)
COBJS   += crc32.o
endif

ifdef CFG_SUNXI_GUNZIP
COBJS   += gunzip.o
COBJS   += zlib/zlib.o
endif

ifdef CFG_SUNXI_LZ4
COBJS   += lz4/lz4_wrapper.o
endif

ifdef CFG_SUNXI_LZMA
COBJS   += lzma/LzmaDec.o
COBJS   += lzma/LzmaTools.o
endif

ifdef CFG_SUNXI_ELF
COBJS   += elf_loader.o
endif

ifdef CFG_BOOT0_LOAD_FLASH
COBJS   += flash_loader.o
COBJS   += update_ispparm.o
endif

ifdef CFG_SUNXI_BOOT_PARAM
COBJS   += boot_param.o
endif

COBJS   += linux_loader.o
COBJS   += board_helper.o

LIBS-$(CFG_SUNXI_FDT) += common/libfdt/libfdt.o

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))


SRCS	:= $(COBJS:.o=.c) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))
OBJS	+= $(LIBS)
all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

depend:	$(obj).depend
.PHONY: depend
#########################################################################

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude .depend

#########################################################################
