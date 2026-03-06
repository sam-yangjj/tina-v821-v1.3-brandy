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

# (C) Copyright 2024
#
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

SKIP_AUTO_CONF:=yes
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/.module.common.mk
include $(TOPDIR)/mk/out_file_manage.mk

include $(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk

PMBOOT_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/pmboot.mk ]; then echo yes; else echo no; fi;)
COPY_FLAG:=
REAL_LICHEE_SPL_CUSTOMS_MK := $(subst ", ,$(LICHEE_SPL_BOARD_MK))
custom_all_mk := $(filter all%, $(REAL_LICHEE_SPL_CUSTOMS_MK))
ifneq ($(custom_all_mk),)
$(foreach mk, $(custom_all_mk), \
	$(info load board mk file $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk) \
	$(eval include $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk);)
COPY_FLAG:=custom
endif

custom_storage_mk := $(filter $(MAKECMDGOALS)%, $(REAL_LICHEE_SPL_CUSTOMS_MK))
ifneq ($(custom_storage_mk),)
$(foreach mk, $(custom_storage_mk), \
	$(info load board mk file $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk) \
	$(eval include $(TOPDIR)/board/$(PLATFORM)/$(word 2, $(subst -, ,$(mk))).mk);)
COPY_FLAG:=custom
endif

ifeq (x$(PMBOOT_EXIST),xyes)
include $(TOPDIR)/board/$(PLATFORM)/pmboot.mk

MAIN_OBJ=pmboot_main.o

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

CPUDIR = $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)
SOLUTIONDIR = $(TOPDIR)/solution
PMBOOT_LDSCRIPT := $(CPUDIR)/pmboot.lds

# We want the final binaries in this directory
PMBOOT_DIR := $(TOPDIR)/pmboot/
obj := $(TOPDIR)/pmboot/

LIBS-y += arch/$(ARCH)/cpu/$(CPU)/libarch.o
LIBS-y += pmboot/main/libmain.o
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

COBJS-y += $(TOPDIR)/pmboot/main/$(MAIN_OBJ)

SULUTION_LIBS-y += solution/libsolution.o
SULUTION_LIBS := $(addprefix $(TOPDIR)/,$(sort $(SULUTION_LIBS-y)))

OBJS = $(sort $(COBJS-y))

ifndef CFG_PMBOOT_SIZE_LIMIT
	PMBOOT_SIZE = $(CFG_SYS_INIT_RAM_SIZE)
else
	PMBOOT_SIZE = $(CFG_PMBOOT_SIZE_LIMIT)
endif

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-DPMBOOTADDR=$(CFG_PMBOOT_RUN_ADDR)	 \
	-DPMBOOTSIZE=$(CFG_PMBOOT_SIZE)	 \
	-DCPUDIR=$(CPUDIR) \
	-DSOLUTIONDIR=$(SOLUTIONDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

define build_pmboot_with_suffix
	$(Q)echo pmbootaddr is $(CFG_PMBOOT_RUN_ADDR)
	$(LD) -r -o lib$(PLATFORM)$1.o $(LIBS)
	$(LD) lib$(PLATFORM)$1.o $(OBJS) $(PLATFORM_LIBGCC) $(LDFLAGS) $(LDFLAGS_GC) -Tpmboot.lds -o pmboot$1.elf -Map pmboot$1.map
	$(OBJCOPY) $(OBJCFLAGS) -O binary  pmboot$1.elf pmboot$1.bin
	@$(TOPDIR)/mk/gen_check_sum $(SRCTREE)/pmboot/pmboot$1.bin pmboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin > /dev/null
	@$(TOPDIR)/mk/encrypto_boot0 -f pmboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin  -c $(LICHEE_IC) >/dev/null
	$(STRIP) -g $(TOPDIR)/pmboot/lib$(PLATFORM)$1.o
	$(CP) $(TOPDIR)/pmboot/lib$(PLATFORM)$1.o $(TOPDIR)/../spl-pub/board/$(CP_BOARD)/lib$(PLATFORM)$1.a
	@-cp -v $(TOPDIR)/pmboot/lib$(PLATFORM)$1.o $(PLATFORM_OUTDIR)/lib/lib$(PLATFORM)$1.a
	@-cp -v $(TOPDIR)/pmboot/pmboot$1.map $(PLATFORM_OUTDIR)/
	@-cp -v $(TOPDIR)/pmboot/pmboot$1.elf $(PLATFORM_OUTDIR)/
endef

$(BOARD_BUILD_NBOOT): $(LIBS) $(SULUTION_LIBS) pmboot.lds
	$(call build_pmboot_with_suffix,_$@)
	$(call copy_bin_to_target_dir, pmboot_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

spinor:	 $(LIBS) pmboot.lds
	$(call build_pmboot_with_suffix,_spinor)
	$(call copy_bin_to_target_dir, pmboot_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

mmc:	 $(LIBS) pmboot.lds
	$(call build_pmboot_with_suffix,_sdcard)
	$(call copy_bin_to_target_dir, pmboot_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

nand:	 $(LIBS) pmboot.lds
	$(call build_pmboot_with_suffix,_nand)
	$(call copy_bin_to_target_dir, pmboot_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

$(SULUTION_LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

pmboot.lds: $(PMBOOT_LDSCRIPT) depend
	$(Q)$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$(PMBOOT_LDSCRIPT) >$@

$(PMBOOT_DIR)cur.log:
	@git show HEAD --pretty=format:"%H" | head -n 1 > cur.log

sinclude $(TOPDIR)/mk/target_for_conf.mk
depend: .depend build-confs

#########################################################################

# defines $(PMBOOT_DIR).depend target
include $(TOPDIR)/mk/rules.mk
sinclude .depend

#########################################################################

endif
