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
# A Jenkins job exists that creates a vivified license file for many ARM tools, that is valid
# for 10 days.
# See devenv/arm_licenses_vivification for details.
#
# Here we take this created file and put it into the tree, such that it is directly used,
# instead of using a (slow) global license server.

include $(SA_PV_TOP)/devenv/build-common/locations.mk


ARM_LICENSES_FILE_NAME := arm_licenses.lic

ARM_LICENSES_SOURCE_URL := http://prov-jen-master.kfn.arm.com:8888/job/vivify_arm_license/lastSuccessfulBuild/artifact/arm_licenses_vivified.lic

ARM_LICENSES_TARGET_DIR := $(SA_PV_TOP)/out/arm_licenses
ARM_LICENSES_TARGET_PATH := $(ARM_LICENSES_TARGET_DIR)/$(ARM_LICENSES_FILE_NAME)

import_arm_licenses:
	mkdir -p $(ARM_LICENSES_TARGET_DIR)
	curl $(ARM_LICENSES_SOURCE_URL) > $(ARM_LICENSES_TARGET_PATH)
	echo ARMLICENSE=$(ARMLMD_LICENSE_FILE)
