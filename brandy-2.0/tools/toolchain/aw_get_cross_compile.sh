#!/bin/bash

manifest=(
arm:gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi:arm-linux-gnueabi-:1.0
arm64:gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu:aarch64-none-linux-gnu-:1.0
riscv64:riscv64-linux-x86_64-20200528:riscv64-unknown-linux-gnu-:1.0
riscv32:nds32le-linux-glibc-v5d:riscv32-unknown-linux-:1.0
ads_rv32:nds32le-linux-glibc-v5d:riscv32-unknown-linux-:1.0
xt_rv32:Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.8.1:riscv64-unknown-linux-gnu-:1.0
)


CURRENT_DIR=$(dirname "$(readlink -f "$0")")
CURRENT_FILE=$(basename "$0")
# echo "Current directory: $CURRENT_DIR"
# echo "Current file: $CURRENT_FILE"

PLATFORM_CROSS_CFG=aw_platform_cross_config.mk

select_cross_compile()
{
	local select_ARCH=$1  # eg: "arm" or "arm64"
	local index=0

	# Load manifest[] from manifest.sh
	local count=0

	for line in ${manifest[@]}; do
		arr_ARCH[$count]=`echo $line | awk -F: '{print $1}'`

		printf "%2d. ${arr_ARCH[$count]}\n" $count
        if [ "x${arr_ARCH[$count]}" == "x$select_ARCH" ]; then
            index=$count
			printf "index %2d \n" $index
            break
        fi
		let count=$count+1
	done

	local AW_ARCH=${select_ARCH}
	local active_manifest=${manifest[$index]}
	local AW_CROSS_COMPILE_DIR=`echo ${active_manifest} | awk -F: '{print $2}'`
	local AW_CROSS_COMPILE=`echo ${active_manifest} | awk -F: '{print $3}'`
	if [ ! -d ${CURRENT_DIR}/${AW_CROSS_COMPILE_DIR} ]; then
		printf aw brandy2.0 toolchain Prepare toolchain ...
		mkdir -p ${CURRENT_DIR}/${AW_CROSS_COMPILE_DIR}
		tar --strip-components=1 -xf ${CURRENT_DIR}/${AW_CROSS_COMPILE_DIR}.tar.xz -C ${CURRENT_DIR}/${AW_CROSS_COMPILE_DIR}
	fi

	rm -f ${PLATFORM_CROSS_CFG}
	touch ${PLATFORM_CROSS_CFG}
	echo "# auto-generated t-coffer configuration file" > ${PLATFORM_CROSS_CFG}
	echo -e "export AW_ARCH=${AW_ARCH}" >> ${PLATFORM_CROSS_CFG}
	echo -e "export AW_CROSS_COMPILE_DIR=${AW_CROSS_COMPILE_DIR}" >> ${PLATFORM_CROSS_CFG}
	echo -e "export AW_CROSS_COMPILE=${AW_CROSS_COMPILE}" >> ${PLATFORM_CROSS_CFG}
	# echo -e "export CROSS_COMPILE=./../tools/toolchain/${AW_CROSS_COMPILE_DIR}/bin/${AW_CROSS_COMPILE}" >> ${PLATFORM_CROSS_CFG}


}

select_cross_compile $1
exit 0
