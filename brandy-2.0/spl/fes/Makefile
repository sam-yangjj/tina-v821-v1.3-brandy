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

SKIP_AUTO_CONF:=yes
include $(TOPDIR)/.module.common.mk
include $(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/mk/out_file_manage.mk

exist = $(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/fes/fes_$(LICHEE_BOOT0_BIN_NAME).mk ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
include $(TOPDIR)/board/$(PLATFORM)/fes/fes_$(LICHEE_BOOT0_BIN_NAME).mk
endif

COPY_FLAG:=
REAL_LICHEE_SPL_CUSTOMS_MK := $(subst ", ,$(LICHEE_SPL_BOARD_MK))
custom_all_mk := $(filter all%, $(REAL_LICHEE_SPL_CUSTOMS_MK))
ifneq ($(custom_all_mk),)
$(foreach mk, $(custom_all_mk), \
	$(info load board mk file $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk) \
	$(eval include $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk);)
COPY_FLAG:=custom
endif

REAL_LICHEE_SPL_CUSTOMS_MK := $(subst ", ,$(LICHEE_SPL_BOARD_MK))
custom_all_mk := $(filter fes%, $(REAL_LICHEE_SPL_CUSTOMS_MK))
ifneq ($(custom_all_mk),)
$(foreach mk, $(custom_all_mk), \
	$(info load board mk file $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk) \
	$(eval include $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk);)
COPY_FLAG:=custom
endif

#This for Tina system, to copy bin to tina SDK
ifeq ($(TARGET_BUILD_VARIANT),tina)
tina_path=$(TOPDIR)/../../../target/allwinner/$(TARGET_PLATFORM)-common/bin
exist = $(shell if [ -d $(tina_path) ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
TARGETDIR=$(TOPDIR)/../../../target/allwinner/$(TARGET_PLATFORM)-common/bin
endif

tina_path=$(TOPDIR)/../../../device/config/chips/$(TARGET_PLATFORM)/bin
exist = $(shell if [ -d $(tina_path) ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
TARGETDIR=$(TOPDIR)/../../../device/config/chips/$(TARGET_PLATFORM)/bin
endif

else
#TOOLS_DIR  := $(TOPDIR)/../../../device/config
#TARGETDIR=$(TOPDIR)/../../../device/config/chips/$(PLATFORM)/bin
ifneq ($(LICHEE_BUSSINESS), )
       TARGETDIR=$(LICHEE_CHIP_CONFIG_DIR)/$(LICHEE_BUSSINESS)/bin
else
       TARGETDIR=$(LICHEE_CHIP_CONFIG_DIR)/bin
endif
endif
CPU ?= armv7
CPUDIR := $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)
FES_LDSCRIPT := $(CPUDIR)/fes1.lds

# We want the final binaries in this directory
FES1_DIR := $(TOPDIR)/fes/
obj := $(TOPDIR)/fes/

ifdef DRAM_TYPE
	DRAM_TYPE_NAME=_$(DRAM_TYPE)
endif


LIBS-y += arch/$(ARCH)/cpu/$(CPU)/libarch.o
LIBS-y += fes/main/libmain.o
LIBS-y += drivers/libdrivers.o
LIBS-y += common/libcommon.o
LIBS-y += board/$(PLATFORM)/libboard.o

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))

ifeq ($(CFG_SUNXI_SBOOT_LD_LIB), y)
ifeq ($(CPU), ads_rv32)
SECURE_LIBS-y := drivers/secure/libsecure_ads_rv32.a
else
SECURE_LIBS-y := drivers/secure/libsecure.a
endif
SECURE_LIBS := $(addprefix $(TOPDIR)/,$(sort $(SECURE_LIBS-y)))

COBJS-y += $(SECURE_LIBS)
endif

COBJS-y += $(TOPDIR)/fes/main/fes1_main.o

OBJS = $(sort $(COBJS-y))




# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-DFES1ADDR=$(CFG_FES1_RUN_ADDR)	 \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')


fes: $(LIBS) fes1.lds
	$(LD) -r -o lib$(PLATFORM)_fes.o $(LIBS)
	$(LD) lib$(PLATFORM)_fes.o $(OBJS) $(PLATFORM_LIBGCC) $(LDFLAGS) $(LDFLAGS_GC) -Tfes1.lds -o fes1.elf -Map fes1.map
	$(OBJCOPY) $(OBJCFLAGS) -O binary fes1.elf fes1.bin
	$(Q)$(TOPDIR)/mk/gen_check_sum fes1.bin fes1_$(PLATFORM)$(DRAM_TYPE_NAME).bin > /dev/null
	$(STRIP) -g $(TOPDIR)/fes/lib$(PLATFORM)_fes.o
	$(CP) $(TOPDIR)/fes/lib$(PLATFORM)_fes.o $(TOPDIR)/../spl-pub/board/$(CP_BOARD)/lib$(PLATFORM)_fes.a
	@-cp -v $(TOPDIR)/fes/lib$(PLATFORM)_fes.o $(PLATFORM_OUTDIR)/lib/lib$(PLATFORM)_fes.a
	@-cp -v $(TOPDIR)/fes/fes1.map $(PLATFORM_OUTDIR)/fes1.map
	@-cp -v $(TOPDIR)/fes/fes1.elf $(PLATFORM_OUTDIR)/fes1.elf
	$(call copy_bin_to_target_dir, fes1_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))


$(LIBS): depend
	$(MAKE) -C $(dir $@)


fes1.lds: $(FES_LDSCRIPT) depend
	$(Q)$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$(FES_LDSCRIPT) >$@


sinclude $(TOPDIR)/mk/target_for_conf.mk
depend: .depend build-confs
#########################################################################

# defines $(NBOOT_DIR).depend target
include $(TOPDIR)/mk/rules.mk

sinclude .depend

#########################################################################
