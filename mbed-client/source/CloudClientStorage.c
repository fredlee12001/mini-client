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

#include <string.h>
#include "key_config_manager.h"
#include "CloudClientStorage.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "stor"

ccs_status_e uninitialize_storage(void)
{
    tr_debug("uninitialize_storage");

    kcm_status_e status = kcm_finalize();
    if(status != KCM_STATUS_SUCCESS) {
        tr_error("storage finalization error %d", status);
        return CCS_STATUS_ERROR;
    }
    return CCS_STATUS_SUCCESS;
}

ccs_status_e initialize_storage(void)
{
    tr_debug("initialize_storage");
    kcm_status_e status = kcm_init();
    if(status != KCM_STATUS_SUCCESS) {
        tr_error("storage initialization error %d", status);
        return CCS_STATUS_ERROR;
    }
    return CCS_STATUS_SUCCESS;
}

ccs_status_e get_config_parameter(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;
    tr_debug("get_config_parameter");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("get_config_parameter error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get parameter value to buffer
    kcm_status = kcm_item_get_data((const uint8_t*)key,
                                    strlen(key),
                                    KCM_CONFIG_ITEM,
                                    buffer,
                                    buffer_size,
                                    value_length);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("get_config_parameter kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("get_config_parameter - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e set_config_parameter(const char* key, const uint8_t *buffer, const size_t buffer_size)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("set_config_parameter");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("set_config_parameter error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Set parameter to storage
    kcm_status = kcm_item_store((const uint8_t*)key,
                                 strlen(key),
                                 KCM_CONFIG_ITEM,
                                 false,
                                 buffer,
                                 buffer_size,
                                 NULL);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("set_config_parameter kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("set_config_parameter - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e delete_config_parameter(const char* key)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("delete_config_parameter");

    if (key == NULL) {
        tr_error("delete_config_parameter error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Delete parameter from storage
    kcm_status = kcm_item_delete((const uint8_t*)key,
                                  strlen(key),
                                  KCM_CONFIG_ITEM);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("delete_config_parameter kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("delete_config_parameter - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e size_config_parameter(const char* key, size_t* size_out)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("size_config_parameter");

    if (key == NULL) {
        tr_error("size_config_parameter error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Delete parameter from storage
    kcm_status = kcm_item_get_data_size((const uint8_t*)key,
                                         strlen(key),
                                         KCM_CONFIG_ITEM,
                                         size_out);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("size_config_parameter kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("size_config_parameter - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e get_config_private_key(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("get_connector_private_key");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("get_connector_private_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get private key from storage
    kcm_status = kcm_item_get_data((const uint8_t*)key,
                                    strlen(key),
                                    KCM_PRIVATE_KEY_ITEM,
                                    buffer,
                                    buffer_size,
                                    value_length);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("get_connector_private_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("get_config_private_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e set_config_private_key(const char* key, const uint8_t *buffer, const size_t buffer_size)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("set_connector_private_key");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("set_connector_private_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Set private key to storage
    kcm_status = kcm_item_store((const uint8_t*)key,
                                 strlen(key),
                                 KCM_PRIVATE_KEY_ITEM,
                                 false,
                                 buffer,
                                 buffer_size,
                                 NULL);

    if (kcm_status == KCM_CRYPTO_STATUS_PRIVATE_KEY_VERIFICATION_FAILED) {
        tr_error("set_connector_private_key kcm validation error");
        return CCS_STATUS_VALIDATION_FAIL;
    }
    else if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("set_connector_private_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("set_config_private_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e delete_config_private_key(const char* key)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("delete_config_private_key");

    if (key == NULL) {
        tr_error("delete_config_private_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Delete private key from storage
    kcm_status = kcm_item_delete((const uint8_t*)key,
                                  strlen(key),
                                  KCM_PRIVATE_KEY_ITEM);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("delete_config_private_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("delete_config_private_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e get_config_public_key(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("get_connector_public_key");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("get_connector_public_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get parameter value to buffer
    kcm_status = kcm_item_get_data((const uint8_t*)key,
                                    strlen(key),
                                    KCM_PUBLIC_KEY_ITEM,
                                    buffer,
                                    buffer_size,
                                    value_length);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("get_connector_public_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("get_config_public_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e set_config_public_key(const char* key, const uint8_t *buffer, const size_t buffer_size)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("set_connector_public_key");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("set_connector_public_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Set public key to storage
    kcm_status = kcm_item_store((const uint8_t*)key,
                                 strlen(key),
                                 KCM_PUBLIC_KEY_ITEM,
                                 false,
                                 buffer,
                                 buffer_size,
                                 NULL);

    if (kcm_status == KCM_CRYPTO_STATUS_PUBLIC_KEY_VERIFICATION_FAILED) {
        tr_error("set_connector_public_key kcm validation error");
        return CCS_STATUS_VALIDATION_FAIL;
    }
    else if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("set_connector_public_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("set_config_public_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e delete_config_public_key(const char* key)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("delete_connector_public_key");

    if (key == NULL) {
        tr_error("delete_connector_public_key error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Delete the public key
    kcm_status = kcm_item_delete((const uint8_t*)key,
                                  strlen(key),
                                  KCM_PUBLIC_KEY_ITEM);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("delete_connector_public_key kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("delete_config_public_key - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e get_config_certificate(const char* key, uint8_t *buffer, const size_t buffer_size, size_t *value_length)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("get_config_certificate");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("get_config_certificate error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get parameter value to buffer
    kcm_status = kcm_item_get_data((const uint8_t*)key,
                                    strlen(key),
                                    KCM_CERTIFICATE_ITEM,
                                    buffer,
                                    buffer_size,
                                    value_length);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("get_config_certificate kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("get_config_certificate - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e set_config_certificate(const char* key, const uint8_t *buffer, const size_t buffer_size)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("set_config_certificate");

    if (key == NULL || buffer == NULL || buffer_size == 0) {
        tr_error("set_config_certificate error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get parameter value to buffer
    kcm_status = kcm_item_store((const uint8_t*)key,
                                 strlen(key),
                                 KCM_CERTIFICATE_ITEM,
                                 false,
                                 buffer,
                                 buffer_size,
                                 NULL);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("set_config_certificate kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("set_config_certificate - success");
    return CCS_STATUS_SUCCESS;
}

ccs_status_e delete_config_certificate(const char* key)
{
    kcm_status_e kcm_status = KCM_STATUS_ERROR;

    tr_debug("delete_config_certificate");

    if (key == NULL) {
        tr_error("delete_config_certificate error, invalid parameters");
        return CCS_STATUS_ERROR;
    }

    // Get parameter value to buffer
    kcm_status = kcm_item_delete((const uint8_t*)key,
                                  strlen(key),
                                  KCM_CERTIFICATE_ITEM);

    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("delete_config_certificate kcm get error %d", kcm_status);
        return CCS_STATUS_ERROR;
    }

    tr_debug("delete_config_certificate - success");
    return CCS_STATUS_SUCCESS;
}
