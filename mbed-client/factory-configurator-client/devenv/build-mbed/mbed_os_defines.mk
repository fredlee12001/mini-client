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
# Common mbed-os defines 

MBEDTLS_DEFINES := \
	-DMBEDTLS_CMAC_C \
	-DMBEDTLS_PEM_WRITE_C \
	-DMBEDTLS_X509_CSR_WRITE_C \
	-DMBEDTLS_X509_CREATE_C \
	-DMBEDTLS_CIPHER_MODE_CTR \
	-DMBEDTLS_CTR_DRBG_MAX_REQUEST=2048

STORAGE_DEFINES := \
	-DPAL_USE_FATFS_SD=1

MBED_OS_DEFINES := \
	$(MBEDTLS_DEFINES) \
	$(STORAGE_DEFINES)