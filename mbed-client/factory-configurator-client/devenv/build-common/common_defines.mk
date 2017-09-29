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
# Common defines 

COMMON_DEFINES := \
	-DSA_PV_OS_$(SA_PV_OS) \
	-DSA_PV_TOOLCHAIN_$(SA_PV_TOOLCHAIN) \
	-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_INFO \
	-DPAL_NUMBER_OF_PARTITIONS=1 -DPAL_PRIMARY_PARTITION_PRIVATE=0 -DPAL_SECONDARY_PARTITION_PRIVATE=0 #multiple partitions defenition. Configured to work with 1 partition only
ifeq ($(DEBUG),1)
	COMMON_DEFINES += -DDEBUG
endif
