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
LIB	=  libdrivers.o

COBJS-y	+= serial.o
COBJS-y	+= pinmux.o
COBJS-y	+= rtc.o
COBJS-y += spi-mem.o
COBJS-$(CFG_SUNXI_SPI) += spi.o
COBJS-$(CFG_SUNXI_SPIF) += spif.o
COBJS-$(CFG_SUNXI_RSB) += rsb.o
COBJS-$(CFG_SUNXI_TWI) += i2c.o
COBJS-$(CFG_SUNXI_PMIC) += pmic_bus.o
COBJS-$(CFG_SUNXI_STANDBY) += standby.o
COBJS-$(CFG_SUNXI_WATCHDOG) += watchdog.o
COBJS-$(CFG_SUNXI_DMA) += sunxi_dma.o
COBJS-$(CFG_SUNXI_HWSPINLOCK) += hwspinlock/hwspinlock.o
COBJS-$(CFG_AW_CHIPS) += chips/sunxi-chips.o

ifdef CFG_SUNXI_PHY_KEY
COBJS-$(CFG_LRADC_KEY) += lrkey.o
COBJS-$(CFG_GPADC_KEY) += gpkey.o
COBJS-$(CFG_GPIO_KEY) += gpiokey.o
endif

LIBS-$(CFG_SUNXI_POWER) += drivers/power/libpower.o
LIBS-y += drivers/dram/libdram.o

ifndef CFG_X509_CERT_DISABLE
LIBS-$(CFG_SUNXI_SBOOT) += sboot/openssl/libopenssl.o
endif

LIBS-y += drivers/sunxi_flashmap/libsunxi_flashmap.o

check_nand_drive=$(shell if [ -f $(TOPDIR)/drivers/nand/$(PLATFORM)/Makefile ]; then echo yes; else echo no; fi;)
ifeq (x$(check_nand_drive), xyes)
	LIBS-$(CFG_SUNXI_NAND) += drivers/nand/$(PLATFORM)/libnand.o
else
	LIBS-$(CFG_SUNXI_NAND) += drivers/nand/general/libnand.o
endif

ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_NAND) += nboot/load_image_nand/libloadnand.o
endif

check_spinand=$(shell if [ -d $(TOPDIR)/nboot/load_image_spinand ]; then echo yes; else echo no; fi;)
ifeq (x$(check_spinand), xyes)
ifeq (x$(check_nand_drive), xyes)
	LIBS-$(CFG_SUNXI_SPINAND) += drivers/nand/$(PLATFORM)/libspinand.o
else
	LIBS-$(CFG_SUNXI_SPINAND) += drivers/nand/general/libspinand.o
endif

ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SPINAND) += nboot/load_image_spinand/libloadspinand.o
endif
endif

check_mmc_drive=$(shell if [ -d $(TOPDIR)/drivers/mmc/$(PLATFORM) ]; then echo yes; else echo no; fi;)
ifeq (x$(check_mmc_drive), xyes)
	LIBS-$(CFG_SUNXI_SDMMC)$(CFG_SUNXI_MMC) += drivers/mmc/$(PLATFORM)/libmmc.o
else
	LIBS-$(CFG_SUNXI_SDMMC)$(CFG_SUNXI_MMC) += drivers/mmc/general/libmmc.o
endif

check_ufs_drive=$(shell if [ -d $(TOPDIR)/drivers/ufs/$(PLATFORM) ]; then echo yes; else echo no; fi;)
ifeq (x$(check_ufs_drive), xyes)
	LIBS-$(CFG_SUNXI_UFS) += drivers/ufs/$(PLATFORM)/libufs.o
else
	LIBS-$(CFG_SUNXI_UFS) += drivers/ufs/libufs.o
endif


ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SDMMC) += nboot/load_image_mmc/libloadmmc.o
endif

ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_UFS) += nboot/load_image_ufs/libloadufs.o
endif



LIBS-$(CFG_SUNXI_SPINOR) += drivers/spinor/libspinor.o
ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SPINOR) += nboot/load_image_spinor/libloadspinor.o
endif

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))


SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
OBJS	+= $(LIBS)
all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))
#########################################################################

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk
