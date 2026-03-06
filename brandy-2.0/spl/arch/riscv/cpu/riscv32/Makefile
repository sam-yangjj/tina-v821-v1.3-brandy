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
#
#

include $(TOPDIR)/mk/config.mk

LIB	:= libarch.o
CPU	?= riscv32

START-$(CFG_SUNXI_FES) = fes1_entry.o

START-$(CFG_SUNXI_SBOOT) = sboot_entry.o

ifeq ($(CFG_SUNXI_FES)$(CFG_SUNXI_SBOOT),)
START-y = boot0_entry.o
endif

HEAD := e907_boot.o

COBJS-y	+= arch_timer.o
COBJS-y   += mmu.o
#SOBJS-y	+= cache-v7.o
COBJS-y   += jmp.o

ifndef CFG_SUNXI_MEMOP
SOBJS-y   += memset.o
SOBJS-y   += memcpy.o
endif

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))


SRCS	:= $(COBJS-y:.o=.c) $(SOBJS-y:.o=.S) $(START-y:.o=.S) $(HEAD:.o=.c)
HEAD    := $(addprefix $(obj),$(HEAD))
START	:= $(addprefix $(obj),$(START-y))
OBJS	:= $(addprefix $(obj),$(COBJS-y) $(SOBJS-y))
OBJS	+= $(LIBS)
all:   $(obj).depend $(HEAD) $(START) $(LIB)


$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))


$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))
#########################################################################
depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude $(obj).depend

#########################################################################
