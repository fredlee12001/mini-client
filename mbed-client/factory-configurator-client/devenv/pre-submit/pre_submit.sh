#!/bin/bash
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

# TODO: This script (and its sub-scripts) became too complicated for Bash. Can be re-written in Python.

source devenv/pre-submit/pre_submit_common.sh

usage()
{
cat << EOF
Usage:
  Profile selection -
    -f, --profile               Compilation/test profile. Can be one of -
                                all - all platforms compilation, PC compilation
                                      and testing with Valgrind.
                                pre_submit - important platforms, release build only,
				                             with Valgrind - useful before submitting code
                                post_submit - like "all", with an extra step to make sure server
				                              compilations work in two different configurations -
						                      useful after submitting code, by a CI tool
                                code_coverage - Generates device code coverage, compiles on PC platform only,
				                                run PC tests and creates code coverage HTML report file
  Modifiers to selected profile -
    -p, --platform              Platform. Can be one of -
				  "all" - All platforms.
				Default is "all".
    -t, --tests                 Run tests. Can be one of - "yes", "no".
                                Default is "yes".
    -b, --build                 Build type. Can be one of - "debug", "release" or "all".
                                Default is "all".
    -v, --valgrind              Run tests with Valgrind enabled. Can be one off "yes", "no" or "all".
                                Default is "yes".
    -e, --extra                 Perform extra steps for a fuller coverage. Can be one of "yes" or "no".
                                Default is "no".
  Additional options -
    -r, --rsync                 Use rsync to /opt/scratch to speed up compilation. 
                                Can be one of "yes" or "no".
    -j, --jenkins-build-number  Jenkings build number to embed into binaries.
EOF
}


# Defaults for command line options and profiles.
PROFILE=""
PLAT="all"
TESTS="yes"
BUILD="all"
VALGRIND="yes"
EXTRA="no"
RSYNC="yes"
CODE_COVERAGE="no"
JENKINS_BUILD_NUMBER=""


# Parse command-line options using getopt.
# See /usr/share/doc/util-linux/examples/getopt-parse.bash for a good example.
GETOPT_OUTPUT=`getopt -o "hf:p:t:b:v:e:r:k:j:" --long "help,profile:,platform:,tests:,build:,extra:,valgrind:,rsync:,jenkins-build-number:" -- "$@"`
if [ $? -gt 0 ] ; then
	# Errors detected by getopt. Exit.
	usage
	exit 1
fi

# Re-parse output of getopt, to properly cope with white-space and quoted strings.
# See getopt manpage for details.
eval set -- "$GETOPT_OUTPUT"

while true ; do
	case "$1" in
		-h|--help)
			usage
			exit 1
			;;
		-f|--profile)
			PROFILE=$2
			shift 2
			;;
		-p|--platform)
			PLAT_OVERRIDE=$2
			shift 2
			;;
		-t|--tests)
		        TESTS_OVERRIDE=$2
			shift 2
			;;
		-b|--build)
			BUILD_OVERRIDE=$2
			shift 2
			;;
		-v|--valgrind)
			VALGRIND_OVERRIDE=$2
			shift 2
			;;
		-e|--extra)
			EXTRA_OVERRIDE=$2
			shift 2
			;;
		-r|--rsync)
			RSYNC=$2
			shift 2
			;;
		-c|--code-coverage)
			CODE_COVERAGE=$2
			shift 2
			;;
		-j|--jenkins-build-number)
			JENKINS_BUILD_NUMBER=$2
			shift 2
			;;
		--)
			shift
			break
			;;
		*)
			echo "Command line parsing internal error!"
			exit 1
			;;
	esac
done


# If a profile was selected, set defaults for it.
case "$PROFILE" in
	# "All" profile
	# 1. Compile Device on all platforms
	# 2. Run all tests with VALGRIND on PC
	"all")
		PLAT="all"
		TESTS="yes"
		BUILD="all"
		VALGRIND="yes"
		EXTRA="no"
		;;
		
	# "Pre-Submit" profile: quick run, with fairly good coverage
	"pre_submit")
		PLAT="all"
		TESTS="yes"
		BUILD="release"
		VALGRIND="yes"
		EXTRA="no"
		;;

	# "Post-Submit" profile: lengthly, full coverage.
	"post_submit")
		PLAT="all"
		TESTS="yes"
		BUILD="all"
		VALGRIND="all"
		EXTRA="yes"
		;;

	# "Device Code Coverage" profile. Generate device code coverage
	# 1. PC platform compilation
	# 2. All PC tests without VALGRIND
	# 3. Create Code Coverage HTML report
	"code_coverage")
		PLAT="PC"
		TESTS="yes"
		BUILD="debug"
		VALGRIND="no"
		EXTRA="no"
		CODE_COVERAGE="yes"
		;;
		
        # No profile - skip.
	"")
		;;

	*)
		echo "Unrecognized profile selected!"
		usage
		exit 1
		;;
esac


# If a command line option exists, override the defaults -

if [[ -n $PLAT_OVERRIDE ]] ; then
    PLAT=$PLAT_OVERRIDE
fi

if [[ -n $TESTS_OVERRIDE ]] ; then
    TESTS=$TESTS_OVERRIDE
fi

if [[ -n $BUILD_OVERRIDE ]] ; then
    BUILD=$BUILD_OVERRIDE
fi

if [[ -n $VALGRIND_OVERRIDE ]] ; then
    VALGRIND=$VALGRIND_OVERRIDE
fi

if [[ -n $EXTRA_OVERRIDE ]] ; then
    EXTRA=$EXTRA_OVERRIDE
fi


# Check correctness of all options -

if [[ ( $TESTS != "yes" ) && ( $TESTS != "no" ) ]] ; then
	echo Tests must be \"yes\" or \"no\".
	exit 1
fi

if [[ ( $BUILD != "debug" ) && ( $BUILD != "release" ) && ( $BUILD != "all" ) ]] ; then
	echo Build must be \"debug\", \"release\" or \"all\".
	exit 1
fi

if [[ ( $VALGRIND != "yes" ) && ( $VALGRIND != "no" ) && ( $VALGRIND != "all" ) ]] ; then
	echo Valgrind must be \"yes\", \"no\" or \"all\".
	exit 1
fi

if [[ ( $EXTRA != "yes" ) && ( $EXTRA != "no" ) ]] ; then
	echo Extra must be \"yes\" or \"no\".
	exit 1
fi

if [[ ( $RSYNC != "yes" ) && ( $RSYNC != "no" ) ]] ; then
	echo Rsync must be \"yes\" or \"no\".
	exit 1
fi

if [[ ( $CODE_COVERAGE != "yes" ) && ( $CODE_COVERAGE != "no" ) ]] ; then
	echo Code Coverage must be \"yes\" or \"no\".
	exit 1
fi

if [[ $PLAT == "all" ]] ; then
	PLATS="PC K64F_MBEDOS_ARMCC K64F_MBEDOS_ARM_GCC K64F_FREERTOS_ARMCC K64F_FREERTOS_ARM_GCC"
else
	PLATS=$PLAT
fi

if [[ $BUILD == "all" ]] ; then
    BUILDS="debug release"
else
    BUILDS=$BUILD
fi


echo "***************************************"
echo Running pre_submit with -
echo \* Profile = \"$PROFILE\"
echo \* Platforms = \"$PLATS\"
echo \* Tests = \"$TESTS\"
echo \* Build = \"$BUILDS\"
echo \* Valgrind = \"$VALGRIND\"
echo \* Extra = \"$EXTRA\"
echo \* Rsync = \"$RSYNC\"
echo \* Code coverage = \"$CODE_COVERAGE\"
echo \* Jenkins build number = \"$JENKINS_BUILD_NUMBER\"
echo "***************************************"


# Build steps start here -

check_exit() {
	if [ $? -ne 0 ] 
	then
		popd > /dev/null
		exit 1
	fi
}

if [[ $RSYNC == "yes" ]] ; then
	# Before running pre_submit, copy the current tree to scratch location.
	# Add to the scratch location name a postfix, when running from cron job, to
	# avoid colliding with existing runs.

	SCRATCH_PATH=/opt/scratch/$USER/spv-pre-submit$PV_SCRATCH_ADDITION
	mkdir -p $SCRATCH_PATH

	start=`profile_start`	
	echo Copying tree to $SCRATCH_PATH...
	rsync -a --stats --delete \
		--exclude 'out' \
		--exclude '__K64F_FreeRTOS' \
		--exclude '__x86_x64_Linux_Native' \
		--exclude 'mbed-client-esfs' \
		--exclude 'mbed-os' \
		--exclude 'sd-driver' \
		--exclude 'mbed-client-pal' \
		--exclude 'mbed-trace' \
		--exclude 'pal-platform' \
		--exclude 'nanostack-libservice' \
		--exclude 'secsrv-cbor' \
		--exclude 'unity' \
		. $SCRATCH_PATH
	
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	profile_end $start "rsync to scratch"
	
	# Change to the pre_submit script directory, in the rsync copy.
	pushd $SCRATCH_PATH > /dev/null
else
	# Change to the pre_submit script directory (locally).
	pushd . > /dev/null
fi

start=`profile_start`
source devenv/pv_env_setup.sh -q PC
profile_end $start "pv_env_setup PC"

for plat in $PLATS; do
	for build in $BUILDS; do
		start=`profile_start`
		echo  /bin/bash devenv/pre-submit/pre_submit_platform.sh $plat $build $TESTS $VALGRIND $EXTRA $CODE_COVERAGE $JENKINS_BUILD_NUMBER 
		/bin/bash devenv/pre-submit/pre_submit_platform.sh $plat $build $TESTS $VALGRIND $EXTRA $CODE_COVERAGE $JENKINS_BUILD_NUMBER 
		check_exit
		profile_end $start "$PROFILE profile, $plat platform, build $build"
	done
done

start=`profile_start`
echo  "Run Doxygen builder"
make doxygen
check_exit
profile_end $start "Doxygen builder"

if [[ $CODE_COVERAGE == "no" ]] ; then
	toilet -f bigmono12 --gay SUCCESS!
	echo "=========================================="
	echo "  All steps successful - you may submit!  "
	echo "  Thanks for running pre_submit.          "
	echo "=========================================="
fi

echo

profile_print_times 'Overall pre_submit'

popd > /dev/null

exit 0

