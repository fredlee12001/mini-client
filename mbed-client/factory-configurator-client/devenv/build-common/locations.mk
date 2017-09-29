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
# Global locations useful for various Makefiles.
# Must be used only after "pv_env_setup.sh" has been sourced.

# Platform-specific output directory (out/<platform>).
# Available in two versions - absolute (starts with "/" and relative).
PV_OUT_DIR_RELATIVE 	:= out/$(PV_PLAT_VECTOR)-$(TARGET_BUILD_TYPE)
PV_OUT_DIR          	:= $(SA_PV_TOP)/$(PV_OUT_DIR_RELATIVE)
PV_OUT_GEN_DIR			:= $(SA_PV_TOP)/out/gen
PV_OUT_UNIT_TESTS_DIR   := $(SA_PV_TOP)/$(PV_OUT_DIR_RELATIVE)/unit-tests
PV_OUT_APPS_DIR         := $(SA_PV_TOP)/$(PV_OUT_DIR_RELATIVE)/apps

LINUX_NATIVE_BUILD_DIR	:= $(SA_PV_TOP)/__x86_x64_NativeLinux_mbedtls
FREERTOS_BUILD_DIR      := $(SA_PV_TOP)/__K64F_FreeRTOS_mbedtls