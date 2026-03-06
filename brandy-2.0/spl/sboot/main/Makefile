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

LIB	:= $(obj)libmain.o

HEAD    := sboot_head.o
MAIN	:= sboot_main.o

STATIC_LIB := libsboot.a

CFLAGS += -I../../drivers/ufs/uinclude/
CFLAGS += -I../../drivers/ufs/uinclude/linux/
CFLAGS += -I../../drivers/ufs/uinclude/dm/
CFLAGS += -I../../drivers/ufs/uinclude/
CFLAGS += -I../../drivers/ufs/
CFLAGS += -I../../drivers/ufs/trace/

ifeq ($(CFG_SUNXI_SBOOT_LIB), y)
COBJS += sboot_toc1.o
COBJS += sboot_flash.o
ifeq ($(CFG_AW_CERT), y)
COBJS += aw_ceritf.o
endif

COBJS += sboot_ceritf.o
COBJS += sunxi_ceritf_interface.o
endif

SRCS	:= $(MAIN:.o=.c) $(COBJS:.o=.c) $(HEAD:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))
HEAD    := $(addprefix $(obj),$(HEAD))
MAIN	:= $(addprefix $(obj),$(MAIN))

ifeq ($(CFG_SUNXI_SBOOT_LD_LIB),y)
all:	$(obj).depend $(HEAD) $(MAIN) $(STATIC_LIB)

$(STATIC_LIB):
	@echo "Not building static library: $@"

ifeq ($(CFG_SUNXI_SBOOT_LIB), y)
$(STATIC_LIB): $(LIB)
	@echo "Building static library: $@"
	$(AR) rcs $@ $^
endif
else
all:	$(obj).depend $(HEAD) $(MAIN) $(LIB)
endif

$(LIB):	$(OBJS) $(HEAD)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude $(obj).depend

#########################################################################
