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


#include "unity_fixture.h"
#include "factory_configurator_client.h"
#include "cn-cbor.h"
#include "fcc_bundle_handler.h"
#include "testdata.h"
#include "kcm_file_prefix_defs.h"
#include "storage.h"
#include "fcc_test_common_helpers.h"
#include "fcc_defs.h"


extern char g_tst_expected_all_warning_string_info[1024];
extern char *g_tst_no_warning_string;

#include "pal.h"

#ifdef USE_CBOR_CONTEXT
#define CONTEXT_NULL , NULL
#define CONTEXT_NULL_COMMA NULL,
#else
#define CONTEXT_NULL
#define CONTEXT_NULL_COMMA
#endif


TEST_GROUP(fcc_verify_device_4mbed_test);


TEST_SETUP(fcc_verify_device_4mbed_test)
{
    bool status;
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    memset(g_tst_expected_all_warning_string_info, 0, strlen(g_tst_expected_all_warning_string_info));

}

TEST_TEAR_DOWN(fcc_verify_device_4mbed_test)
{
    bool status;
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;

    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}


/**
* =====   Positive Tests  ======
*/

TEST(fcc_verify_device_4mbed_test, cbor_bootstrap_parameters)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_with_all_mandatory_parameters,
                                    sizeof(testdata_cbor_with_all_mandatory_parameters),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       FCC_STATUS_SUCCESS,
                                       NULL,
                                       NULL,
                                       g_tst_no_warning_string);

    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == FCC_STATUS_SUCCESS);
}

TEST(fcc_verify_device_4mbed_test, cbor_lwm2m_bootstrap_parameters)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_device_private_key_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_lwm2m_bootstrap_parameters, sizeof(testdata_cbor_lwm2m_bootstrap_parameters), &response_cbor_message, &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       FCC_STATUS_SUCCESS,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == FCC_STATUS_SUCCESS);
}

TEST(fcc_verify_device_4mbed_test, cbor_message_with_current_time)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    uint64_t now = 0;
    palStatus_t pal_status;
    bool status = false;

    // "CurrentTime" is an optional field, check that we able to enroll CBOR message with time value.
    fcc_status = fcc_bundle_handler(testdata_cbor_with_current_time, sizeof(testdata_cbor_with_current_time), &response_cbor_message, &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       FCC_STATUS_SUCCESS,
                                       NULL,
                                       NULL,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT(status == true);
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);


    // Wait a bit (one second) to allow time go a head
    pal_status = pal_osDelay(1000);
    TEST_ASSERT_EQUAL(PAL_SUCCESS, pal_status);

    // Check that time was set correctly
    now = pal_osGetTime();
    TEST_ASSERT(now > testdata_current_time);
}

/**
* =====  Negative Tests  ======
*/

TEST(fcc_verify_device_4mbed_test, cbor_without_endpoint_name)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_without_endpoint_name_parameter,
                                    sizeof(testdata_cbor_without_endpoint_name_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_endpoint_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_with_wrong_value_of_bootstrap)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_BOOTSTRAP_MODE_ERROR;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_with_wrong_value_of_bootstrap_mode,
                                    sizeof(testdata_cbor_with_wrong_value_of_bootstrap_mode),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_wrong_bootstrap_use_value_error_str,
                                       g_fcc_use_bootstrap_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_without_bootstrap_mode)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_without_bootstrap_mode_parameter,
                                    sizeof(testdata_cbor_without_bootstrap_mode_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_use_bootstrap_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_wrong_bootstrap_mode)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_bootstrap_mode_parameter,
                                    sizeof(testdata_cbor_wrong_bootstrap_mode_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_lwm2m_server_ca_certificate_name,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_without_root_ca)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_without_root_ca_parameter,
                                    sizeof(testdata_cbor_without_root_ca_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_bootstrap_server_ca_certificate_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");


}
TEST(fcc_verify_device_4mbed_test, cbor_without_dtls_private_key)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_server_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");

    fcc_status = fcc_bundle_handler(testdata_cbor_without_dtls_private_key_parameter,
                                    sizeof(testdata_cbor_without_dtls_private_key_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_bootstrap_device_private_key_name,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_bootstrap_false_without_lwm2m_ca)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_bootstrap_false_without_lwm2m_ca_parameter,
                                    sizeof(testdata_cbor_bootstrap_false_without_lwm2m_ca_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);


    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_lwm2m_server_ca_certificate_name,
                                       g_tst_expected_all_warning_string_info);

    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");


}
TEST(fcc_verify_device_4mbed_test, cbor_wrong_uri_prefix)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_URI_WRONG_FORMAT;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_server_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");



    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_bootstrap_uri_parameter,
                                    sizeof(testdata_cbor_wrong_bootstrap_uri_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_uri_wrong_format_error_str,
                                       g_fcc_bootstrap_server_uri_name,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_without_serial_number)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_ITEM_NOT_EXIST;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_without_serial_number_parameter,
                                    sizeof(testdata_cbor_without_serial_number_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_item_not_exists_error_str,
                                       g_fcc_device_serial_number_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_bootstrap_self_signed_ca)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_self_signed_ca,
                                    sizeof(testdata_cbor_self_signed_ca),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_server_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_self_signed_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_device_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");


    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");;


}
TEST(fcc_verify_device_4mbed_test, cbor_lwm2m_self_signed_ca)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_self_signed_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_device_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_lwm2m_self_signed_ca,
                                    sizeof(testdata_cbor_lwm2m_self_signed_ca),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_without_current_time_name)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_time_is_not_set_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_current_time_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_cert_time_validity_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_device_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_cert_time_validity_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_lwm2m_without_time,
                                    sizeof(testdata_cbor_lwm2m_without_time),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_with_empty_endpoint)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_EMPTY_ITEM;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_with_empty_endpoint,
                                    sizeof(testdata_cbor_with_empty_endpoint),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_empty_item_error_str,
                                       g_fcc_endpoint_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_uri_without_aid_data)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_URI_WRONG_FORMAT;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_uri_without_aid,
                                    sizeof(testdata_cbor_uri_without_aid),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_uri_wrong_format_error_str,
                                       g_fcc_bootstrap_server_uri_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_uri_with_wrong_prefix_location)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_URI_WRONG_FORMAT;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_server_uri_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_uri_wrong_prefix_location,
                                    sizeof(testdata_cbor_uri_wrong_prefix_location),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_uri_wrong_format_error_str,
                                       g_fcc_bootstrap_server_uri_name,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_with_empty_uri)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_EMPTY_ITEM;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_empty_uri,
                                    sizeof(testdata_cbor_empty_uri),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_empty_item_error_str,
                                       g_fcc_bootstrap_server_uri_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");
}

TEST(fcc_verify_device_4mbed_test, cbor_without_time_zone)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_time_zone_is_not_set_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_device_time_zone_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_without_time_zone,
                                    sizeof(testdata_cbor_without_time_zone),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_without_utc_offset)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_utc_offset_is_not_set_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_offset_from_utc_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_without_utc_offset,
                                    sizeof(testdata_cbor_without_utc_offset),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_wrong_utc_offset_number)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_UTC_OFFSET_WRONG_FORMAT;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;


    fcc_status = fcc_bundle_handler(testdata_cbor_with_wrong_utc_offset,
                                    sizeof(testdata_cbor_with_wrong_utc_offset),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_wrong_utc_offset_value_error_str,
                                       g_fcc_offset_from_utc_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_wrong_utc_offset_sign)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_UTC_OFFSET_WRONG_FORMAT;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_with_wrong_utc_offset_sign,
                                    sizeof(testdata_cbor_with_wrong_utc_offset_sign),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_wrong_utc_offset_value_error_str,
                                       g_fcc_offset_from_utc_parameter_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_with_empty_manufacture_name)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_ca_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_missing_parameter_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_with_empty_manf_name,
                                    sizeof(testdata_cbor_with_empty_manf_name),
                                    &response_cbor_message,
                                    &response_cbor_message_size);


    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST(fcc_verify_device_4mbed_test, cbor_with_wrong_integrity_chain)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_INVALID_CA_CERT_SIGNATURE;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_with_wrong_integrity_chain_parameters,
                                    sizeof(testdata_cbor_with_wrong_integrity_chain_parameters),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_wrong_ca_certificate_error_str,
                                       g_fcc_firmware_integrity_certificate_name,
                                       g_tst_no_warning_string);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_with_integrity_cert_expiration_less_than_10)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    strcat(g_tst_expected_all_warning_string_info, g_fcc_cert_validity_less_10_years_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, "\0");


    fcc_status = fcc_bundle_handler(testdata_cbor_with_integ_cert_expiration_less_than_10,
                                    sizeof(testdata_cbor_with_integ_cert_expiration_less_than_10),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       NULL,
                                       NULL,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_test, cbor_lwm2m_with_wrong_cn)
{
    fcc_status_e fcc_status;
    fcc_status_e expected_fcc_status = FCC_STATUS_INVALID_LWM2M_CN_ATTR;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_self_signed_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_lwm2m_device_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_lwm2m_with_wrong_cn,
                                    sizeof(testdata_cbor_lwm2m_with_wrong_cn),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_invalid_cn_certificate_error_str,
                                       g_fcc_lwm2m_device_certificate_name,
                                       g_tst_expected_all_warning_string_info);
    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}
TEST_GROUP_RUNNER(fcc_verify_device_4mbed_test)
{
    //negative tests
    //This test should be first , before pal_setTime was called (once this function was called the current time is in the system).
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_current_time_name);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_endpoint_name);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_empty_endpoint);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_bootstrap_mode);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_wrong_bootstrap_mode);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_bootstrap_false_without_lwm2m_ca);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_wrong_value_of_bootstrap);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_root_ca);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_dtls_private_key);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_empty_manufacture_name);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_serial_number);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_lwm2m_self_signed_ca);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_bootstrap_self_signed_ca);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_wrong_uri_prefix);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_uri_without_aid_data);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_uri_with_wrong_prefix_location);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_empty_uri);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_time_zone);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_without_utc_offset);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_wrong_utc_offset_number);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_wrong_utc_offset_sign);


    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_wrong_integrity_chain);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_with_integrity_cert_expiration_less_than_10);

    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_lwm2m_with_wrong_cn);

   // positive tests
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_bootstrap_parameters);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_lwm2m_bootstrap_parameters);
    RUN_TEST_CASE(fcc_verify_device_4mbed_test, cbor_message_with_current_time);




}
