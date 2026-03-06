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

TOOL_PATH ?= ~/tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/
CC      = $(TOOL_PATH)/riscv64-unknown-elf-gcc
LD      = $(TOOL_PATH)/riscv64-unknown-elf-ld
ASM     = $(TOOL_PATH)/riscv64-unknown-elf-as
OBJDUMP = $(TOOL_PATH)/riscv64-unknown-elf-objdump
OBJCOPY = $(TOOL_PATH)/riscv64-unknown-elf-objcopy

BIN2HEX = bin2hex

MAKE_DEBUG1 ?= NONE
MAKE_DEBUG2 ?= NONE
XIP_EN ?= NONE

SRCDIRS_ALL=$(shell find . -type d)
SRCDIRS = $(filter-out $(DIR_IGNORE),$(SRCDIRS_ALL))
INCLUDES:=$(INCLDES) $(foreach dir, $(SRCDIRS), -I$(dir))

FILE = e907_boot
PRJNAME = e907_boot
MARCH_FLAGS=
CFLAGS:= -c -Os -D$(MAKE_DEBUG1) -D$(MAKE_DEBUG2)
SFLAGS:= -c

FLAG_ABI =
FLAG_MARCH = -mtune=e907

ENDIAN_MODE :=little-endian

FLAG_MARCH += -march=rv32imafcxthead
FLAG_ABI    = -mabi=ilp32f

CFLAGS += ${FLAG_MARCH} ${FLAG_ABI} -ffunction-sections -fdata-sections -Wall
SFLAGS += ${FLAG_MARCH} ${FLAG_ABI}
SOURCE    = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.[csS]))
OBJS      = $(patsubst %.c, %.o, $(SOURCE))
LINKFLAGS = -nostartfiles -Wl,--gc-sections ${FLAG_MARCH} ${FLAG_ABI}
LINKLIBS  = -lc -lgcc

all:pattern

${PRJNAME}.elf:$(OBJS)
	@${CC} -T brom.ld ${LINKFLAGS} ${LINKLIBS} ${OBJS} -o $@ -lm

pattern:$(PRJNAME).elf
	${OBJCOPY} -O binary -j .text -j .rodata -j .data -j .bss $< ${FILE}.bin
	${OBJDUMP} -D -t ${FILE}.elf > ${FILE}.txt

%.o: %.c
	@$(CC) $(MARCH_FLAGS) $(CFLAGS) ${INCLUDES} $< -o $@

%.o: %.S
	@$(CC) $(MARCH_FLAGS) $(CFLAGS) $< -o $@

%.o: %.s
	@$(CC) $(MARCH_FLAGS) $(CFLAGS) $< -o $@


echo:
	@echo "DIR_IGNORE =${DIR_IGNORE}"
	@echo "SRCDIRS_ALL=${SRCDIRS_ALL}"
	@echo "SRCDIRS    =${SRCDIRS}"
	@echo "SOURCE     =${SOURCE}"
	@echo "PRJNAME    =${PRJNAME}"
	@echo "PRJDIR     =${PRJDIR}"


.PHONY: clean
clean:
	find .  -type f -name "*.o"  -delete
	find .  -type f -name "*.d"  -delete
	find .  -type f -name "*.bin"  -delete
	find .  -type f -name "*.hex"  -delete
	find .  -type f -name "*.objdump"  -delete
	rm -rf $(PRJNAME).elf
