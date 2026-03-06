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

target_mk=$(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk
NEED_BUILD_SIMPLEBOOT=$(shell grep -E '^CFG_SUNXI_SIMPLEBOOT\s*=\s*y$$' $(target_mk) > /dev/null && echo yes)
#include $(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk
include $(TOPDIR)/board/$(PLATFORM)/simpleboot.mk

exist = $(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/simpleboot/simpleboot_$(MAKECMDGOALS).mk ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
include $(TOPDIR)/board/$(PLATFORM)/simpleboot/simpleboot_$(MAKECMDGOALS).mk
endif

ifeq ($(NEED_BUILD_SIMPLEBOOT),yes)

MAIN_OBJ=simpleboot_main.o

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
SIMPLEBOOT_LDSCRIPT := $(CPUDIR)/simpleboot.lds

# We want the final binaries in this directory
SIMPLEBOOT_DIR := $(TOPDIR)/simpleboot/
obj := $(TOPDIR)/simpleboot/

LIBS-y += arch/$(ARCH)/cpu/$(CPU)/libarch.o
LIBS-y += simpleboot/main/libmain.o
LIBS-y += drivers/libdrivers.o
LIBS-y += common/libcommon.o
LIBS-y += board/$(PLATFORM)/libboard.o

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))

SULUTION_LIBS-y += solution/libsolution.o
SULUTION_LIBS := $(addprefix $(TOPDIR)/,$(sort $(SULUTION_LIBS-y)))

ifeq ($(CFG_SUNXI_SBOOT_LD_LIB), y)
ifeq ($(CPU), ads_rv32)
SECURE_LIBS-y := drivers/secure/libsecure_ads_rv32.a
else
SECURE_LIBS-y := drivers/secure/libsecure.a
endif
SECURE_LIBS := $(addprefix $(TOPDIR)/,$(sort $(SECURE_LIBS-y)))

COBJS-y += $(SECURE_LIBS)
endif

COBJS-y += $(TOPDIR)/simpleboot/main/$(MAIN_OBJ)

OBJS = $(sort $(COBJS-y))

ifndef CFG_SIMPLEBOOT_SIZE_LIMIT
	SIMPLEBOOT_SIZE = $(CFG_SYS_INIT_RAM_SIZE)
else
	SIMPLEBOOT_SIZE = $(CFG_SIMPLEBOOT_SIZE_LIMIT)
endif

BOOT0_BIN_PATH := $(subst _mmc,_sdcard,../nboot/boot0_$(MAKECMDGOALS)_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

S_FLAGS +=  -DBOOT0_BIN_PATH=\"$(BOOT0_BIN_PATH)\"
export S_FLAGS

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-DSIMPLEBOOTADDR=$(CFG_SIMPLEBOOT_RUN_ADDR)	 \
	-DSIMPLEBOOTSIZE=$(SIMPLEBOOT_SIZE)	 \
	-DCPUDIR=$(CPUDIR) \
	-DSOLUTIONDIR=$(SOLUTIONDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

define build_boot0_with_suffix
	$(Q)echo simplebootaddr is $(CFG_SIMPLEBOOT_RUN_ADDR)
	$(LD) -r -o lib$(PLATFORM)$1.o $(LIBS)
	$(LD) lib$(PLATFORM)$1.o $(OBJS) $(PLATFORM_LIBGCC) $(LDFLAGS) $(LDFLAGS_GC) -Tsimpleboot.lds -o simpleboot$1.elf -Map simpleboot$1.map
	$(OBJCOPY) $(OBJCFLAGS) -O binary  simpleboot$1.elf simpleboot$1.bin
	@$(TOPDIR)/mk/gen_check_sum $(SRCTREE)/simpleboot/simpleboot$1.bin simpleboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin > /dev/null
	@$(TOPDIR)/mk/encrypto_boot0 -f simpleboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin  -c $(LICHEE_IC) >/dev/null
	$(STRIP) -g $(TOPDIR)/simpleboot/lib$(PLATFORM)$1.o
	$(CP) $(TOPDIR)/simpleboot/lib$(PLATFORM)$1.o $(TOPDIR)/../spl-pub/board/$(CP_BOARD)/lib$(PLATFORM)$1.a
	@-cp -v $(TOPDIR)/simpleboot/lib$(PLATFORM)$1.o $(PLATFORM_OUTDIR)/lib/lib$(PLATFORM)$1.a
	@-cp -v $(TOPDIR)/simpleboot/simpleboot$1.map $(PLATFORM_OUTDIR)/
	@-cp -v $(TOPDIR)/simpleboot/simpleboot$1.elf $(PLATFORM_OUTDIR)/
endef

$(BOARD_BUILD_NBOOT): $(LIBS) $(SULUTION_LIBS) simpleboot.lds
	$(call build_boot0_with_suffix,_$@)
	@-cp simpleboot_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	$(call copy_bin_to_target_dir, boot0_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

spinor:	 $(LIBS) $(SULUTION_LIBS) simpleboot.lds
	$(call build_boot0_with_suffix,_spinor)
	@-cp simpleboot_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	$(call copy_bin_to_target_dir, boot0_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

mmc:	 $(LIBS) $(SULUTION_LIBS) simpleboot.lds
	$(call build_boot0_with_suffix,_sdcard)
	@-cp simpleboot_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	$(call copy_bin_to_target_dir, boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

ufs:	 $(LIBS) $(SULUTION_LIBS) simpleboot.lds
	$(call build_boot0_with_suffix,_ufs)
	@-cp simpleboot_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	$(call copy_bin_to_target_dir, boot0_ufs_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

nand:	 $(LIBS) $(SULUTION_LIBS) simpleboot.lds
	$(call build_boot0_with_suffix,_nand)
	@-cp simpleboot_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_$@_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	$(call copy_bin_to_target_dir, boot0_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin)

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

$(SULUTION_LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

simpleboot.lds: $(SIMPLEBOOT_LDSCRIPT) depend
	$(Q)$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$(SIMPLEBOOT_LDSCRIPT) >$@

$(SIMPLEBOOT_DIR)cur.log:
	@git show HEAD --pretty=format:"%H" | head -n 1 > cur.log

sinclude $(TOPDIR)/mk/target_for_conf.mk
depend: .depend build-confs

#########################################################################

# defines $(simpleboot_DIR).depend target
include $(TOPDIR)/mk/rules.mk
sinclude .depend

#########################################################################

else

$(BOARD_BUILD_NBOOT) spinor mmc ufs nand:
	@

endif # NEED_BUILD_SIMPLEBOOT
