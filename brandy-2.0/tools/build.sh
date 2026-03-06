#!/bin/bash

#
#build.sh for uboot/spl
#wangwei@allwinnertech
#

TOP_DIR=$(cd `dirname $0`;pwd;cd - >/dev/null 2>&1)
BRANDY_SPL_DIR=$TOP_DIR/spl
BRANDY_SPL_PUB_DIR=$TOP_DIR/spl-pub
SPL_OLD=$(grep  ".module.common.mk"  ${BRANDY_SPL_DIR}/Makefile)
UBOOT_NAME=""   # eg: u-boot-2023
UBOOT_DIR=""
UBOOT_VER=""
UBOOT_ARCH=""
UBOOT_BUILD_FLAG=""   # eg: UC=arm64
UBOOT_BSP_DIR="$TOP_DIR/u-boot-bsp"
UBOOT_CONFIGS_PRE_DIR="configs"
UBOOT_DTB_NAME=""
UBOOT_ARCH_DIR=""
FLASHMAP_ENABLE=""
TARGET_CROSS_COMPILE=''
set -e

_MAKE=$TOP_DIR/tools/make_dir/make4.1/bin/make
_FLASHMAP_TOOL=$TOP_DIR/../..//tools/pack/pctools/linux/mod_update/sunxi_flashmap_tool
echo "_MAKE PATH: $_MAKE"

uboot_cross_compiler=(
'arm		arm		gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi.tar.xz	 /bin/arm-linux-gnueabi-'
'arm64		aarch64		gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz	 /bin/aarch64-none-linux-gnu-'
'rv32i_ads	riscv32		nds32le-linux-glibc-v5d.tar.xz					 /bin/riscv32-unknown-linux-'
'rv32i		riscv32		Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410.tar.gz  /bin/riscv64-unknown-linux-gnu-'
'e90x		riscv32		Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.8.1.tar.xz		 /bin/riscv64-unknown-linux-gnu-'
'rv64i		riscv64		riscv64-linux-x86_64-20200528.tar.xz				 /bin/riscv64-unknown-linux-gnu-'
)

get_sys_config_name()
{
	[ -f $UBOOT_DIR/.config ] && \
	awk -F '=' '/CONFIG_SYS_CONFIG_NAME=/{print $2}' $UBOOT_DIR/.config | sed 's|"||g'
}

show_help()
{
	printf "\nbuild.sh - Top level build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -t install gcc tools chain"
	echo "  -o build,e.g. uboot,spl,clean"
	echo "  -p <platform> platform, e.g. sun8iw18p1,  sun5i, sun6i, sun8iw1p1, sun8iw3p1, sun9iw1p1"
	echo "  -m mode,e.g. nand,mmc,nor"
	echo "  -c copy,e.g. any para"
	echo "example:"
	echo "./build.sh -o uboot -p sun8iw18p1"
	echo "./build.sh -o spl -p sun8iw18p1 -m nand"
	echo "./build.sh -o spl -p sun8iw18p1 -m nand -c copy"
	printf "\n\n"
}

#get uboot arch from -a or LICHEE_BRANDY_UBOOT_ARCH, default arm
if [ "x${UBOOT_ARCH}" = "x" ];then
	UBOOT_ARCH=${LICHEE_BRANDY_UBOOT_ARCH}
fi
if [ "x${UBOOT_ARCH}" = "x" ];then
	UBOOT_ARCH="arm"
fi

function parse_cross_compiler()
{
	local scan_compiler_table=$1
	local toolchain_path=''
	local toolchain_name=''
	local cross_compile_suffix=''
	local toolchain_tar_path=''
	local toolchain_tar_name=''
	local arch=''

	eval "array_ref=(\"\${$scan_compiler_table[@]}\")"

	for i in "${array_ref[@]}" ; do
		local arr=($i)
		for j in "${arr[@]}" ; do
			if [ ${j} = $2 ] ; then
				if [ -z "${LICHEE_UBOOT_TOOLCHAIN}" ]; then
					arch=${arr[1]}
					toolchain_tar_name=${arr[2]}
					toolchain_name=${toolchain_tar_name%.tar*}
					cross_compile_suffix=${arr[3]}
					break 2
				else
					arch=${arr[1]}
					toolchain_tar_name=${LICHEE_UBOOT_TOOLCHAIN}
					toolchain_name=${toolchain_tar_name%.tar*}
					cross_compile_suffix=${arr[3]}
					if [ -n "${LICHEE_UBOOT_CROSS_COMPILER}" ]; then
						cross_compile_suffix="/bin/${LICHEE_UBOOT_CROSS_COMPILER}"
					fi
					break 2
				fi
			fi
		done
	done

	if [ "x${toolchain_tar_name}" = "x" ] ; then
		echo "can not match COMPILER_TAR. $1 $2"
		exit 1
	fi

	if [ -f "${LICHEE_TOP_DIR}/prebuilt/kernelbuilt/${arch}/${toolchain_tar_name}" ] ; then
		toolchain_tar_path=${LICHEE_TOP_DIR}/prebuilt/kernelbuilt/${arch}/${toolchain_tar_name}
		toolchain_path=${LICHEE_TOP_DIR}/out/toolchain/${toolchain_name}
	else
		toolchain_tar_path=${TOP_DIR}/tools/toolchain/${toolchain_tar_name}
		toolchain_path=${TOP_DIR}/tools/toolchain/${toolchain_name}
	fi

	if [ ! -e "${toolchain_path}/.prepared" ]; then
			echo "tar ${toolchain_path}"
			rm -rf "${toolchain_path}"
			mkdir -p ${toolchain_path} || exit 1
			tar --strip-components=1 -xf ${toolchain_tar_path} -C ${toolchain_path} || exit 1
			touch "${toolchain_path}/.prepared"
	fi

	TARGET_CROSS_COMPILE="${toolchain_path}${cross_compile_suffix}"
}

uboot_prepare_toolchain()
{
	local config_arm=''
	local config_arm64=''
	local config_riscv=''
	local config_arch_rv32i=''
	local config_arch_ads_rv32i=''
	local config_cpu_e90x=''
	local config_arch=''

	if [ -f .config ];then
		config_arm=$(awk -F= '/^CONFIG_ARM=/ {print $2}' .config)
		config_arm64=$(awk -F= '/^CONFIG_ARM64=/ {print $2}' .config)
		config_riscv=$(awk -F= '/^CONFIG_RISCV=/ {print $2}' .config)
		config_arch_ads_rv32i=$(awk -F= '/^CONFIG_CPU_A27L2=/ {print $2}' .config)
		config_arch_rv32i=$(awk -F= '/^CONFIG_ARCH_RV32I=/ {print $2}' .config)
		config_cpu_e90x=$(awk -F= '/^CONFIG_CPU_E90X=/ {print $2}' .config)
	else
		echo "please set compiler_obj"
		exit 1
	fi

	if [ "x${config_riscv}" = "xy" ]; then
		if  [ "x${config_cpu_e90x}" = "xy" ]; then
			config_arch='e90x'
		elif [ "x${config_arch_ads_rv32i}" = "xy" ]; then
			config_arch='rv32i_ads'
		elif [ "x${config_arch_rv32i}" = "xy" ]; then
			config_arch='rv32i'
		else
			config_arch='rv64i'
		fi
	elif [ "x${config_arm64}" = "xy" ]; then
		config_arch='arm64'
	else
		config_arch='arm'
	fi

	parse_cross_compiler uboot_cross_compiler ${config_arch}
}

function build_clean()
{
	(cd $TOP_DIR/spl; $_MAKE distclean)
	(cd $TOP_DIR/spl-pub; $_MAKE distclean)
	(cd $UBOOT_DIR; $_MAKE distclean)
}

build_uboot_once()
{
	local defconfig=$1
	local CONFIG_SYS_CONFIG_NAME=$(get_sys_config_name)
	local last_board=''

	if [ "x${defconfig}" = "x" ];then
		echo "please set defconfig"
		exit 1
	fi
	echo build for ${defconfig} ...
	(
	cd $UBOOT_NAME/
	if [ -f .tmp_defcofig.o.md5sum ];then
		last_defconfig_md5sum=$(awk '{printf $1}' ".tmp_defcofig.o.md5sum")
		last_board=$(awk '{printf $2}' ".tmp_defcofig.o.md5sum")
	fi
	if [ -f .config ];then
		cur_defconfig_md5sum=$(md5sum "${UBOOT_CONFIGS_PRE_DIR}/${defconfig}" | awk '{printf $1}')
		if [ "x${CONFIG_SYS_CONFIG_NAME}" != "x${PLATFORM}" ];then
			$_MAKE ${UBOOT_BUILD_FLAG} distclean
			$_MAKE ${UBOOT_BUILD_FLAG} ${defconfig}
		elif [ "x${last_defconfig_md5sum}" != "x${cur_defconfig_md5sum}" ];then
			$_MAKE ${UBOOT_BUILD_FLAG} ${defconfig}
		elif [ "x${last_board}" != "x${BOARD}" ];then
			$_MAKE ${UBOOT_BUILD_FLAG} distclean
			$_MAKE ${UBOOT_BUILD_FLAG} ${defconfig}
		fi
	else
		$_MAKE ${UBOOT_BUILD_FLAG} distclean
		$_MAKE ${UBOOT_BUILD_FLAG} ${defconfig}
	fi
	uboot_prepare_toolchain
	$_MAKE ${UBOOT_BUILD_FLAG} CROSS_COMPILE=${TARGET_CROSS_COMPILE} -j
	)
}

function uboot_create_links()
{
	#cd $UBOOT_DIR
	#git checkout .
	#git clean -dfx
	#cd -

	rm -f $UBOOT_DIR/bsp
	ln -sfr ${UBOOT_BSP_DIR}/ $UBOOT_DIR/bsp
}

function build_uboot()
{
	echo "uboot version:${UBOOT_NAME}"
	local UBOOT_CONFIG_DIR=$UBOOT_DIR/${UBOOT_CONFIGS_PRE_DIR}
	if [ "x${PLATFORM}" = "xall" ];then
		if [ "x${UBOOT_VER}" = "x2023" ];then
			UBOOT_CONFIG_DIR=${UBOOT_BSP_DIR}/${UBOOT_CONFIGS_PRE_DIR}
		fi
		for defconfig in `ls $UBOOT_CONFIG_DIR`;do
			if [[ $defconfig =~ .*_defconfig$ ]];then
				build_uboot_once $defconfig
			fi
		done
	else

		#compile u-boot-efex bin
		if [ -e $UBOOT_DIR/${UBOOT_CONFIGS_PRE_DIR}/${PLATFORM%%_*}_${LICHEE_EFEX_BIN_NAME}_defconfig ]; then
			echo "----compile u-boot-efex.bin----"
			build_uboot_once ${PLATFORM%%_*}_${LICHEE_EFEX_BIN_NAME}_defconfig
		fi

		if [ "x${LICHEE_FLASH}" = "x" ];then
			for defconfig in `ls $UBOOT_DIR/${UBOOT_CONFIGS_PRE_DIR}/${PLATFORM}_*`;do
				if [[ $defconfig =~ .*_defconfig$ ]];then
					build_uboot_once ${defconfig##*/}
				fi
			done
		elif [ "x${LICHEE_FLASH}" = "xdefault" ];then
			build_uboot_once ${PLATFORM}_defconfig
			#spi nor need same offset in different bin(one for burn, one for load), check if they are the same
			local NOR_UBOOT_OFFSET=(0 0 0 0)
			local NOR_LOGICAL_OFFSET=(0 0 0 0)
		elif [ "x${LICHEE_FLASH}" = "xnor" ];then
			if [ -e $UBOOT_DIR/${UBOOT_CONFIGS_PRE_DIR}/${PLATFORM}_nor_defconfig ]; then
				build_uboot_once ${PLATFORM}_defconfig
				NOR_UBOOT_OFFSET[0]=`grep CONFIG_SPINOR_UBOOT_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				NOR_LOGICAL_OFFSET[0]=`grep CONFIG_SPINOR_LOGICAL_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				NOR_UBOOT_OFFSET[2]=`grep CONFIG_SPINOR_UBOOT_SECURE_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				NOR_LOGICAL_OFFSET[2]=`grep CONFIG_SPINOR_LOGICAL_SECURE_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				build_uboot_once ${PLATFORM}_nor_defconfig
				NOR_UBOOT_OFFSET[1]=`grep CONFIG_SPINOR_UBOOT_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				NOR_LOGICAL_OFFSET[1]=`grep CONFIG_SPINOR_LOGICAL_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				if [  ${NOR_UBOOT_OFFSET[0]} -ne ${NOR_UBOOT_OFFSET[1]} ]; then
					echo "UBOOT OFFSET NOT MATCH, burn to ${NOR_UBOOT_OFFSET[0]}, load from ${NOR_UBOOT_OFFSET[1]}"
					exit 1
				fi
				if [  ${NOR_LOGICAL_OFFSET[0]} -ne ${NOR_LOGICAL_OFFSET[1]} ]; then
					echo "LOGICAL OFFSET NOT MATCH, burn to ${NOR_LOGICAL_OFFSET[0]}, load from ${NOR_LOGICAL_OFFSET[1]}"
					exit 1
				fi
			else
				echo "Cant find ${PLATFORM}_nor_defconfig"
				exit 1
			fi
			if [ -e $UBOOT_DIR/${UBOOT_CONFIGS_PRE_DIR}/${PLATFORM}_nor_secure_defconfig ]; then
				build_uboot_once ${PLATFORM}_nor_secure_defconfig
				NOR_UBOOT_OFFSET[3]=`grep CONFIG_SPINOR_UBOOT_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				NOR_LOGICAL_OFFSET[3]=`grep CONFIG_SPINOR_LOGICAL_OFFSET $UBOOT_NAME/.config |awk -F = '{print $2}'`
				if [ ${NOR_UBOOT_OFFSET[2]} -ne ${NOR_UBOOT_OFFSET[3]} ]; then
					echo "SECURE UBOOT OFFSET NOT MATCH, burn to ${NOR_UBOOT_OFFSET[2]}, load from ${NOR_UBOOT_OFFSET[3]}"
					exit 1
				fi
				if [ ${NOR_LOGICAL_OFFSET[2]} -ne ${NOR_LOGICAL_OFFSET[3]} ]; then
					echo "SECURE LOGICAL OFFSET NOT MATCH, burn to ${NOR_LOGICAL_OFFSET[2]}, load from ${NOR_LOGICAL_OFFSET[3]}"
					exit 1
				fi
			fi
		else
			echo "unsupport flash mode"
			exit 2
		fi
	fi
}

function build_spl-pub_once()
{
	board=$1
	mode=$2
	path=$3
	(
	cd ${path}

	if [ "x${mode}" = "xall" ];then
		echo --------build for mode:${mode} board:${board}-------------------
		$_MAKE distclean
		$_MAKE b=${board}  ${CP}
		$_MAKE -j ${CP}
	else
		echo --------build for mode:${mode} board:${board}-------------------
		$_MAKE distclean
		$_MAKE b=${board} m=${mode}  ${CP}
		case ${mode} in
			nand | mmc | spinor)
				$_MAKE boot0 -j ${CP}
				;;
			sboot_nor)
				echo "Neednot build sboot_nor ..."
				;;
			*)
				$_MAKE ${mode} -j ${CP}
				;;
		esac
	fi
	)
}


function build_spl-pub()
{
	if [ "x${BOARD}" = "xall" ];then
		for board in `ls $TOP_DIR/$1/board`;do
			if [ "x${MODE}" = "xall" ];then
				build_spl-pub_once ${board} all $1
			else
				build_spl-pub_once ${board} ${MODE} $1
			fi
		done
	elif [ "x${MODE}" = "xall" ];then
		build_spl-pub_once ${BOARD} all $1
	else
		build_spl-pub_once ${BOARD} ${MODE} $1
	fi
}

function build_spl_once()
{
	platform=$1
	mode=$2
	path=$3

	if [ "x${LICHEE_BRANDY_SPL_VER}" == "xv2" ]; then
		cd ./spl-2.0
		./build.sh -w $platform -p ${LICHEE_SPL_V2_PRODUCT_CONFIG}
		cd -
		return 0
	fi

	support_board_exit=`cat $TOP_DIR/${path}/board/${platform}/common.mk | grep -w "SUPPORT_BOARD" | awk -F= '{printf $2}'`
	if [ "x${BOARD}" != "xall" ];then
		suport_board=${BOARD}
	elif [ "x${support_board_exit}" = "x" ];then
		suport_board=null
	else
		suport_board=`cat $TOP_DIR/${path}/board/${platform}/common.mk | grep -w "SUPPORT_BOARD" | awk -F= '{printf $2}'`
	fi
	echo "suport_board:${suport_board}"
	(
	cd ${path}

	for board in ${suport_board};do
		if [ "x${mode}" = "xall" ];then
			echo --------build for platform:${platform} mode:${mode} board:${board}-------------------
			$_MAKE distclean
			$_MAKE p=${platform} b=${board} ${CP}
			$_MAKE -j b=${board} ${CP}
		else
			echo --------build for platform:${platform} mode:${mode} board:${board}-------------------
			$_MAKE distclean
			$_MAKE p=${platform} m=${mode} b=${board} ${CP}
			case ${mode} in
				nand | mmc | spinor)
					$_MAKE boot0 -j b=${board} ${CP}
					;;
				sboot_nor)
					echo "Neednot build sboot_nor ..."
					;;
				*)
					$_MAKE ${mode} -j b=${board} ${CP}
					;;
			esac
		fi
	done
	)
}


function build_spl()
{
	local CONFIG_SYS_CONFIG_NAME=$(get_sys_config_name)
	if [ ! -d $TOP_DIR/$1/board/${PLATFORM} ] && [ "x${PLATFORM}" != "xall" ];then
		PLATFORM=${CONFIG_SYS_CONFIG_NAME}
	fi
	# prepare some dts info
	[ -e ${BRANDY_SPL_DIR}/include/autogen_dts_info.h ] && rm ${BRANDY_SPL_DIR}/include/autogen_dts_info.h
	[ "x${LICHEE_GEN_BOOT0_DTS_INFO}" = "xyes" ] && [ -e ${TOP_DIR}/../../.buildconfig ] && \
			${TOP_DIR}/tools/gen_boot0_dts_head ${TOP_DIR}/../../.buildconfig ${BRANDY_SPL_DIR}/include/autogen_dts_info.h > /dev/null

	if [ "x${PLATFORM}" = "xall" ];then
		for platform in `ls $TOP_DIR/$1/board`;do
			if [ "x${MODE}" = "xall" ];then
				build_spl_once ${platform} all $1
			else
				build_spl_once $platform ${MODE} $1
			fi
		done
	elif [ "x${MODE}" = "xall" ];then
		build_spl_once ${PLATFORM} all $1
	else
		build_spl_once ${PLATFORM} ${MODE} $1
	fi
}

function build_spl_old()
{
	if [ "x${PLATFORM}" = "xall" ];then
		for platform in `ls $TOP_DIR/spl/board`;do
			if [ "x${MODE}" = "xall" ];then
				for mode in `ls ${TOP_DIR}/spl/board/${platform}`;do
					if [[ $mode =~ .*\.mk$ ]]  \
					&& [ "x$mode" != "xcommon.mk" ];then
						mode=${mode%%.mk*}
						build_spl_once ${platform} ${mode} $1
					fi
				done
			else
				build_spl_once $platform ${MODE} $1
			fi
		done
	elif [ "x${MODE}" = "xall" ];then
		for mode in `ls ${TOP_DIR}/spl/board/${PLATFORM}`;do
			if [[ $mode =~ .*\.mk$ ]]  \
			&& [ "x$mode" != "xcommon.mk" ];then
				mode=${mode%%.mk*}
				build_spl_once ${PLATFORM} ${mode} $1
			fi
		done
	else
		build_spl_once ${PLATFORM} ${MODE} $1
	fi
}

function sync_one_attribute()
{
	storage_type=$1
	dtb_attribute=$2
	mk_attribute=$3
	directory="./spl/board/${LICHEE_CHIP}/"

	attribute_value=$(${_FLASHMAP_TOOL} -f get_dtb_node -d ./u-boot-${UBOOT_VER}/arch/${UBOOT_ARCH_DIR}/dts/${UBOOT_DTB_NAME}.dtb -t ${storage_type} -a ${dtb_attribute})

	find "$directory" -type f -name "*.mk" | while read file; do
		${_FLASHMAP_TOOL} -f change_boot0_mk -c $file -a ${mk_attribute} -v ${attribute_value} -t ${storage_type}
	done

}

function boot0_flash_map_sync()
{
	storage_type=$1

	if [ "x${storage_type}" = "xdefault" ] ; then
		echo "TODO..."
	elif [ "x${storage_type}" = "xnor" ] ; then
		sync_one_attribute flash uboot_start CFG_SPINOR_UBOOT_OFFSET
		sync_one_attribute flash boot_param_start CFG_SPINOR_UBOOT_PARAMS_OFFSET
		sync_one_attribute flash logic_offset CFG_SUNXI_LOGIC_OFFSET
		sync_one_attribute flash logic_offset CFG_SPINOR_GPT_ARD
	else
		echo "storage_type type is error"
	fi
}

function uboot_flash_map_sync()
{
	UBOOT_BIN_DIR="${LICHEE_CHIP_CONFIG_DIR}/bin/"

	if [ ! -f $TOP_DIR/u-boot-${UBOOT_VER}/configs/${LICHEE_CHIP}_${LICHEE_EFEX_BIN_NAME}_defconfig ]; then
		echo "$TOP_DIR/u-boot-${UBOOT_VER}/configs/${LICHEE_CHIP}_${LICHEE_EFEX_BIN_NAME}_defconfig not exit"
		return 0
	fi

	mkdir -p ${LICHEE_BOARD_CONFIG_DIR}/bin

	cp ${LICHEE_CHIP_CONFIG_DIR}/bin/u-boot-${LICHEE_EFEX_BIN_NAME}.bin ${LICHEE_BOARD_CONFIG_DIR}/bin/u-boot-${LICHEE_EFEX_BIN_NAME}.bin
}

function flash_map_sync()
{
	if [ "x${UBOOT_VER}" = "x2023" ] ; then
		UBOOT_DTB_NAME=$(cat "./u-boot-${UBOOT_VER}/bsp/configs/${LICHEE_BRANDY_DEFCONF}" | grep -w "CONFIG_DEFAULT_DEVICE_TREE" | awk -F= '{printf $2}')
	else
		UBOOT_DTB_NAME=$(cat "./u-boot-${UBOOT_VER}/configs/${LICHEE_BRANDY_DEFCONF}" | grep -w "CONFIG_DEFAULT_DEVICE_TREE" | awk -F= '{printf $2}')
	fi
	UBOOT_DTB_NAME="${UBOOT_DTB_NAME//\"/}"

	if [ "x${LICHEE_ARCH}" = "xriscv32" ] || [ "x${LICHEE_ARCH}" = "xriscv64" ] || [ "x${LICHEE_ARCH}" = "xriscv64ilp32" ]; then
		UBOOT_ARCH_DIR=riscv
	elif [ "x${LICHEE_ARCH}" = "xarm" ] || [ "x${LICHEE_ARCH}" = "xarm64" ] ; then
		UBOOT_ARCH_DIR=arm
	else
		echo "LICHEE_ARCH is error"
		return 0
	fi

	if [ "x${UBOOT_VER}" = "x2023" ] ; then
		if [ "x${LICHEE_FLASH}" = "xdefault" ] ; then
			FLASHMAP_ENABLE=$(${_FLASHMAP_TOOL} -f check_flashmap_enable -t mmc -d ./u-boot-${UBOOT_VER}/arch/${UBOOT_ARCH_DIR}/dts/uboot-board.dtb)
		elif [ "x${LICHEE_FLASH}" = "xnor" ] ; then
			FLASHMAP_ENABLE=$(${_FLASHMAP_TOOL} -f check_flashmap_enable -t flash -d ./u-boot-${UBOOT_VER}/arch/${UBOOT_ARCH_DIR}/dts/uboot-board.dtb)
		else
			echo "LICHEE_FLASH type is error"
		fi
	else
		if [ "x${LICHEE_FLASH}" = "xdefault" ] ; then
			FLASHMAP_ENABLE=$(${_FLASHMAP_TOOL} -f check_flashmap_enable -t mmc -d ./u-boot-${UBOOT_VER}/arch/${UBOOT_ARCH_DIR}/dts/${UBOOT_DTB_NAME}.dtb)
		elif [ "x${LICHEE_FLASH}" = "xnor" ] ; then
			FLASHMAP_ENABLE=$(${_FLASHMAP_TOOL} -f check_flashmap_enable -t flash -d ./u-boot-${UBOOT_VER}/arch/${UBOOT_ARCH_DIR}/dts/${UBOOT_DTB_NAME}.dtb)
		else
			echo "LICHEE_FLASH type is error"
		fi
	fi

	if [ "x${FLASHMAP_ENABLE}" = "xenabled" ] ; then
		uboot_flash_map_sync
		boot0_flash_map_sync ${LICHEE_FLASH}
	fi
}

function save_original_firmware()
{
	# If the last compilation was interrupted, restore the file first and go back
	if [ -d ${LICHEE_CHIP_CONFIG_DIR}/temp_bin ]; then
		cp -rf ${LICHEE_CHIP_CONFIG_DIR}/temp_bin/* ${LICHEE_CHIP_CONFIG_DIR}/bin/ || true
	fi

	if [ ! -d ${LICHEE_CHIP_CONFIG_DIR}/temp_bin ]; then
		mkdir -p ${LICHEE_CHIP_CONFIG_DIR}/temp_bin || true
	fi

	cp -rf ${LICHEE_CHIP_CONFIG_DIR}/bin/* ${LICHEE_CHIP_CONFIG_DIR}/temp_bin || true
	rm -rf ${LICHEE_CHIP_CONFIG_DIR}/bin/* || true
}

function restore_original_firmware()
{
	if [ ! -d ${LICHEE_BOARD_CONFIG_DIR}/bin ]; then
		mkdir ${LICHEE_BOARD_CONFIG_DIR}/bin || true
	fi

	cp -rf ${LICHEE_CHIP_CONFIG_DIR}/bin/* ${LICHEE_BOARD_CONFIG_DIR}/bin/ || true
	rm -rf ${LICHEE_CHIP_CONFIG_DIR}/bin/* || true

	cp -rf ${LICHEE_CHIP_CONFIG_DIR}/temp_bin/* ${LICHEE_CHIP_CONFIG_DIR}/bin/ || true
	rm -rf ${LICHEE_CHIP_CONFIG_DIR}/temp_bin/ || true
}

build_opensbi_dtb()
{
	local script_path=${LICHEE_BUILD_DIR}/mkdts.sh
	local dtc_path=${LICHEE_BUILD_DIR}/bin/dtc
	local dts_path=${LICHEE_BOARD_CONFIG_DIR}/opensbi.dts
	local include_dtsi_path=${LICHEE_CHIP_CONFIG_DIR}/configs/default/
	local out_path=${LICHEE_PLAT_OUT}

	if [ ! -f ${dts_path} ]; then
		echo "No dts for opensbi"
		return 0
	fi

	if [ ! -f ${dtc_path} ]; then
		echo "opensbi compile error: ${dtc_path} not exist!"
		return 1
	fi

	echo "--- Make OPENSBI DTB"
	${script_path} -i ${dts_path} -I ${include_dtsi_path} -o ${out_path} -t ${dtc_path}
}

function build_all()
{
	build_uboot

	flash_map_sync

	if [ "x${FLASHMAP_ENABLE}" = "xenabled" ] ; then
		save_original_firmware
	fi

	if [ -d ${BRANDY_SPL_PUB_DIR} ];then
		build_spl-pub spl-pub
	fi

	if [ -d ${BRANDY_SPL_DIR} ] && [ "x${SPL_OLD}" != "x" ] ; then
			[ "x${LICHEE_BRANDY_SPL}" != "x" ] && build_${LICHEE_BRANDY_SPL} ${LICHEE_BRANDY_SPL} || build_spl spl
	elif [ -d ${BRANDY_SPL_DIR} ] && [ "x${SPL_OLD}" = "x" ] ; then
		build_spl_old spl
	fi

	if [ "x${FLASHMAP_ENABLE}" = "xenabled" ] ; then
		restore_original_firmware
	fi

	build_opensbi_dtb
}

[ -e ${TOP_DIR}/../../.buildconfig ] && source ${TOP_DIR}/../../.buildconfig

while getopts to:p:m:c:b:u:a: OPTION; do
	case $OPTION in
	t)
		echo "This command is not supported, the toolchain will be unpacked during building"
		exit $?
		;;
	o)
		if [ "x${SPL_OLD}" = "x" ] && [ "x$OPTARG" = "xspl" ]; then
			command="build_spl_old $OPTARG"
		else
			command="build_$OPTARG $OPTARG"
		fi
		;;

	p)
		#prepare_toolchain
		PLATFORM=$OPTARG
		;;
	m)
		MODE=$OPTARG
		;;
	c)
		CP=C\=$OPTION
		;;
	b)
		BOARD=$OPTARG
		UBOOT_BUILD_FLAG+=" board=${BOARD}"
		;;
	u)
		UBOOT_VER=$OPTARG
		;;
	a)
		UBOOT_ARCH=$OPTARG
		;;
	*)
		show_help
		exit $?
		;;
	esac
done

#get uboot version from -u or LICHEE_BRANDY_UBOOT_VER, default 2018
if [ "x${UBOOT_VER}" = "x" ];then
	UBOOT_VER=${LICHEE_BRANDY_UBOOT_VER}
fi
if [ "x${UBOOT_VER}" = "x" ];then
	UBOOT_VER="2018"
fi

UBOOT_NAME="u-boot-${UBOOT_VER}"
UBOOT_DIR="$TOP_DIR/$UBOOT_NAME"

if ! [ -d $UBOOT_DIR ];then
	echo "Build error: can not find uboot dir: $UBOOT_DIR "
fi
if [ "x${UBOOT_VER}" != "x2018" ];then
	echo "Independent BSP for $UBOOT_NAME"
	UBOOT_INDEPENDENT_BSP=true
	UBOOT_CONFIGS_PRE_DIR="bsp/configs"
	UBOOT_BUILD_FLAG+=" UA=${UBOOT_ARCH}"
fi

if [ "x${PLATFORM}" = "x" ];then
	PLATFORM=all
fi
if [ "x${MODE}" = "x" ];then
	MODE=all
fi
if [ "x${BOARD}" = "x" ];then
	BOARD=all
fi
# echo "PLATFORM:${PLATFORM}  MODE:${MODE}  CP:${CP}  BOARD:${BOARD} "
if [ "x${UBOOT_INDEPENDENT_BSP}" = "xtrue" ];then
	uboot_create_links
fi

if [ "x$command" != "x" ];then
	$command
else
	build_all
fi
exit $?
