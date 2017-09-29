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


TEST_GROUP(fcc_verify_device_4mbed_python_certs_test);


TEST_SETUP(fcc_verify_device_4mbed_python_certs_test)
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

TEST_TEAR_DOWN(fcc_verify_device_4mbed_python_certs_test)
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
TEST(fcc_verify_device_4mbed_python_certs_test, cbor_python_certificate)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_python_certificates,
         sizeof(testdata_cbor_python_certificates),
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
/**
* =====   Negative Tests  ======
*/
TEST(fcc_verify_device_4mbed_python_certs_test, cbor_python_child_certificate_is_expired)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e expected_fcc_status = FCC_STATUS_EXPIRED_CERTIFICATE;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_python_child_expired_certificates,
        sizeof(testdata_cbor_python_child_expired_certificates),
        &response_cbor_message,
        &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
        response_cbor_message_size,
        expected_fcc_status,
        g_fcc_crypto_cert_expired_error_str,
        g_fcc_firmware_integrity_certificate_name,
        g_tst_no_warning_string);

    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_python_certs_test, cbor_python_future_certificates)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e expected_fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    //Compose expected cbor warning string
    strcat(g_tst_expected_all_warning_string_info, g_fcc_cert_time_validity_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n");
    strcat(g_tst_expected_all_warning_string_info, g_fcc_cert_validity_less_10_years_warning_str);
    strcat(g_tst_expected_all_warning_string_info, g_fcc_firmware_integrity_certificate_name);
    strcat(g_tst_expected_all_warning_string_info, "\n\0");

    fcc_status = fcc_bundle_handler(testdata_cbor_python_future_firmw_intg_certificates,
        sizeof(testdata_cbor_python_future_firmw_intg_certificates),
        &response_cbor_message,
        &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
        response_cbor_message_size,
        expected_fcc_status,
        NULL,
        NULL,
        g_tst_expected_all_warning_string_info);

    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == FCC_STATUS_SUCCESS, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");

}

TEST(fcc_verify_device_4mbed_python_certs_test, cbor_python_device_expired_certificate)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e expected_fcc_status = FCC_STATUS_EXPIRED_CERTIFICATE;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_python_device_expired_certificate,
        sizeof(testdata_cbor_python_device_expired_certificate),
        &response_cbor_message,
        &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
        response_cbor_message_size,
        expected_fcc_status,
        g_fcc_crypto_cert_expired_error_str,
        g_fcc_bootstrap_device_certificate_name,
        NULL);

    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");
}
TEST(fcc_verify_device_4mbed_python_certs_test, cbor_python_fw_integ_untrusted)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e expected_fcc_status = FCC_STATUS_INVALID_CA_CERT_SIGNATURE;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;

    fcc_status = fcc_bundle_handler(testdata_cbor_python_fw_integrity_intrusted_certificate,
        sizeof(testdata_cbor_python_fw_integrity_intrusted_certificate),
        &response_cbor_message,
        &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
        response_cbor_message_size,
        expected_fcc_status,
        g_fcc_wrong_ca_certificate_error_str,
        g_fcc_firmware_integrity_certificate_name,
        NULL);

    free(response_cbor_message);
    TEST_ASSERT_MESSAGE(fcc_status == expected_fcc_status, "fcc_status different than expected");
    TEST_ASSERT_MESSAGE(status == true, "tst_check_fcc_output_info failed");
}

TEST_GROUP_RUNNER(fcc_verify_device_4mbed_python_certs_test)
{
/**
* =====   Positive Tests  ======
*/
RUN_TEST_CASE(fcc_verify_device_4mbed_python_certs_test, cbor_python_certificate);
/**
* =====   Negative Tests  ======
*/
    RUN_TEST_CASE(fcc_verify_device_4mbed_python_certs_test, cbor_python_child_certificate_is_expired);
    RUN_TEST_CASE(fcc_verify_device_4mbed_python_certs_test, cbor_python_future_certificates);
    RUN_TEST_CASE(fcc_verify_device_4mbed_python_certs_test, cbor_python_device_expired_certificate);
    RUN_TEST_CASE(fcc_verify_device_4mbed_python_certs_test, cbor_python_fw_integ_untrusted);
}
