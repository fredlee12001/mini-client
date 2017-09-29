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

#ifndef CLOUD_CLIENT_STORAGE_H
#define CLOUD_CLIENT_STORAGE_H

#define KEY_ACCOUNT_ID                          "mbed.AccountID"
#define KEY_INTERNAL_ENDPOINT                   "mbed.InternalEndpoint"
#define KEY_DEVICE_SOFTWAREVERSION              "mbed.SoftwareVersion"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CCS_STATUS_MEMORY_ERROR = -4,
    CCS_STATUS_VALIDATION_FAIL = -3,
    CCS_STATUS_KEY_DOESNT_EXIST = -2,
    CCS_STATUS_ERROR = -1,
    CCS_STATUS_SUCCESS = 0
} ccs_status_e;

/**
*  \brief Uninitializes the CFStore handle.
*/
ccs_status_e uninitialize_storage(void);

/**
*  \brief Initializes the CFStore handle.
*/
ccs_status_e initialize_storage(void);

/* Bootstrap credential handling methods */
ccs_status_e get_config_parameter(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length);
ccs_status_e set_config_parameter(const char* key, const uint8_t *buffer, const size_t buffer_size);
ccs_status_e delete_config_parameter(const char* key);
ccs_status_e size_config_parameter(const char* key, size_t* size_out);

ccs_status_e get_config_private_key(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length);
ccs_status_e set_config_private_key(const char* key, const uint8_t *buffer, const size_t buffer_size);
ccs_status_e delete_config_private_key(const char* key);

ccs_status_e get_config_public_key(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length);
ccs_status_e set_config_public_key(const char* key, const uint8_t *buffer, const size_t buffer_size);
ccs_status_e delete_config_public_key(const char* key);

ccs_status_e get_config_certificate(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length);
ccs_status_e set_config_certificate(const char* key, const uint8_t *buffer, const size_t buffer_size);
ccs_status_e delete_config_certificate(const char* key);



#ifdef __cplusplus
}
#endif
#endif // CLOUD_CLIENT_STORAGE_H
