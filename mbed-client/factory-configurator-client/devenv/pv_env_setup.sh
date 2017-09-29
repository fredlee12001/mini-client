# ----------------------------------------------------------------------------
# Copyright 2016-2017 ARM Ltd.
#  
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ----------------------------------------------------------------------------
# source (!) this file while at SPV tree top - DO NOT run it
export SA_PV_TOP=`pwd`

# Platforms combination {HW, OS, TOOLCHAIN}
SA_PV_PLAT_PC="PC/Linux pc linux gcc"
SA_PV_PLAT_K64F_MBEDOS_ARM_GCC="Freescale-FRDM-K64F/MBED k64f mbedos arm_gcc"
SA_PV_PLAT_K64F_MBEDOS_ARMCC="Freescale-FRDM-K64F/MBED k64f mbedos armcc"
SA_PV_PLAT_K64F_FREERTOS_ARM_GCC="Freescale-FRDM-K64F/MBED k64f freertos arm_gcc"
SA_PV_PLAT_K64F_FREERTOS_ARMCC="Freescale-FRDM-K64F/MBED k64f freertos armcc"
SA_PV_PLATS="PC K64F_MBEDOS_ARMCC K64F_MBEDOS_ARM_GCC K64F_FREERTOS_ARMCC K64F_FREERTOS_ARM_GCC"
SA_PV_PLAT_DEFAULT=PC

# Parse command line -
PLATFORM_ARG=""
QUICK=0
for ARG in $* ; do
	case "$ARG" in
		"-q" | "-Q")
			# If specified, "make deploy" will not be called.
			QUICK=1
			;;
		"-l" | "-L")
			# List all the platforms and exit.
			for plat in $SA_PV_PLATS; do
			    SA_PV_PLAT_VAR_NAME=SA_PV_PLAT_$plat
			    SA_PV_PLAT_INFO=(${!SA_PV_PLAT_VAR_NAME})
			    SA_PV_PLAT_DESC=${SA_PV_PLAT_INFO[0]}
			    printf "%-10s %s\n" "$plat" "$SA_PV_PLAT_DESC"
			done
			return
			;;
		*)
			PLATFORM_ARG=$ARG
			;;
	esac
done

if [[ "$PLATFORM_ARG" == "" ]]; then
	# If platform not specified, query the user.
	echo -n "* Please select platform [$SA_PV_PLATS]($SA_PV_PLAT_DEFAULT): "
	read LOCAL_SA_PV_PLAT
	if [[ "$LOCAL_SA_PV_PLAT" == "" ]]; then
		export SA_PV_PLAT=$SA_PV_PLAT_DEFAULT
	else
		export SA_PV_PLAT=$LOCAL_SA_PV_PLAT    
	fi
else
	echo $PLATFORM_ARG
	export SA_PV_PLAT=$PLATFORM_ARG
fi

SA_PV_PLAT_VAR_NAME=SA_PV_PLAT_$SA_PV_PLAT
SA_PV_PLAT_INFO=(${!SA_PV_PLAT_VAR_NAME})
SA_PV_PLAT_DESC=${SA_PV_PLAT_INFO[0]}
SA_PV_HW=${SA_PV_PLAT_INFO[1]}
SA_PV_OS=${SA_PV_PLAT_INFO[2]}
SA_PV_TOOLCHAIN=${SA_PV_PLAT_INFO[3]}


export PV_PLAT_VECTOR=${SA_PV_HW}_${SA_PV_OS}_${SA_PV_TOOLCHAIN}
echo "Setting up SPV development environment for ($SA_PV_PLAT ==> $PV_PLAT_VECTOR) $SA_PV_PLAT_DESC: HW=$SA_PV_HW OS=$SA_PV_OS TOOLCHAIN=$SA_PV_TOOLCHAIN"

# converting to upper case OS and TOOLCHAIN for future use as flags
export SA_PV_OS=`echo $SA_PV_OS | tr '[:lower:]' '[:upper:]'`
export SA_PV_TOOLCHAIN=`echo $SA_PV_TOOLCHAIN | tr '[:lower:]' '[:upper:]'`

spv_remove_duplicate_path_entries() {
	# This magical scriptlet removes from PATH all duplicates, keeping non-duplicated
	# items in their original order.
	# Taken from Stackoverflow.
	export PATH=$(echo "$PATH" | awk -v RS=':' -v ORS=":" '!a[$1]++{if (NR > 1) printf ORS; printf $a[$1]}')
}

spv_set_toolchain_gcc() {	
	export ARCH=`uname -m`
	unset CROSS_COMPILE
	export CC=gcc
	export CXX=g++
	export AR=ar
	export LD=ld
	export OBJCOPY=/usr/bin/objcopy #FIXME: move to /opt
	
	export PATH=/usr/bin:$PATH
	spv_remove_duplicate_path_entries
}

spv_set_toolchain_arm_common() {
	export ARCH=arm
	export CROSS_COMPILE=arm
	# Default license server/file for ARM tools
	# Use the global ARM license server -
	#export ARMLMD_LICENSE_FILE=7010@euhpc-lic04.euhpc.arm.com:7010@euhpc-lic05.euhpc.arm.com:7010@euhpc-lic07.euhpc.arm.com:7010@euhpc-lic03.euhpc.arm.com
	# Use local vivified license file -
	export ARMLMD_LICENSE_FILE=$SA_PV_TOP/out/arm_licenses/arm_licenses.lic
	# Supress errors for issues, such as "Waiting for license..."
	export ARMCC5_CCOPT=--diag_suppress=9933,C4017,C9931
	
	export CC=armcc
	export AR=armar
	export LD=armlink
}

spv_set_toolchain_arm_ds5(){
	spv_set_toolchain_arm_common

	export SA_PV_DS5_COMPILER_DIR=/usr/local/DS-5_v5.25.0/ARMCompiler5.06u3
	export PATH=$SA_PV_DS5_COMPILER_DIR/bin:$PATH
	export ARM_PATH=$SA_PV_DS5_COMPILER_DIR/bin
	export ARMCC_DIR=$SA_PV_DS5_COMPILER_DIR

	mbed config --global ARM_PATH $SA_PV_DS5_COMPILER_DIR
	spv_remove_duplicate_path_entries

}

spv_set_toolchain_arm_gcc_6() {
	export ARCH=arm
	unset CROSS_COMPILE

	# "ARMGCC_DIR" is being used by FreeRTOS build system
	export ARMGCC_DIR=/opt/arm/gcc6-arm-none-eabi

	export CC=$ARMGCC_DIR/bin/arm-none-eabi-gcc
	export OBJCOPY=$ARMGCC_DIR/bin/arm-none-eabi-objcopy
	export CXX=$ARMGCC_DIR/bin/arm-none-eabi-g++
	export AR=$ARMGCC_DIR/bin/arm-none-eabi-ar
	export LD=$ARMGCC_DIR/bin/arm-none-eabi-ld

	mbed config --global GCC_ARM_PATH $ARMGCC_DIR/bin
}

spv_set_toolchain_arm_gcc() {
	export ARCH=arm
	unset CROSS_COMPILE

	export ARMGCC_DIR=/usr
	export CC=arm-none-eabi-gcc
	export OBJCOPY=arm-none-eabi-objcopy
	export CXX=arm-none-eabi-g++
	export AR=arm-none-eabi-ar
	export LD=arm-none-eabi-ld
}

spv_mbed_import_config() {
	mbed config -G cache $SA_PV_TOP/../mbed-cache
	mbed config -G protocol ssh
}


case "$SA_PV_PLAT" in
PC )
	spv_mbed_import_config
	spv_set_toolchain_gcc

	set_pv_build () {
		echo Setting build type to: $1
		if [[ "$1" == "debug" ]]; then
			export DEBUG=1
			export TARGET_BUILD_TYPE=debug
		elif [[ "$1" == "release" ]]; then
			unset DEBUG
			export TARGET_BUILD_TYPE=release
		else
			echo "Unknown build type \"$1\" - Select \"debug\" or \"release\""
			return 1
		fi
		return 0
	}
	;;
K64F_FREERTOS_ARMCC )
	;&
K64F_MBEDOS_ARMCC )
	# MBED
	spv_set_toolchain_arm_ds5
	spv_mbed_import_config

	set_pv_build () {
		echo Setting build type to: $1
		if [[ "$1" == "debug" ]]; then
			export DEBUG=1
			export TARGET_BUILD_TYPE=debug
		elif [[ "$1" == "release" ]]; then
			unset DEBUG
			export TARGET_BUILD_TYPE=release
		else
			echo "Unknown build type \"$1\" - Select \"debug\" or \"release\""
			return 1
		fi
		return 0
	}
	;;
K64F_FREERTOS_ARM_GCC ) 
	spv_set_toolchain_arm_gcc_6
	spv_mbed_import_config

	set_pv_build () {
		echo Setting build type to: $1
		if [[ "$1" == "debug" ]]; then
			export DEBUG=1
			export TARGET_BUILD_TYPE=debug
		elif [[ "$1" == "release" ]]; then
			unset DEBUG
			export TARGET_BUILD_TYPE=release
		else
			echo "Unknown build type \"$1\" - Select \"debug\" or \"release\""
			return 1
		fi
		return 0
	}
	;;
K64F_MBEDOS_ARM_GCC ) 
	# MBED
	spv_set_toolchain_arm_gcc_6
	spv_mbed_import_config

	set_pv_build () {
		echo Setting build type to: $1
		if [[ "$1" == "debug" ]]; then
			export DEBUG=1
			export TARGET_BUILD_TYPE=debug
		elif [[ "$1" == "release" ]]; then
			unset DEBUG
			export TARGET_BUILD_TYPE=release
		else
			echo "Unknown build type \"$1\" - Select \"debug\" or \"release\""
			return 1
		fi
		return 0
	}
	;;
* )
	echo "Unknown platform: \"$SA_PV_PLAT\". Known platforms: $SA_PV_PLATS"
	return 1
	;;

esac

# Default to debug build type
set_pv_build debug

if [[ $QUICK == 0 ]] ; then
	# Deploy/ Updated repositories for new platform, unless in quick mode.
	echo "Deploying external repositories for $SA_PV_PLAT..."
	make deploy
else
	echo "Quick mode - skipping deploy/update of external repositories, as requested."
fi

return 0

