// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#ifndef __MBED_CLOUD_DEV_CREDENTIALS_H__
#define __MBED_CLOUD_DEV_CREDENTIALS_H__

#include <inttypes.h>

const char  MBED_CLOUD_DEV_BOOTSTRAP_ENDPOINT_NAME[] = "dd789127-c8f9-41c8-b06b-e655dde1bb2b";
const char  MBED_CLOUD_DEV_BOOTSTRAP_SERVER_URI[] = "coaps://bootstrap.connector.mbed.com:5684?aid=de98c7883408488cbffac482a837a73a";

const uint8_t MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_CERTIFICATE[] = {
    0x30, 0x82, 0x02, 0x0c, 0x30, 0x82, 0x01, 0xb2, 0x02, 0x01, 0x03, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x81, 0x82, 0x31, 0x0b, 0x30, 0x09,
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x4b, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x0a, 0x4b, 0x66, 0x61, 0x72, 0x2d, 0x6e, 0x65, 0x74, 0x65, 0x72,
    0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x4e, 0x65, 0x74, 0x61, 0x6e, 0x69, 0x61, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03, 0x41,
    0x52, 0x4d, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x03, 0x49, 0x4f, 0x54, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x06, 0x44, 0x65, 0x76,
    0x69, 0x63, 0x65, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x10, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x31, 0x40, 0x61,
    0x72, 0x6d, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x37, 0x30, 0x36, 0x32, 0x32, 0x30, 0x38, 0x30, 0x39, 0x31, 0x30, 0x5a, 0x17, 0x0d, 0x33, 0x33, 0x31, 0x31, 0x32,
    0x35, 0x30, 0x38, 0x30, 0x39, 0x31, 0x30, 0x5a, 0x30, 0x81, 0xa0, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x4b, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
    0x55, 0x04, 0x08, 0x0c, 0x0a, 0x4b, 0x66, 0x61, 0x72, 0x2d, 0x6e, 0x65, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x4e, 0x65, 0x74, 0x61,
    0x6e, 0x69, 0x61, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03, 0x41, 0x52, 0x4d, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x03, 0x49, 0x4f,
    0x54, 0x31, 0x2d, 0x30, 0x2b, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x24, 0x64, 0x64, 0x37, 0x38, 0x39, 0x31, 0x32, 0x37, 0x2d, 0x63, 0x38, 0x66, 0x39, 0x2d, 0x34, 0x31, 0x63, 0x38,
    0x2d, 0x62, 0x30, 0x36, 0x62, 0x2d, 0x65, 0x36, 0x35, 0x35, 0x64, 0x64, 0x65, 0x31, 0x62, 0x62, 0x32, 0x62, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
    0x01, 0x09, 0x01, 0x16, 0x10, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x31, 0x40, 0x61, 0x72, 0x6d, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x93, 0x77, 0x00, 0x8c, 0x9b, 0x03, 0x34, 0xa0, 0x19, 0xc8, 0xa7, 0xed,
    0x96, 0x81, 0xd3, 0x7c, 0xd5, 0x54, 0x84, 0x81, 0x31, 0x1d, 0xe9, 0x66, 0x6f, 0xd6, 0xd4, 0x71, 0x43, 0x81, 0xd2, 0x92, 0x3f, 0x3f, 0xd8, 0x66, 0x35, 0x2b, 0x09, 0xe0, 0xf0, 0xc2,
    0xc6, 0x1f, 0x24, 0xb8, 0x4a, 0x89, 0xdd, 0x7e, 0xf9, 0x25, 0x0d, 0x13, 0x43, 0x59, 0x21, 0xdf, 0xda, 0xa0, 0xf9, 0xdd, 0x45, 0xc0, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0xe1, 0x58, 0x0b, 0x90, 0x0b, 0xde, 0x37, 0x1e, 0xd3, 0xa9, 0x27, 0x00, 0xf1, 0xa4, 0x17, 0x2d, 0x54, 0x9d,
    0xfb, 0x58, 0x7a, 0x55, 0x44, 0xb3, 0xa5, 0x71, 0x62, 0xc9, 0x1f, 0x7e, 0x3d, 0x03, 0x02, 0x20, 0x51, 0x46, 0x83, 0x9a, 0x2b, 0x87, 0x10, 0x92, 0xc6, 0xe5, 0x02, 0x32, 0xe6, 0x07,
    0xd4, 0xc4, 0x16, 0x48, 0x7e, 0x09, 0x65, 0x8c, 0xe0, 0x3b, 0x2b, 0x31, 0x53, 0xc6, 0xf4, 0x0a, 0x36, 0x9a,

};


const uint8_t MBED_CLOUD_DEV_BOOTSTRAP_SERVER_ROOT_CA_CERTIFICATE[] = {
    0x30, 0x82, 0x01, 0xf6, 0x30, 0x82, 0x01, 0x9c, 0x02, 0x09, 0x00, 0x83, 0x50, 0xdd, 0x75, 0x5b, 0x09, 0x15, 0xd5, 0x30, 0x0a, 0x06,
    0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x81, 0x82, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x4b, 0x31, 0x13, 0x30, 0x11, 0x06,
    0x03, 0x55, 0x04, 0x08, 0x0c, 0x0a, 0x4b, 0x66, 0x61, 0x72, 0x2d, 0x6e, 0x65, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x4e, 0x65, 0x74,
    0x61, 0x6e, 0x69, 0x61, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03, 0x41, 0x52, 0x4d, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x03, 0x49,
    0x4f, 0x54, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x06, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x09, 0x01, 0x16, 0x10, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x31, 0x40, 0x61, 0x72, 0x6d, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x37, 0x30, 0x33,
    0x31, 0x33, 0x30, 0x38, 0x30, 0x38, 0x30, 0x36, 0x5a, 0x17, 0x0d, 0x32, 0x32, 0x30, 0x39, 0x30, 0x33, 0x30, 0x38, 0x30, 0x38, 0x30, 0x36, 0x5a, 0x30, 0x81, 0x82, 0x31, 0x0b, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x4b, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x0a, 0x4b, 0x66, 0x61, 0x72, 0x2d, 0x6e, 0x65, 0x74, 0x65,
    0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x4e, 0x65, 0x74, 0x61, 0x6e, 0x69, 0x61, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03,
    0x41, 0x52, 0x4d, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x03, 0x49, 0x4f, 0x54, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x06, 0x44, 0x65,
    0x76, 0x69, 0x63, 0x65, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x10, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x31, 0x40,
    0x61, 0x72, 0x6d, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07,
    0x03, 0x42, 0x00, 0x04, 0x5f, 0xa4, 0x54, 0x74, 0xe5, 0x5a, 0x55, 0xe5, 0x26, 0x31, 0x46, 0x3f, 0x0e, 0x23, 0xad, 0xe3, 0x4b, 0x2f, 0xb8, 0xb6, 0xa6, 0x3a, 0x2c, 0x04, 0x36, 0x50,
    0x1d, 0x08, 0xf9, 0x8e, 0x87, 0xd9, 0x0e, 0xc1, 0xdb, 0x06, 0xd0, 0x37, 0x26, 0xea, 0xbb, 0x6b, 0x2c, 0x69, 0xd1, 0x0e, 0x62, 0x95, 0x57, 0x3e, 0x29, 0xdf, 0x54, 0x7f, 0x24, 0x6d,
    0xcc, 0xc2, 0x40, 0x51, 0xe5, 0x09, 0x93, 0x02, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x27, 0x7e, 0x65,
    0x94, 0x4d, 0x53, 0x8a, 0xe9, 0x92, 0x2e, 0xbf, 0x94, 0xa9, 0xa5, 0xc5, 0x59, 0x6a, 0x84, 0xb9, 0x4f, 0xf6, 0x08, 0x27, 0xc5, 0xf5, 0xab, 0xc6, 0x65, 0x32, 0x3b, 0x45, 0x23, 0x02,
    0x21, 0x00, 0xb6, 0x73, 0x60, 0x15, 0x51, 0xaa, 0xf2, 0x14, 0x70, 0xc1, 0x5f, 0x36, 0x66, 0xaf, 0x87, 0xee, 0xc4, 0xa3, 0x54, 0x4e, 0x4f, 0x68, 0x76, 0x84, 0xee, 0x47, 0x98, 0xcc,
    0x60, 0x64, 0x60, 0x2d,
};


const uint8_t MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_PRIVATE_KEY[] = {
    0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x47, 0x23, 0xc9, 0xf1, 0xf3, 0xf3, 0x90, 0x94, 0xa9, 0x03, 0x9c, 0xc0, 0xfa, 0xcb, 0x27, 0x6b, 0x9d, 0x24, 0x35, 0xbc, 0xc6, 0xfd, 0x8e,
    0x15, 0xa6, 0x4f, 0x9a, 0x0a, 0x30, 0x32, 0xe3, 0xe2, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x5f, 0xa4, 0x54,
    0x74, 0xe5, 0x5a, 0x55, 0xe5, 0x26, 0x31, 0x46, 0x3f, 0x0e, 0x23, 0xad, 0xe3, 0x4b, 0x2f, 0xb8, 0xb6, 0xa6, 0x3a, 0x2c, 0x04, 0x36, 0x50, 0x1d, 0x08, 0xf9, 0x8e, 0x87, 0xd9, 0x0e,
    0xc1, 0xdb, 0x06, 0xd0, 0x37, 0x26, 0xea, 0xbb, 0x6b, 0x2c, 0x69, 0xd1, 0x0e, 0x62, 0x95, 0x57, 0x3e, 0x29, 0xdf, 0x54, 0x7f, 0x24, 0x6d, 0xcc, 0xc2, 0x40, 0x51, 0xe5, 0x09, 0x93,
    0x02,
};


const char MBED_CLOUD_DEV_MANUFACTURER[] = "dev_manufacturer";

const char MBED_CLOUD_DEV_MODEL_NUMBER[] = "dev_model_num";

const char MBED_CLOUD_DEV_SERIAL_NUMBER[] = "0";

const char MBED_CLOUD_DEV_DEVICE_TYPE[] = "dev_device_type";

const char MBED_CLOUD_DEV_HARDWARE_VERSION[] = "dev_hardware_version";

const uint32_t MBED_CLOUD_DEV_MEMORY_TOTAL_KB = 0;


const uint32_t MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_CERTIFICATE_SIZE = sizeof(MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_CERTIFICATE);
const uint32_t MBED_CLOUD_DEV_BOOTSTRAP_SERVER_ROOT_CA_CERTIFICATE_SIZE = sizeof(MBED_CLOUD_DEV_BOOTSTRAP_SERVER_ROOT_CA_CERTIFICATE);
const uint32_t MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_PRIVATE_KEY_SIZE = sizeof(MBED_CLOUD_DEV_BOOTSTRAP_DEVICE_PRIVATE_KEY);

#endif //__MBED_CLOUD_DEV_CREDENTIALS_H__
