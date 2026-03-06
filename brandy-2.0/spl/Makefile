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
# Makefile for sunxi bootloader
# wangwei@allwinnertech.com
#

TOPDIR=$(CURDIR)
SRCTREE=$(TOPDIR)

#Q: quit for compile
Q = @
ifeq ("$(origin V)", "command line")
VERBOSE=$(V)
endif
ifndef VERBOSE
VERBOSE=0
endif

ifeq ($(VERBOSE),1)
Q=
CMD_ECHO_SILENT := true
else
CMD_ECHO_SILENT := echo
endif

#P: platform: sun50iw3p1/sun8iw15p1 etc.
PLATFORM=NULL
ifeq ("$(origin p)", "command line")
PLATFORM=$(p)
endif


#M: compile module: nand mmc nor etc.
MODULE=NULL
ifeq ("$(origin m)", "command line")
MODULE=$(m)
endif

#ddr: compile module: dd3/lpddr3/ddr4/lpddr4
MODULE=NULL
ifeq ("$(origin ddr)", "command line")
DRAM_TYPE=$(ddr)
DRAM_TYPE_NAME=_$(DRAM_TYPE)
endif

#'@':no echo, '-':error ignore, '\#':comment execution command
CP=@-\#
MKDIR=@-\#
ifeq ("$(origin C)", "command line")
spl-pub_exist_check=$(shell if [ -d ../spl-pub ]; then echo yes; else echo no; fi;)
ifeq (x$(spl-pub_exist_check), xyes)
CP=@-cp -v
MKDIR=@-mkdir
endif
endif

ifeq ("$(origin b)", "command line")
BOARD=$(b)
endif

#get arch
buildconfig = ../../../.buildconfig
ifeq ($(buildconfig), $(wildcard $(buildconfig)))
	LICHEE_BUSSINESS=$(shell cat $(buildconfig) | grep -w "LICHEE_BUSSINESS" | awk -F= '{printf $$2}')
	LICHEE_CHIP_CONFIG_DIR=$(shell cat $(buildconfig) | grep -w "LICHEE_CHIP_CONFIG_DIR" | awk -F= '{printf $$2}')
	LICHEE_BOOT0_BIN_NAME=$(shell cat $(buildconfig) | grep -w "LICHEE_BOOT0_BIN_NAME" | awk -F= '{printf $$2}')
	LICHEE_IC=$(shell cat $(buildconfig) | grep -w "LICHEE_IC" | awk -F= '{printf $$2}')
	LICHEE_PLAT_OUT=$(shell cat $(buildconfig) | grep -w "LICHEE_PLAT_OUT" | awk -F= '{printf $$2}')
	LICHEE_BOARD=$(shell cat $(buildconfig) | grep -w "LICHEE_BOARD" | awk -F= '{printf $$2}')
	LICHEE_SPL_BOARD_MK=$(shell cat $(buildconfig) | grep -w "LICHEE_SPL_BOARD_MK" | awk -F= '{printf $$2}')
	export LICHEE_BUSSINESS LICHEE_CHIP_CONFIG_DIR LICHEE_BOOT0_BIN_NAME LICHEE_IC LICHEE_PLAT_OUT LICHEE_BOARD LICHEE_SPL_BOARD_MK
endif

CPU ?= armv7

#check input param
MK_FILE_CHANGE := $(shell if [ x$(p) != x ]; then echo yes; else echo no; fi;)
#$(info MK_FILE_CHANGE=$(MK_FILE_CHANGE))
ifeq (x$(MK_FILE_CHANGE),xyes)
	FILE_EXIST=$(shell if [ -f $(TOPDIR)/board/$(p)/common.mk ]; then echo yes; else echo no; fi;)
ifeq (x$(FILE_EXIST),xno)
$(info ***);
$(info ***configure plat or module not exist, Please run some configurator);
$(info ***e.g. make p=sun50iw3p1; make boot0);
$(info ***e.g. make p=sun50iw3p1; make sboot);
$(info ***e.g. make p=sun50iw3p1; make fes);
$(info ***e.g. copy spl file to spi-pub run then following command);
$(info ***e.g. make p=sun50iw3p1 C=1; make fes C=1)
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make boot0);
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make sboot);
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make fes);
$(info ***);
$(error exit);
endif
$(shell cp -f $(TOPDIR)/board/$(p)/common.mk $(TOPDIR)/.module.common.mk)
BOARD_LIST=$(subst -,_,$(subst .,_,$(shell ls $(LICHEE_CHIP_CONFIG_DIR)/configs)))
TMP=$(subst -,_,$(subst .,_,$(LICHEE_BOARD)))
TMP_IC=$(subst -,_,$(subst .,_,$(LICHEE_IC)))
$(shell CNT=0; for TEMP_BOARD in ${BOARD_LIST} ;\
do CNT=`expr $${CNT} + 1`;\
echo CFG_$${TEMP_BOARD}=$${CNT} >> $(TOPDIR)/.module.common.mk;\
if [ "CFG_${TMP}" = "CFG_$${TEMP_BOARD}" ];\
then echo CFG_LICHEE_BOARD=$${CNT} >> $(TOPDIR)/.module.common.mk;fi;done)
$(shell sed -i '$$a\CFG_${TMP_IC}=1' $(TOPDIR)/.module.common.mk)
$(shell sed -i '$$a\DRAM_TYPE_NAME=$(DRAM_TYPE_NAME)' $(TOPDIR)/.module.common.mk)
endif

#include config file
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/mk/checkconf.mk
sinclude $(TOPDIR)/.module.common.mk


ifeq ($(shell echo ${LICHEE_BOARD} |grep -o fpga ),fpga)
	CFG_FPGA_PLATFORM=y
	export CFG_FPGA_PLATFORM
endif


CP_BOARD=$(filter $(BOARD), $(SUPPORT_BOARD))
ifeq (x$(CP_BOARD), x)
ifneq (x$(LICHEE_IC), x)
CP_BOARD=$(LICHEE_IC)
endif
ifeq (x$(CP_BOARD), x)
CP_BOARD=$(PLATFORM)
endif
endif

export Q SOC TOPDIR SRCTREE CMD_ECHO_SILENT MODULE PLATFORM DRAM_TYPE_NAME CP CPU CP_BOARD MANUFACTURER

# $(info platform=$(PLATFORM) module=$(MODULE))

define build-boot0-module-if-exist
	@if [ x$(NAND_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/nboot nand; fi
	@if [ x$(MMC_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/nboot mmc; fi
	@if [ x$(SPINOR_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/nboot spinor; fi
	@if [ x$(UFS_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/nboot ufs; fi
	@if [ x$(SIMPLEBOOT_EXIST) = xyes ] && [ x$(NAND_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/simpleboot nand; fi
	@if [ x$(SIMPLEBOOT_EXIST) = xyes ] && [ x$(MMC_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/simpleboot mmc; fi
	@if [ x$(SIMPLEBOOT_EXIST) = xyes ] && [ x$(SPINOR_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/simpleboot spinor; fi
	@if [ x$(PMBOOT_EXIST) = xyes ] && [ x$(NAND_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/pmboot nand; fi
	@if [ x$(PMBOOT_EXIST) = xyes ] && [ x$(MMC_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/pmboot mmc; fi
	@if [ x$(PMBOOT_EXIST) = xyes ] && [ x$(SPINOR_EXIST) = xyes ]; then  \
		$(MAKE) -B -C  $(SRCTREE)/pmboot spinor; fi
endef

define build-sboot-module-if-exist
	@if [ x$(SBOOT_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/sboot sboot; fi
	@if [ x$(SBOOT_NOR_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/sboot sboot_nor; fi
	@if [ x$(SBOOT_MMC_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/sboot sboot_mmc; fi
	@if [ x$(SBOOT_NAND_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/sboot sboot_nand; fi
	@if [ x$(SBOOT_UFS_EXIST) = xyes ]; then  \
	$(MAKE) -B -C  $(SRCTREE)/sboot sboot_ufs; fi
endef

define update-commit-info
	$(SRCTREE)/tools/generate_hash_header_file.sh > \
		$(SRCTREE)/include/commit_info.h.tmp
	$(call mv-if-changed,$(SRCTREE)/include/commit_info.h.tmp,$(SRCTREE)/include/commit_info.h)
	$(CP) $(TOPDIR)/include/commit_info.h $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
endef

MAKEFLAGS +=  --no-print-directory

NAND_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/nand.mk ]; then echo yes; else echo no; fi;)
MMC_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/mmc.mk ]; then echo yes; else echo no; fi;)
UFS_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/ufs.mk ]; then echo yes; else echo no; fi;)
SPINOR_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/spinor.mk ]; then echo yes; else echo no; fi;)
SBOOT_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot.mk ]; then echo yes; else echo no; fi;)
SBOOT_NOR_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot_nor.mk ]; then echo yes; else echo no; fi;)
SBOOT_MMC_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot_mmc.mk ]; then echo yes; else echo no; fi;)
SBOOT_NAND_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot_nand.mk ]; then echo yes; else echo no; fi;)
SBOOT_UFS_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot_ufs.mk ]; then echo yes; else echo no; fi;)
PMBOOT_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/pmboot.mk ]; then echo yes; else echo no; fi;)
SIMPLEBOOT_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/simpleboot.mk ]; then echo yes; else echo no; fi;)
ifeq (x$(MK_FILE_CHANGE),xyes)
#makefile is called to config platform, do nothing
all:
	@echo "platform set to $(PLATFORM)"
	$(MKDIR) -p $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
	$(CP) $(TOPDIR)/board/$(PLATFORM)/*.mk $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
	$(CP) -rf $(TOPDIR)/include/*	$(TOPDIR)/../spl-pub/include/
	$(CP) -rf $(TOPDIR)/mk/*	$(TOPDIR)/../spl-pub/mk/
	$(CP) -rf $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/*.lds $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/*_entry.S  $(TOPDIR)/../spl-pub/arch/$(ARCH)/cpu/$(CPU)/
	$(CP) -rf $(SRCTREE)/fes/main/fes1_*.c $(TOPDIR)/../spl-pub/fes/main/
	$(CP) -rf $(SRCTREE)/nboot/main/boot0_*.c $(TOPDIR)/../spl-pub/nboot/main/
	$(CP) -rf $(SRCTREE)/sboot/main/sboot_main.c $(SRCTREE)/sboot/main/sboot_head.c $(TOPDIR)/../spl-pub/sboot/main/
	$(CP) -rf $(TOPDIR)/solution $(TOPDIR)/../spl-pub/
else
all: mkdepend
	$(MAKE) -B -C $(SRCTREE)/fes fes
	$(call build-boot0-module-if-exist)
ifneq (x$(BOARD_BUILD_NBOOT),x)
	@for mk in $(BOARD_BUILD_NBOOT);\
	do\
		$(MAKE) -B -C $(SRCTREE)/nboot $$mk;\
	done
ifneq (x$(PMBOOT_EXIST),x)
	@for mk in $(BOARD_BUILD_NBOOT); \
	do \
		$(MAKE) -B -C  $(SRCTREE)/pmboot $$mk; \
	done
endif
ifneq (x$(SIMPLEBOOT_EXIST),x)
	@for mk in $(BOARD_BUILD_NBOOT); \
	do \
		$(MAKE) -B -C  $(SRCTREE)/simpleboot $$mk; \
	done
endif
else
	$(warning not nboot board target!!!)
endif
	$(call build-sboot-module-if-exist)
endif

$(BOARD_BUILD_NBOOT):mkdepend
	$(MAKE) -B -C  $(SRCTREE)/nboot $@

ifeq (x$(NAND_EXIST),xyes)
nand: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/nboot nand
endif
ifeq (x$(MMC_EXIST),xyes)
mmc: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/nboot mmc
endif

ifeq (x$(UFS_EXIST),xyes)
ufs: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/nboot ufs
endif


ifeq (x$(SPINOR_EXIST),xyes)
spinor: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/nboot spinor
endif

fes: mkdepend
	$(MAKE) -B -C $(SRCTREE)/fes fes
boot0: mkdepend
	$(call build-boot0-module-if-exist)
ifneq (x$(BOARD_BUILD_NBOOT),x)
	@for mk in $(BOARD_BUILD_NBOOT);\
	do\
		$(MAKE) -B -C $(SRCTREE)/nboot $$mk;\
	done
else
	$(warning not nboot board target!!!)
endif

ifeq (x$(SIMPLEBOOT_EXIST),xyes)
simpleboot: mkdepend
	$(MAKE) -B -C $(SRCTREE)/simpleboot simpleboot;
endif

ifeq (x$(PMBOOT_EXIST),xyes)
pmboot: mkdepend
	$(call build-boot0-module-if-exist)
ifneq (x$(PMBOOT_EXIST),x)
	@for mk in $(BOARD_BUILD_NBOOT);\
	do\
		$(MAKE) -B -C $(SRCTREE)/pmboot $$mk;\
	done
else
	$(warning not nboot board target!!!)
endif
ifeq (x$(NAND_EXIST),xyes)
pmboot_nand: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/pmboot nand
endif
ifeq (x$(MMC_EXIST),xyes)
pmboot_mmc: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/pmboot mmc
endif
ifeq (x$(SPINOR_EXIST),xyes)
pmboot_spinor: mkdepend
	$(MAKE) -B -C  $(SRCTREE)/pmboot spinor
endif
endif

ifeq (x$(SBOOT_EXIST),xyes)
sboot: mkdepend
	$(call build-sboot-module-if-exist)
endif

ifeq (x$(SBOOT_NOR_EXIST),xyes)
sboot_nor: mkdepend
	$(MAKE) -B -C $(SRCTREE)/sboot sboot_nor
endif

ifeq (x$(SBOOT_MMC_EXIST),xyes)
sboot_mmc: mkdepend
	$(MAKE) -B -C $(SRCTREE)/sboot sboot_mmc
endif

ifeq (x$(SBOOT_NAND_EXIST),xyes)
sboot_nand: mkdepend
	$(MAKE) -B -C $(SRCTREE)/sboot sboot_nand
endif

ifeq (x$(SBOOT_UFS_EXIST),xyes)
sboot_ufs: mkdepend
	$(MAKE) -B -C $(SRCTREE)/sboot sboot_ufs
endif

offline_secure_spinor: mkdepend
	$(MAKE) -B -C $(SRCTREE)/nboot $@
offline_secure_nand: mkdepend
	$(MAKE) -B -C $(SRCTREE)/nboot $@
offline_secure_mmc: mkdepend
	$(MAKE) -B -C $(SRCTREE)/nboot $@
offline_secure_ufs: mkdepend
	$(MAKE) -B -C $(SRCTREE)/nboot $@


simulate: mkdepend
	$(MAKE) -B -C $(SRCTREE)/nboot $@

clean:
	@find $(TOPDIR) -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'	-o -name '*.exe' -o -name '*.axf' \
		-o -name '*.elf' \
		-o -name '*.depend' \
		-o -name '*.bin' \
		-o -name '*.log' \
		-o -name '*.map' \) -print \
		| xargs rm -f

	@rm -f $(TOPDIR)/nboot/boot0.lds
	@rm -f $(TOPDIR)/fes/fes1.lds
	@rm -f $(TOPDIR)/sboot/sboot.lds
	@rm -f $(TOPDIR)/sboot/sboot_nor.lds
	@rm -f $(TOPDIR)/autoconf.mk
	@rm -rf $(TOPDIR)/out

distclean: clean
	@rm -f $(TOPDIR)/include/config.h
	@rm -f $(TOPDIR)/.module.common.mk
	@rm -f $(cleanfiles)

mkdepend :
	@if [ ! -f $(TOPDIR)/.module.common.mk ]; then  \
	rm -rf autoconf.mk; \
	echo "***"; \
	echo "*** Configuration file \".module.common.mk\" not found!"; \
	echo "***"; \
	echo "*** Please run some configurator (e.g. make p=sun50iw3p1)"; \
	echo "***"; \
	exit 1; \
	fi;
	$(call update-commit-info)
depend:

PHONY +=FORCE
FORCE:
.PHONY:$(PHONY)



