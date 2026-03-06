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


# Load generated board configuration
sinclude $(TOPDIR)/include/autoconf.mk
ifneq ($(SKIP_AUTO_CONF),yes)
sinclude $(TOPDIR)/autoconf.mk
endif

ifeq (x$(CPU), xriscv64)
#riscv64 ;xiantie riscv64
TOOLCHAIN_PATH:=riscv64-linux-x86_64-20200528
CROSS_COMPILE := $(TOPDIR)/../tools/toolchain/riscv64-linux-x86_64-20200528/bin/riscv64-unknown-linux-gnu-
else ifeq (x$(CPU), xriscv32)
#xt_rv32 ; xuantie riscv32
ifeq (x$(MANUFACTURER), xxuantie_c90x)
	TOOLCHAIN_PATH:=Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.0-rc5-20240319
	CROSS_COMPILE := $(TOPDIR)/../tools/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.0-rc5-20240319/bin/riscv64-unknown-linux-gnu-
else
	TOOLCHAIN_PATH:=Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.8.1
	CROSS_COMPILE := $(TOPDIR)/../tools/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.8.1/bin/riscv64-unknown-linux-gnu-
endif
else ifeq (x$(CPU), xads_rv32)
#riscv32 ; andes riscv32
TOOLCHAIN_PATH:=nds32le-linux-glibc-v5d
CROSS_COMPILE := $(TOPDIR)/../tools/toolchain/nds32le-linux-glibc-v5d/bin/riscv32-unknown-linux-
else
#arm
TOOLCHAIN_PATH:=gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi
CROSS_COMPILE := $(TOPDIR)/../tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-
endif
toolchain_check=$(shell if [ ! -e $(TOPDIR)/../tools/toolchain/$(TOOLCHAIN_PATH)/.prepared ]; then echo yes; else echo no; fi;)
ifeq (x$(toolchain_check), xyes)
$(info $(CPU)...);
$(info Prepare toolchain ...);
$(shell mkdir -p $(TOPDIR)/../tools/toolchain/$(TOOLCHAIN_PATH) || exit 1)
$(shell tar --strip-components=1 -xf $(TOPDIR)/../tools/toolchain/$(TOOLCHAIN_PATH).tar.xz -C $(TOPDIR)/../tools/toolchain/$(TOOLCHAIN_PATH) || exit 1)
$(shell touch $(TOPDIR)/../tools/toolchain/$(TOOLCHAIN_PATH)/.prepared)
endif

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
LDR		= $(CROSS_COMPILE)ldr
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

##########################################################
ifeq (x$(CPU), xriscv64)
MARCH := rv64gcvxthead
MABI := lp64

COMPILEINC :=  -isystem $(shell dirname `$(CC)  -print-libgcc-file-name`)/../../include
SPLINCLUDE    := \
		-I$(SRCTREE)/include \
		-I$(SRCTREE)/include/arch/riscv/ \
		-I$(SRCTREE)/include/configs/ \
		-I$(SRCTREE)/include/arch/$(PLATFORM)/ \
		-I$(SRCTREE)/include/openssl/

 COMM_FLAGS := -nostdinc  $(COMPILEINC) \
	-g  -Os   -fno-common \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__  \
	-march=$(MARCH)\
	-mabi=$(MABI)\
	-fno-stack-protector \
	-Wall \
	-Werror \
	-Wstrict-prototypes \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe

else ifeq (x$(CPU), xriscv32)

ifeq (x$(MANUFACTURER), xxuantie_c90x)
MARCH := rv32imafdcv_zicbom_zicbop_zicboz_zicond_zihintntl_zihintpause_zawrs_zfa_zfbfmin_zfh_zba_zbb_zbc_zbs_zvfbfmin_zvfbfwma_svinval_svnapot_svpbmt_xtheadc_xtheadvdot
MTUNE := c907
else
MARCH := rv32imacxtheade
MTUNE := e907
endif

MABI := ilp32
LD    += -m elf32lriscv
COMPILEINC :=  -isystem $(shell dirname `$(CC)  -print-libgcc-file-name`)/../../include
SPLINCLUDE    := \
		-I$(SRCTREE)/include \
		-I$(SRCTREE)/include/arch/riscv/ \
		-I$(SRCTREE)/include/configs/ \
		-I$(SRCTREE)/include/arch/$(PLATFORM)/ \
		-I$(SRCTREE)/include/openssl/

 COMM_FLAGS := -nostdinc  $(COMPILEINC) \
	-g  -Os   -fno-common \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__  \
	-march=$(MARCH)\
	-mabi=$(MABI)\
	-mtune=$(MTUNE) \
	-fno-stack-protector \
	-Wall \
	-Werror \
	-mcmodel=medany \
	-Wstrict-prototypes \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe

else ifeq (x$(CPU), xads_rv32)
MARCH := rv32imafdcxandes
MABI := ilp32d

COMPILEINC :=  -isystem $(shell dirname `$(CC)  -print-libgcc-file-name`)/../../include
SPLINCLUDE    := \
		-I$(SRCTREE)/include \
		-I$(SRCTREE)/include/arch/riscv/ \
		-I$(SRCTREE)/include/configs/ \
		-I$(SRCTREE)/include/arch/$(PLATFORM)/ \
		-I$(SRCTREE)/include/openssl/

 COMM_FLAGS := -nostdinc  $(COMPILEINC) \
	-g  -Os   -fno-common \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__  \
	-march=$(MARCH)\
	-mabi=$(MABI)\
	-fno-stack-protector \
	-Wall \
	-Werror \
	-Wstrict-prototypes \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe

else
# ifeq (x$(CPU), xarm)

ifeq ($(CFG_SUNXI_USE_NEON),y)
FLOAT_FLAGS := -mfloat-abi=softfp
else
FLOAT_FLAGS := -msoft-float
endif

COMPILEINC :=  -isystem $(shell dirname `$(CC)  -print-libgcc-file-name`)/include
SPLINCLUDE    := \
		-I$(SRCTREE)/include \
		-I$(SRCTREE)/include/arch/arm/ \
		-I$(SRCTREE)/include/configs/ \
		-I$(SRCTREE)/include/arch/$(PLATFORM)/ \
		-I$(SRCTREE)/include/openssl/

 COMM_FLAGS := -nostdinc  $(COMPILEINC) \
	-g  -Os   -fno-common -mfpu=neon  $(FLOAT_FLAGS) \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__  \
	-DCONFIG_ARM -D__ARM__ \
	-D__NEON_SIMD__  \
	-mabi=aapcs-linux \
	-mthumb-interwork \
	-fno-stack-protector \
	-Wall \
	-Werror \
	-Wstrict-prototypes \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe

 COMM_FLAGS += -mno-unaligned-access
 COMM_FLAGS += -D__LINUX_ARM_ARCH__=7
endif
# $(CPU)

C_FLAGS += $(SPLINCLUDE)   $(COMM_FLAGS)
S_FLAGS += $(SPLINCLUDE)   -D__ASSEMBLY__  $(COMM_FLAGS)
LDFLAGS_GC += --gc-sections
#LDFLAGS += --gap-fill=0xff
export LDFLAGS_GC
###########################################################

###########################################################
PLATFORM_LIBGCC = -L $(shell dirname `$(CC) $(C_FLAGS) -print-libgcc-file-name`) -lgcc
export PLATFORM_LIBGCC
###########################################################

# Allow boards to use custom optimize flags on a per dir/file basis
ALL_AFLAGS = $(AFLAGS)  $(PLATFORM_RELFLAGS) $(S_FLAGS)
ALL_CFLAGS = $(CFLAGS)  $(PLATFORM_RELFLAGS) $(C_FLAGS)
export ALL_CFLAGS ALL_AFLAGS


$(obj)%.o:	%.S
	$(Q)$(CC)  $(ALL_AFLAGS) -o $@ $< -c
	@echo " CC      "$< ...
$(obj)%.o:	%.c
	$(Q)$(CC)  $(ALL_CFLAGS) -o $@ $< -c
	@echo " CC      "$< ...

#########################################################################

# If the list of objects to link is empty, just create an empty built-in.o
cmd_link_o_target = $(if $(strip $1),\
			  $(LD) $(LDFLAGS) -r -o $@ $1,\
		      rm -f $@; $(AR) rcs $@ )

#########################################################################

