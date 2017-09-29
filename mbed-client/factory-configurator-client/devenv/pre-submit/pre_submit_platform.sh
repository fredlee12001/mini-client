#!/bin/bash -l
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
# -l is used in order to make sure ~/.bashrc would be sourced for alises and functions.

cd `dirname $0`
source ./pre_submit_common.sh

if [[ -z $1 ]]; then
	echo Platform not specified.
	exit 1
fi
PLAT=$1

if [[ -z $2 ]]; then
	echo Build not specified.
	exit 1
fi
BUILD=$2

if [[ -z $3 ]]; then
	echo Test mode not specified.
	exit 1
fi
TESTS=$3

if [[ -z $4 ]]; then
	echo Valgrind mode not specified.
	exit 1
fi
VALGRIND=$4

if [[ -z $5 ]]; then
	echo Extra is not specified.
	exit 1
fi
EXTRA=$5

if [[ -z $6 ]]; then
	echo Code Coverage is not specified.
	exit 1
fi
CODE_COVERAGE_PARAM=$6

JENKINS_BUILD_NUMBER=""
if [[ -n ${7} ]]; then
	JENKINS_BUILD_NUMBER=${7}
fi	


# SPV top is two directories above this script dir. Can't use $SA_PV_TOP
# because it doesn't exist yet.
cd ../..
start=`profile_start`
# be called anyway.
source devenv/pv_env_setup.sh $PLAT -q  

profile_end $start "pv_env_setup"

start=`profile_start`
set_pv_build $BUILD
profile_end $start "$build set_pv_build"

start=`profile_start`
make clean
check_pre_submit_error "'make clean' on $PLAT for $BUILD FAILED, check what you broke (or clean manually...)"
profile_end $start "$build clean"

start=`profile_start`
make rebuild SA_PV_BUILD_NUMBER=$JENKINS_BUILD_NUMBER
check_pre_submit_error "'make rebuild' on $PLAT for $BUILD FAILED, check what you broke..."
profile_end $start "$build rebuild"

if [[ $CODE_COVERAGE_PARAM == "yes" ]] ; then
	CODE_COVERAGE_TEST="1"
	COVERAGE_CODE_PATH=out/pc_linux_linux-debug
	#add gcov compilation flags to qa_slave makefile
	sed '/CXXFLAGS +=/ a CXXFLAGS += -fprofile-arcs -ftest-coverage' ./out/pc_linux_linux-debug/import/qa_slave/devenv/pc_linux_linux.mk > ./out/pc_linux_linux-debug/import/qa_slave/devenv/pc_linux_linux.mk_temp
	mv ./out/pc_linux_linux-debug/import/qa_slave/devenv/pc_linux_linux.mk_temp ./out/pc_linux_linux-debug/import/qa_slave/devenv/pc_linux_linux.mk
elif [[ $CODE_COVERAGE_PARAM == "no" ]] ; then
	CODE_COVERAGE_TEST="0"
fi

if [[ $PLAT == "PC" && $TESTS == "yes" ]] ; then
	start=`profile_start`
	if [[ $VALGRIND == "yes" ]] ; then
		make valgrind
	else
		make check
	fi
	check_pre_submit_error "Unity tests for PC FAILED, check what you broke..."
	profile_end $start "make check"
fi

if [[ $CODE_COVERAGE_PARAM == "yes" ]] ; then
	start=`profile_start`
	lcov  --capture --directory $COVERAGE_CODE_PATH -b ./  --output-file $COVERAGE_CODE_PATH/coverage.info
	genhtml $COVERAGE_CODE_PATH/coverage.info --output-directory $COVERAGE_CODE_PATH/coverage_html
	profile_end $start "Generate code coverage data"
	
	profile_print_times "CODE_COVERAGE"
	
	toilet -f term  Code coverage data created
	tput setaf 6				 
	toilet -f term -F border  -F crop You can see code coverage data in $COVERAGE_CODE_PATH/coverage_html folder
	tput sgr0
else
	profile_print_times $PLAT

fi

exit 0

