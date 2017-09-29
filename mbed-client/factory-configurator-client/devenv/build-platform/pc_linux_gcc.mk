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
# Makefile for PC/Linux development environment

include $(SA_PV_TOP)/devenv/build-common/locations.mk

# Keep defines before `k64f_common.mk`
BUILD_TOOLCHAIN := GCC
BUILD_DIR       := $(LINUX_NATIVE_BUILD_DIR)
BUILD_OS_BRAND  := linux
INSTALL_LICENSE :=

# Common makefile for FRDM-K64F mbed armcc and mbed arm-none-eabi-gcc platforms
include $(SA_PV_TOP)/devenv/build-platform/k64f_common.mk

include $(SA_PV_TOP)/devenv/build-common/valgrind.mk

check: cbor_tests fcc_component_tests
	${valg_cmd} $(PV_OUT_UNIT_TESTS_DIR)/cbor_tests.elf
	${valg_cmd} $(PV_OUT_UNIT_TESTS_DIR)/fcc_component_tests.elf
	@echo check component tests completed.
