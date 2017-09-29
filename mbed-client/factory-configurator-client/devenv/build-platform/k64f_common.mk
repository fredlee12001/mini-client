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
include $(SA_PV_TOP)/devenv/build-common/gen.mk
include $(SA_PV_TOP)/devenv/build-common/armcc_licenses.mk
include $(SA_PV_TOP)/devenv/build-common/warnings_as_errors.mk


########################### Assert ###########################

ifeq ($(BUILD_TOOLCHAIN),)
	$(error Please set BUILD_TOOLCHAIN)
endif

ifeq ($(BUILD_DIR),)
	$(error Please set BUILD_DIR)
endif

ifeq ($(BUILD_OS_BRAND),)
	$(error Please set BUILD_OS_BRAND)
endif

########################### Helpers ###########################

# leave empty for 'debug'
BUILD_TYPE_GENERATE_ARG :=

# default is 'Debug' (note the initial cap 'D')
BUILD_TYPE ?= Debug

ifeq ($(TARGET_BUILD_TYPE), release)
	BUILD_TYPE_GENERATE_ARG := --release
	BUILD_TYPE := Release
endif

BUILD_TOOLCHAIN_ARG :=
ifneq ($(BUILD_OS_BRAND), linux)
	BUILD_TOOLCHAIN_ARG := --toolchain=$(BUILD_TOOLCHAIN)
endif

########################### Targets ###########################

.PHONY: deploy
deploy:
	@python $(SA_PV_TOP)/env_setup.py -v $(BUILD_OS_BRAND)-deploy

generate:
	@python $(SA_PV_TOP)/env_setup.py -v \
					$(BUILD_OS_BRAND)-generate \
					$(BUILD_TYPE_GENERATE_ARG) \
					$(BUILD_TOOLCHAIN_ARG)

install_license: import_arm_licenses

all: \
	$(INSTALL_LICENSE) \
	gen_testdata \
	generate \
	tests

clean:
	@echo [CLN] $(BUILD_DIR)
	@rm -rf $(BUILD_DIR)

tests: cbor_tests fcc_component_tests

########################### Artifacts ###########################

fcc_component_tests:
	@# 2>&1 redirects stderr into stdout and then we pipe to warning script
	@cd $(SA_PV_TOP) && make -C $(BUILD_DIR) $@.elf 2>&1 | $(WARNINGS_AS_ERRORS)
	@cp $(BUILD_DIR)/$(BUILD_TYPE)/$@.* $(PV_OUT_UNIT_TESTS_DIR)/

cbor_tests:
	@# 2>&1 redirects stderr into stdout and then we pipe to warning script
	@cd $(SA_PV_TOP) && make -C $(BUILD_DIR) $@.elf 2>&1 | $(WARNINGS_AS_ERRORS)
	@cp $(BUILD_DIR)/$(BUILD_TYPE)/$@.* $(PV_OUT_UNIT_TESTS_DIR)/
