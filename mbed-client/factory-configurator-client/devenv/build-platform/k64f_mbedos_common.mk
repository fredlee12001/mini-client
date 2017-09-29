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

include $(SA_PV_TOP)/devenv/build-common/locations.mk

# Fetch common Unity defines into $(UNITY_COMMON_DEFINES).
include $(SA_PV_TOP)/devenv/build-common/unity_common_defines.mk

# Fetch common mbed-os defines into $(MBED_OS_DEFINES).
include $(SA_PV_TOP)/devenv/build-mbed/mbed_os_defines.mk

# Fetch common mbed-os defines into $(COMMON_DEFINES).
include $(SA_PV_TOP)/devenv/build-common/common_defines.mk

include $(SA_PV_TOP)/devenv/build-common/gen.mk

# Fetch script to treat compilation warnings as errors only for our desired directories
include $(SA_PV_TOP)/devenv/build-common/warnings_as_errors.mk


MBED_OS_IMPORT_DIR := $(SA_PV_TOP)/mbed-os
BUILD_PLATFORM := K64F


########################### Targets ###########################

.PHONY: deploy
deploy:
	@python $(SA_PV_TOP)/env_setup.py mbed-os-deploy

generate:
	@echo nothing really to generate for mbed-os...

install_license: import_arm_licenses

all: \
	$(INSTALL_LICENSE) \
	mbed_os_patch \
	tests 

tests: cbor_tests fcc_component_tests

clean: clean_cbor_tests clean_fcc_component_tests 

mbed_os_patch:
	@echo [PATCH] $(MBED_OS_IMPORT_DIR)
	@echo "*" > $(PV_OUT_DIR)/.mbedignore
	@#FIXME: should be uncomment when mbed-os compilation issue will be resolved, to treat all warnings as errors
	@#@sed -i 's/-Wextra/-Wextra,-Werror/g' $(MBED_OS_IMPORT_DIR)/core/tools/toolchains/gcc.py
	@sed -i 's/"-Otime"/"-Otime","-Ospace"/g' $(MBED_OS_IMPORT_DIR)/tools/toolchains/arm.py
	@# ignore mbed-os unity to avoid ambiguity of SPV unity testing framework
	@echo "*" > $(MBED_OS_IMPORT_DIR)/features/frameworks/.mbedignore
	@# ignore mbed-trace of mbed-os as we import trace repository to fc
	@echo "*" > $(MBED_OS_IMPORT_DIR)/features/FEATURE_COMMON_PAL/mbed-trace/.mbedignore
	@# ignore nanostack as we import it only for linux
	@if [ -d "$(SA_PV_TOP)/nanostack-libservice" ]; then echo "*" > $(SA_PV_TOP)/nanostack-libservice/.mbedignore; fi


	@# other OS's repos
	@if [ -d "$(SA_PV_TOP)/pal-platform" ]; then echo "*" > $(SA_PV_TOP)/pal-platform/.mbedignore; fi
	@if [ -d "$(LINUX_NATIVE_BUILD_DIR)" ]; then echo "*" > $(LINUX_NATIVE_BUILD_DIR)/.mbedignore; fi
	@if [ -d "$(FREERTOS_BUILD_DIR)" ]; then echo "*" > $(FREERTOS_BUILD_DIR)/.mbedignore; fi

########################### Defines / Cflags ###########################

BUILD_DEFINES := \
	$(TOOLCHAIN_DEFINES) \
	$(MBED_OS_DEFINES) \
	$(UNITY_COMMON_DEFINES) \
	$(COMMON_DEFINES) \
	-DPV_REPRODUCIBLE_RANDOM_FUNCTION \
	-DPV_TEST_ONLY_APIS \
	-DPAL_TEST_MAIN_THREAD_STACK_SIZE=7168

BUILD_COMMON_SRCS := \
	mbed-client-pal/Examples/PlatformBSP/Include \
	mbed-client-pal/Examples/PlatformBSP/MK64F_mbedOS \
	mbed-client-pal/Test/Unitest/TestRunner/pal_testMbedOS.cpp \
	mbed-client-pal/Test/Unitest/Includes

FCC_MBED_APP_JSON := $(SA_PV_TOP)/devenv/build-mbed/config_for_unit_tests.json

# set mbed-os CFLAGS profile
BUILD_CFLAGS := --profile $(MBED_OS_IMPORT_DIR)/tools/profiles/release.json
ifeq ($(DEBUG),1)	
	BUILD_CFLAGS := --profile $(MBED_OS_IMPORT_DIR)/tools/profiles/debug.json
endif


########################### Artifacts ###########################

TESTS_BUILD_DIR := $(PV_OUT_DIR)/build

#### FCC component tests

FCC_BIN_NAME := fcc-component-runner
FCC_TARGET_BIN_PATH := $(TESTS_BUILD_DIR)/TESTS/runner/$(FCC_BIN_NAME)/$(FCC_BIN_NAME).bin

FCC_CMP_TEST_SRCS := \
	$(SA_PV_TOP)/TESTS/tests \
	$(BUILD_COMMON_SRCS)

fcc_component_tests: gen_testdata
	@mbed test \
	-t $(BUILD_TOOLCHAIN) \
	-m $(BUILD_PLATFORM) \
	-n tests-runner-$(FCC_BIN_NAME) \
	--compile -v \
	--source . \
	$(addprefix --source , $(FCC_CMP_TEST_SRCS)) \
	--app-config $(FCC_MBED_APP_JSON) \
	--build $(TESTS_BUILD_DIR) \
	$(BUILD_DEFINES) $(BUILD_CFLAGS) \
	2>&1 | $(WARNINGS_AS_ERRORS) # 2>&1 redirects stderr into stdout and then we pipe to warning script
	@echo [CP] $(FCC_TARGET_BIN_PATH) 
	@cp $(FCC_TARGET_BIN_PATH) $(PV_OUT_UNIT_TESTS_DIR)

clean_fcc_component_tests:
	@echo [CLN] $@
	@rm -rf $(TESTS_BUILD_DIR)
	@rm -rf $(PV_OUT_UNIT_TESTS_DIR)/$(TARGET_BIN_NAME).bin
	


#### CBOR tests

CBOR_BIN_NAME := cbor_main
CBOR_BIN_PATH := $(TESTS_BUILD_DIR)/secsrv-cbor/TESTS/runner/$(CBOR_BIN_NAME)/$(CBOR_BIN_NAME).bin

CBOR_TEST_SRCS := \
	$(SA_PV_TOP)/secsrv-cbor/TESTS/runner/cbor_runner \
	$(SA_PV_TOP)/secsrv-cbor/TESTS/tests \
	$(SA_PV_TOP)/TESTS/tests/mbed_cloud_dev_credentials.c \
	$(BUILD_COMMON_SRCS)

cbor_tests: gen_testdata
	@mbed test \
	-t $(BUILD_TOOLCHAIN) \
	-m $(BUILD_PLATFORM) \
	-n secsrv-cbor-tests-runner-cbor_main \
	--compile -v \
	--source . \
	$(addprefix --source , $(CBOR_TEST_SRCS)) \
	--app-config $(FCC_MBED_APP_JSON) \
	--build $(TESTS_BUILD_DIR) \
	$(BUILD_DEFINES) $(BUILD_CFLAGS) \
	2>&1 | $(WARNINGS_AS_ERRORS) # 2>&1 redirects stderr into stdout and then we pipe to warning script
	@echo [CP] $(CBOR_BIN_PATH) 
	@cp $(CBOR_BIN_PATH) $(PV_OUT_UNIT_TESTS_DIR)

clean_cbor_tests:
	@echo [CLN] $@
	@rm -rf $(TESTS_BUILD_DIR)
	@rm -rf $(PV_OUT_UNIT_TESTS_DIR)/$(CBOR_BIN_NAME).bin