//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef MBED_CLOUD_CLIENT_UPDATE_RESOURCES_H
#define MBED_CLOUD_CLIENT_UPDATE_RESOURCES_H

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "update-client-hub/update_client_hub.h"
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_ID
extern const uint8_t arm_uc_vendor_id[];
extern const uint16_t arm_uc_vendor_id_size;
extern const uint8_t arm_uc_class_id[];
extern const uint16_t arm_uc_class_id_size;
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_CERT
extern const uint8_t arm_uc_default_fingerprint[];
extern const uint16_t arm_uc_default_fingerprint_size;
extern const uint8_t arm_uc_default_certificate[];
extern const uint16_t arm_uc_default_certificate_size;
#endif

#ifdef MBED_CLOUD_DEV_UPDATE_PSK
extern const uint8_t arm_uc_default_psk[];
extern uint16_t arm_uc_default_psk_bits;
#endif

#ifdef __cplusplus
}
#endif

#endif // MBED_CLOUD_CLIENT_UPDATE_RESOURCES_H
