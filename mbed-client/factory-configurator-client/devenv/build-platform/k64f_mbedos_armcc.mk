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
# Makefile for FRDM-K64F mbed armcc platform
# The target is a single bare-metal image (.axf --> .bin)

######################################################
### Note - the order of *.mk includes is mandatory ###
######################################################

include $(SA_PV_TOP)/devenv/build-common/armcc_licenses.mk

BUILD_TOOLCHAIN   := ARM
TOOLCHAIN_DEFINES :=
INSTALL_LICENSE   := install_license

# Common makefile for FRDM-K64F mbed armcc and mbed arm-none-eabi-gcc platforms
include $(SA_PV_TOP)/devenv/build-platform/k64f_mbedos_common.mk
