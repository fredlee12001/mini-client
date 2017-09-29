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


BUILD_LOG_FILE := $(PV_OUT_DIR)/build_log_for_warning_analysis_$(MAKECMDGOALS).txt

SOME_RAND := $(eval, $$RANDOM)
RANDOM := $(shell bash -c 'echo $(pid)')
A_RAND := 
#BUILD_LOG_FILE := $(SA_PV_TOP)/out/build_log_for_warning_analysis_$(RANDOM).txt

WARNING_PATTERN_LIST := $(SA_PV_TOP)/devenv/build-common/dirs_for_warnings.txt


# WARNINGS_AS_ERRORS must be placed right after the final compilation flag
# Caller must pipe stout and stderr to WARNINGS_AS_ERRORS.
# tee command writes from the pipe to stdout as well as to $(BUILD_LOG_FILE).
# If the piped compilation failes -> tee fails -> echo error message and exit with status 80.
# If compilation passes -> tee passes -> greps a list of warning patterns. if the pattern is found -> show warnings and echo error message. If pattern not found -> echo success message.

# Note: in order for this to work properly, tee must return an error if the previous command fails. So somewhere in the makefile, before WARNINGS_AS_ERRORS is used, the following line must be called:
SHELL=/bin/bash -o pipefail

# Note: when building with mbed cli, building a binary a second time using 'make' command does not recompile files that have not been changed.
# Therefore, running make a second time will not display warnings.
# In order to display rhem run 'make rebuild', 'make clean', or add -c flag to the compilation. 

WARNINGS_AS_ERRORS := tee $(BUILD_LOG_FILE); \
							if [ $$? = 0 ]; then \
								grep --color=always -f $(WARNING_PATTERN_LIST) $(BUILD_LOG_FILE) && \
								echo -e "\e[1;31m---------- factory-configurator-client build status: FAIL (Warnings) ----------\e[0m" && \
								exit 81 ; \
								echo -e "\e[1;32m ---------- factory-configurator-client build status: SUCCESS ----------\e[0m"  ; \
							else \
								echo -e "\e[1;31m---------- factory-configurator-client build status: FAIL (Errors) ----------\e[0m" && \
								exit 80; \
							fi
