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

LIB	=  libdram.o
COBJS-y	+= sunxi_dram.o

ifeq ($(notdir $(shell find $(PLATFORM) -name lib-dram)),lib-dram)
LIBS-y += drivers/dram/$(PLATFORM)/libdram$(DRAM_TYPE_NAME)
else ifeq ($(CFG_FPGA_PLATFORM)$(notdir $(shell find $(PLATFORM) -name libdram-fpga)),ylibdram-fpga)
LIBS-y += drivers/dram/$(PLATFORM)/libdram-fpga$(DRAM_TYPE_NAME)
else
LIBS-y += drivers/dram/$(PLATFORM)/libdram$(DRAM_TYPE_NAME)
endif

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
OBJS	+= $(LIBS)
all:	$(obj).depend $(LIB)
ifeq (x$(dramlib_exist), xyes)
	if [ -f $(TOPDIR)/drivers/dram/$(PLATFORM)/libdram$(DRAM_TYPE_NAME) ]; then \
	rm $(TOPDIR)/drivers/dram/$(PLATFORM)/libdram$(DRAM_TYPE_NAME); \
	fi;
endif

$(LIB):	$(OBJS) $(LIBS)
	$(call cmd_link_o_target, $(OBJS))

# add LIBS_TO_BUILD to deleted possiable repeated "drivers/dram/$(PLATFORM)"
# from drivers/dram/$(PLATFORM)/libchipid
# and drivers/dram/$(PLATFORM)/libdram
LIBS_TO_BUILD := $(sort $(dir $(subst $(OBJTREE),,$(LIBS))))

$(LIBS): depend $(LIBS_TO_BUILD)

.PHONY:$(LIBS_TO_BUILD)
$(LIBS_TO_BUILD):
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))
#########################################################################

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk
