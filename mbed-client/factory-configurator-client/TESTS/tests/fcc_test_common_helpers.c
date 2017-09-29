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


#include <assert.h>

#include "fcc_test_common_helpers.h"
#include "pv_error_handling.h"
#include "pv_macros.h"
#include "testdata.h"
#include "kcm_file_prefix_defs.h"
#include "key_config_manager.h"
#include "cn-cbor.h"
#include "fcc_bundle_handler.h"
#include "fcc_output_info_handler.h"
#include "esfs.h"

#define MAX_RAND_ITERATIONS 10

#ifdef USE_CBOR_CONTEXT
#define CONTEXT_NULL , NULL
#define CONTEXT_NULL_COMMA NULL,
#else
#define CONTEXT_NULL
#define CONTEXT_NULL_COMMA
#endif

char g_tst_expected_all_warning_string_info[1024];
char *g_tst_no_warning_string = NULL;

test_certificate_chain_data_s test_certificate_vector[CERTIFICATE_TEST_ITERATIONS][NUMBER_OF_CERTIFICATES_IN_CHAIN] = {
    {
        {
            .certificate = testdata_x509_pth_ca_chain_1der,
            .certificate_size = sizeof(testdata_x509_pth_ca_chain_1der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_1der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_1der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_2der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_2der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_3der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_3der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_4der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_4der),
        }
    },
    {
        {
            .certificate = testdata_x509_pth_chain_child_4der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_4der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_3der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_3der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_2der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_2der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_1der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_1der),
        },
        {
            .certificate = testdata_x509_pth_ca_chain_1der,
            .certificate_size = sizeof(testdata_x509_pth_ca_chain_1der),
        }
    },
    {
        {
            .certificate = testdata_x509_pth_chain_child_3der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_3der),
        },
        {

            .certificate = testdata_x509_pth_chain_child_4der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_4der),
        },
        {
            .certificate = testdata_x509_pth_ca_chain_1der,
            .certificate_size = sizeof(testdata_x509_pth_ca_chain_1der),

        },
        {
            .certificate = testdata_x509_pth_chain_child_1der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_1der),
        },
        {
            .certificate = testdata_x509_pth_chain_child_2der,
            .certificate_size = sizeof(testdata_x509_pth_chain_child_2der),
        }
    },
};

test_kcm_item_data_template_s test_kcm_vector[] = {
    {
        .kcm_item_name = (uint8_t*)testdata_priv_ecc_key1der_name,
        .kcm_item_data = testdata_priv_ecc_key1der,
        .kcm_item_data_size = sizeof(testdata_priv_ecc_key1der),
        .kcm_item_type = KCM_PRIVATE_KEY_ITEM,
        .kcm_prefix = KCM_FILE_PREFIX_PRIVATE_KEY,

    },
    {
        .kcm_item_name = (uint8_t*)testdata_pub_ecc_key1der_name,
        .kcm_item_data = testdata_pub_ecc_key1der,
        .kcm_item_data_size = sizeof(testdata_pub_ecc_key1der),
        .kcm_item_type = KCM_PUBLIC_KEY_ITEM,
        .kcm_prefix = KCM_FILE_PREFIX_PUBLIC_KEY
    },
    {
        .kcm_item_name = (uint8_t*)testdata_aeskey4key_name,
        .kcm_item_data = testdata_aeskey4key,
        .kcm_item_data_size = sizeof(testdata_aeskey4key),
        .kcm_item_type = KCM_SYMMETRIC_KEY_ITEM,
        .kcm_prefix = KCM_FILE_PREFIX_SYMMETRIC_KEY
    },
    {
        .kcm_item_name = (uint8_t*)testdata_certificate1der_name,
        .kcm_item_data = testdata_certificate1der,
        .kcm_item_data_size = sizeof(testdata_certificate1der),
        .kcm_item_type = KCM_CERTIFICATE_ITEM,
        .kcm_prefix = KCM_FILE_PREFIX_CERTIFICATE
    },
    {
        .kcm_item_name = (uint8_t*)testdata_bootstrap_urlbin_name,
        .kcm_item_data = testdata_bootstrap_urlbin,
        .kcm_item_data_size = sizeof(testdata_bootstrap_urlbin),
        .kcm_item_type = KCM_CONFIG_ITEM,
        .kcm_prefix = KCM_FILE_PREFIX_CONFIG_PARAM
    }
};

size_t g_size_of_test_kcm_vector = sizeof(test_kcm_vector) / sizeof(test_kcm_item_data_template_s);

kcm_status_e check_encryption_settings(const uint8_t *file_name, size_t file_name_length, bool *encryption_status)
{
    esfs_result_e esfs_status;
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;

    uint16_t esfs_mode = 0;
    esfs_file_t esfs_file_h;

    //check esfs encryption bit
    esfs_status = esfs_open(file_name, file_name_length, &esfs_mode, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), KCM_STATUS_STORAGE_ERROR, "Failed opening file (esfs_status %d)", esfs_status);

    esfs_status = esfs_close(&esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), KCM_STATUS_STORAGE_ERROR, "Failed closing file (esfs_status %d)", esfs_status);

    *encryption_status = ((esfs_mode & ESFS_ENCRYPTED) == ESFS_ENCRYPTED);

    return kcm_status;
}


bool tst_check_fcc_output_info(uint8_t *response_message,
                               size_t size_of_response_message,
                               fcc_status_e expected_status,
                               const char *expected_error_string,
                               const char *expected_failed_item_name,
                               const char *expected_warning_string)
{
    cn_cbor *response_cbor_cb = NULL;
    cn_cbor *group_value_cb = NULL;
    cn_cbor_errback err;
    bool status = true;


    if (expected_status != FCC_STATUS_SUCCESS && expected_error_string == NULL) {
        SA_PV_LOG_ERR("Should be some error string");
        return false;
    }

    //Decode response message
    response_cbor_cb = cn_cbor_decode(response_message, size_of_response_message CONTEXT_NULL, &err);
    if (response_cbor_cb == NULL) {
        SA_PV_LOG_ERR("cn_cbor_decode failed");
        return false;
    }

    /**
    *      Check status number
    */
    //Get value of return status group
    group_value_cb = cn_cbor_mapget_string(response_cbor_cb, FCC_RETURN_STATUS_GROUP_NAME);
    if (group_value_cb == NULL) {
        SA_PV_LOG_ERR("\ncn_cbor_mapget_string of status group failed");
        status = false;
        goto exit;
    }

    //Check that stored status value is the same as expected status
    if (group_value_cb->type != CN_CBOR_UINT) {
        SA_PV_LOG_ERR("\nThe retrieved cbor type is not as expected");
        status = false;
        goto exit;
    }
    if (expected_status != group_value_cb->v.uint) {
        SA_PV_LOG_ERR("\nStatus value is different than expected expected_status is %d group_value_cb->v.uint is  %llu", expected_status, group_value_cb->v.uint);
        status = false;
        goto exit;
    }

    /**
    *      Check error string info
    */
    group_value_cb = NULL;
    //Get name of failed item
    group_value_cb = cn_cbor_mapget_string(response_cbor_cb, FCC_ERROR_INFO_GROUP_NAME);
    if (group_value_cb ==  NULL) {
        SA_PV_LOG_ERR("\ncn_cbor_mapget_string of info message group failed");
        status = false;
        goto exit;
    }
    if (group_value_cb->type != CN_CBOR_BYTES) {
        SA_PV_LOG_ERR("Retrieved cbor type is not as expected");
        status = false;
        goto exit;
    }

    if (expected_error_string != NULL) {
        //Compare error string with the expected string
        if (memcmp(group_value_cb->v.bytes, expected_error_string, strlen(expected_error_string)) != 0) {
            SA_PV_LOG_ERR("\nThe retrieved info message is different than expected, expected_error_string is %s strlen(expected_error_string) is %" PRIu32 ", ", expected_error_string, (uint32_t)strlen(expected_error_string));
            SA_PV_LOG_ERR("\nThe retrieved error info is \n-----\n%.*s----- ", (int)group_value_cb->length, group_value_cb->v.bytes);
            status = false;
            goto exit;
        }
    }

    if (expected_failed_item_name != NULL) {
        //Compare failed item name  with the expected string
        if (memcmp(group_value_cb->v.bytes + strlen(expected_error_string), expected_failed_item_name, strlen(expected_failed_item_name)) != 0) {
            SA_PV_LOG_ERR("\nThe retrieved info message is different than expected  expected_failed_item_name %s ", expected_failed_item_name);
            status = false;
            goto exit;
        }
    }
    /**
    *      Check warning string info
    */

    group_value_cb = NULL;
    //Get name of failed item
    group_value_cb = cn_cbor_mapget_string(response_cbor_cb, FCC_WARNING_INFO_GROUP_NAME);
    if (group_value_cb == NULL && expected_warning_string != NULL) {
        SA_PV_LOG_ERR("\ncn_cbor_mapget_string of info warning string info failed");
        status = false;
        goto exit;
    }
    if (group_value_cb == NULL && expected_warning_string == NULL) {
        status = true;
        goto exit;
    }

    if (group_value_cb->length != 0 && expected_warning_string == NULL) {
        SA_PV_LOG_ERR("\nThe retrieved warning info is different than expected,expected warning was empty  ");
        SA_PV_LOG_ERR("\nThe retrieved warning info is \n-----\n%.*s----- ", (int)group_value_cb->length, group_value_cb->v.bytes);
        status = false;
        goto exit;
    }
    //Compare wrning with the expected warning string
    if (memcmp(group_value_cb->v.bytes, expected_warning_string, group_value_cb->length) != 0) {
        SA_PV_LOG_ERR("\nThe retrieved warning info is different than expected, expected_warning_string: \n-----\n%s----- ", expected_warning_string);
        SA_PV_LOG_ERR("\nThe retrieved warning info is \n-----\n%.*s----- ",(int)group_value_cb->length, group_value_cb->v.bytes);
        status = false;
        goto exit;
    }

exit:
    cn_cbor_free(response_cbor_cb CONTEXT_NULL);
    return status;
}

#ifdef PV_TEST_REPRODUCE_RANDOM_STATE
#include "pv_random_tls.h"

// The function TestSetreproduced_seed() should be run only if we want to set fixed seed to pseudo random function
// You should place your seed shown in your TTY console at the very beginning of your run into "reproduced_seed"
bool tst_set_reproduced_seed(void)
{
    uint8_t reproduced_seed[] = {};

    return pv_test_only_random_ctr_drbg_ctx_set(reproduced_seed, sizeof(reproduced_seed));
}
#endif
