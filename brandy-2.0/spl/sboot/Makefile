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
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/mk/out_file_manage.mk
include $(TOPDIR)/.module.common.mk
sinclude $(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk

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

CPUDIR = $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)
SBOOT_LDSCRIPT := $(CPUDIR)/sboot.lds

# We want the final binaries in this directory
SBOOT_DIR := $(TOPDIR)/sboot/

ifdef DRAM_TYPE
	DRAM_TYPE_NAME=_$(DRAM_TYPE)
endif

LIBS-y += drivers/libdrivers.o
LIBS-y += arch/$(ARCH)/cpu/$(CPU)/libarch.o
ifeq ($(CFG_SUNXI_SBOOT_LD_LIB), y)
ifeq ($(CPU), ads_rv32)
LIBS-y += sboot/main/libsboot_ads_rv32.a
else
LIBS-y += sboot/main/libsboot.a
endif
else
LIBS-y += sboot/main/libmain.o
endif
LIBS-y += common/libcommon.o
LIBS-y += board/$(PLATFORM)/libboard.o

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))

COBJS-y += $(TOPDIR)/sboot/main/sboot_main.o

ifeq ($(CPU), ads_rv32)
A-LIBS-$(CFG_SUNXI_SBOOT_LD_LIB)  := $(TOPDIR)/drivers/secure/libsecure_ads_rv32.a
A-LIBS-$(CFG_SUNXI_SBOOT_LD_LIB)  += $(TOPDIR)/sboot/main/libsboot_ads_rv32.a
else
A-LIBS-$(CFG_SUNXI_SBOOT_LD_LIB)  := $(TOPDIR)/drivers/secure/libsecure.a
A-LIBS-$(CFG_SUNXI_SBOOT_LD_LIB)  += $(TOPDIR)/sboot/main/libsboot.a
endif

A-OBJS := $(A-LIBS-y)
COBJS-y += $(A-OBJS)

OBJS = $(sort $(COBJS-y))

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-DSBOOTADDR=$(CFG_SBOOT_RUN_ADDR)	 \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

define build_sboot_with_suffix
	$(Q)echo bootaddr is $(CFG_SBOOT_RUN_ADDR)
	$(LD) -r -o lib$(PLATFORM)_sboot$1.o $(LIBS)
	$(LD) lib$(PLATFORM)_sboot$1.o $(OBJS) $(LIBSPINOR) $(PLATFORM_LIBGCC) $(LDFLAGS) $(LDFLAGS_GC) -Tsboot.lds -o sboot$1.elf -Map sboot$1.map
	$(OBJCOPY) $(OBJCFLAGS) -O binary  sboot$1.elf sboot$1.bin
	@$(TOPDIR)/mk/gen_check_sum $(SRCTREE)/sboot/sboot$1.bin sboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin > /dev/null
	@$(TOPDIR)/mk/encrypto_boot0 -f sboot$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin  -c $(LICHEE_IC) >/dev/null
	$(STRIP) -g $(TOPDIR)/sboot/lib$(PLATFORM)_sboot$1.o
	$(CP) $(TOPDIR)/sboot/lib$(PLATFORM)_sboot$1.o $(TOPDIR)/../spl-pub/board/$(CP_BOARD)/lib$(PLATFORM)_sboot$1.a
	@-cp -v $(TOPDIR)/sboot/lib$(PLATFORM)_sboot$1.o $(PLATFORM_OUTDIR)/lib/lib$(PLATFORM)_sboot$1.a
	@-cp -v $(TOPDIR)/sboot/sboot$1.map $(PLATFORM_OUTDIR)/
	@-cp -v $(TOPDIR)/sboot/sboot$1.elf $(PLATFORM_OUTDIR)/
endef

sboot:	$(LIBS) $(OBJS) sboot.lds
	$(call build_sboot_with_suffix,)
	$(call copy_bin_to_target_dir, sboot_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

sboot_nor:	$(LIBS) $(OBJS) sboot.lds
	$(call build_sboot_with_suffix,_nor)
	$(call copy_bin_to_target_dir, sboot_nor_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

sboot_mmc:	$(LIBS) $(OBJS) sboot.lds
	$(call build_sboot_with_suffix,_sdcard)
	$(call copy_bin_to_target_dir, sboot_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

sboot_nand:	$(LIBS) $(OBJS) sboot.lds
	$(call build_sboot_with_suffix,_nand)
	$(call copy_bin_to_target_dir, sboot_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

sboot_ufs:	$(LIBS) $(OBJS) sboot.lds
	$(call build_sboot_with_suffix,_ufs)
	$(call copy_bin_to_target_dir, sboot_ufs_$(PLATFORM)$(DRAM_TYPE_NAME).bin, $(COPY_FLAG))

sboot_$(DRAM_TYPE):sboot


$(LIBS): depend
	$(MAKE) -C $(dir $@)

sboot.lds: $(SBOOT_LDSCRIPT) depend
	$(Q)$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$(SBOOT_LDSCRIPT) >$@

$(NBOOT_DIR)cur.log:
	@git show HEAD --pretty=format:"%H" | head -n 1 > cur.log

depend: .depend build-confs
sinclude $(TOPDIR)/mk/target_for_conf.mk
#########################################################################

# defines $(NBOOT_DIR).depend target
include $(TOPDIR)/mk/rules.mk

sinclude .depend

#########################################################################
