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
# Makefile with rules for softcrys builds
# To be included from within externals.mk

SOFTCRYS_IMPORT_DIR := $(PV_IMPORT_DIR)/softcrys

# Modules required for SPV
SOFTCRYS_ALGS := common aes aesccm hash hmac rnd kdf ecpki pki dx_ecpki_domains

# Path of Softcrys sources within crypto_fw source tree
SOFTCRYS_PATH := host/src/softcrys
TZINFRA_SOFTCRYS_PATH := TZInfra/Crys/Crys_External/host/src/softcrys

# Include algorithms definitions as given in .mk of each algorithm
-include $(foreach alg,$(SOFTCRYS_ALGS),$(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/$(alg)/$(alg)_defs.mk)
SOFTCRYS_SRCS_DIRS := $(addprefix $(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/,$(SOFTCRYS_ALGS))
SOFTCRYS_API_INC := $(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/softcrys_api
SOFTCRYS_SRCS := $(sort $(foreach alg,$(SOFTCRYS_ALGS),$(addprefix  $(SOFTCRYS_PATH)/$(alg)/,$(SOFTCRYS_SRC_$(alg)))))

# Add the import path to the sources list -
SOFTCRYS_SRCS := $(addprefix $(SOFTCRYS_IMPORT_DIR)/,$(SOFTCRYS_SRCS))

# Default to LITTLE_ENDIAN
SOFTCRYS_DEFINES := LITTLE__ENDIAN
SOFTCRYS_DEFINES_WITH_D := $(addprefix -D,$(SOFTCRYS_DEFINES))

# Modules required for SPV
SOFTCRYS_ALGS_4_LIB_TEE_TA := common hash rnd ecpki pki dx_ecpki_domains

# Include algorithms definitions as given in .mk of each algorithm
-include $(foreach alg,$(SOFTCRYS_ALGS_4_LIB_TEE_TA),$(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/$(alg)/$(alg)_defs.mk)
SOFTCRYS_SRCS_DIRS_4_LIB_TEE_TA := $(addprefix $(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/,$(SOFTCRYS_ALGS_4_LIB_TEE_TA))
#SOFTCRYS_API_INC := $(SOFTCRYS_IMPORT_DIR)/$(SOFTCRYS_PATH)/softcrys_api
SOFTCRYS_SRCS_4_LIB_TEE_TA := $(sort $(foreach alg,$(SOFTCRYS_ALGS_4_LIB_TEE_TA),$(addprefix  $(SOFTCRYS_PATH)/$(alg)/,$(SOFTCRYS_SRC_$(alg)))))

# Add the import path to the sources list -
SOFTCRYS_SRCS_4_LIB_TEE_TA := $(addprefix $(SOFTCRYS_IMPORT_DIR)/,$(SOFTCRYS_SRCS_4_LIB_TEE_TA))
