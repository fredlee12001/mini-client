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

all:

TESTS_DIR := $(SA_PV_TOP)/TESTS
GENERATE_TEST_DATA_DIR := $(TESTS_DIR)/generate-test-data
GENERATED_FILES_DIR := $(PV_OUT_GEN_DIR)/datafiles
DATA_OUT_DIR := $(PV_OUT_GEN_DIR)/testdata
TESTS_GENERATE_DATA_SCRIPT  := $(GENERATE_TEST_DATA_DIR)/generate_test_data_main.py

# Create various target directories.
$(GENERATED_FILES_DIR) $(DATA_OUT_DIR):
	@mkdir -p $@

# The two PvLib tests H and C files targets.
TESTS_TWO_TARGETS := $(DATA_OUT_DIR)/testdata.h $(DATA_OUT_DIR)/testdata.c

# Test-data H and C files need to be generated only if one of the input files has changed, the PRS has 
# changed or one of the scripts needed changed. 
TESTS_GEN_DEPS := \
	$(GENERATE_TEST_DATA_DIR) \
	$(GENERATED_FILES_DIR) \
	$(DATA_OUT_DIR)

# BEWARE! $(TESTS_TWO_TARGETS) is a list of two targets, a H and a C file!
$(TESTS_TWO_TARGETS): $(TESTS_GEN_DEPS)
	@python -u $(TESTS_GENERATE_DATA_SCRIPT) -cf=$(DATA_OUT_DIR)/testdata.c  -hf=$(DATA_OUT_DIR)/testdata.h -d=$(DATA_OUT_DIR) -f=$(GENERATED_FILES_DIR) -p=testdata_

# DO NOT combine this rule with the one calling servers_build_py - otherwise dependency checking will break
gen_testdata: $(TESTS_TWO_TARGETS)
