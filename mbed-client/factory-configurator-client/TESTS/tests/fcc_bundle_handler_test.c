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
#include "storage.h"
#include "fcc_test_common_helpers.h"

extern const uint8_t g_error_message[];
#ifdef USE_CBOR_CONTEXT
#define CONTEXT_NULL , NULL
#define CONTEXT_NULL_COMMA NULL,
#else
#define CONTEXT_NULL
#define CONTEXT_NULL_COMMA
#endif

TEST_GROUP(fcc_bundle_handler_test);


TEST_SETUP(fcc_bundle_handler_test)
{
    bool status;
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);
}

TEST_TEAR_DOWN(fcc_bundle_handler_test)
{
    bool status;

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

/**
* =====   Negative Tests  ======
*/

TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_group_names)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;
    fcc_status_e expected_fcc_status = FCC_STATUS_BUNDLE_INVALID_GROUP;

    //Call to fcc_bundle_handler with wrong key name group
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_key_group_name,
                                    sizeof(testdata_cbor_wrong_key_group_name),
                                    &response_cbor_message,
                                    &response_cbor_message_size);



    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);

    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);

    //Call to fcc_bundle_handler with wrong certificate name group
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_certificates_group_name,
                                    sizeof(testdata_cbor_wrong_certificates_group_name),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);


}

TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_scheme_version)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;
    fcc_status_e expected_fcc_status = FCC_STATUS_BUNDLE_INVALID_SCHEME;

    //Call to fcc_bundle_handler with cbor blob without scheme version group
    fcc_status = fcc_bundle_handler(testdata_cbor_without_scheme_version_group,
                                    sizeof(testdata_cbor_without_scheme_version_group),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);


    //Call to fcc_bundle_handler with wrong scheme version
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_scheme_version,
                                    sizeof(testdata_cbor_wrong_scheme_version),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);


}

TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_parameter_field_names)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;
    fcc_status_e expected_fcc_status = FCC_STATUS_KCM_ERROR;

    //Call to fcc_bundle_handler with cbor blob with wrong key name field
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_key_name_field,
                                    sizeof(testdata_cbor_wrong_key_name_field),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);

}
//open after fixing of PAL bug  308
IGNORE_TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_parameters)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;
    fcc_status_e expected_fcc_status = FCC_STATUS_KCM_ERROR;
    char *ecc_private_key_name = "E2EDevice";
    char *certificate_name = "UpdateVerification";

    //Call to fcc_bundle_handler with cbor blob with wrong key
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_ecc_private_key,
                                    sizeof(testdata_cbor_wrong_ecc_private_key),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message, response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_crypto_unsupported_curve_error_str,
                                       ecc_private_key_name,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);

    //Call to fcc_bundle_handler with cbor blob with wrong certificate
    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_certificate,
                                    sizeof(testdata_cbor_wrong_certificate),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_crypto_parsing_der_cert_error_str,
                                       certificate_name,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);

    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);




}

TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_with_unsupported_group)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    bool status = false;
    fcc_status_e expected_fcc_status = FCC_STATUS_BUNDLE_UNSUPPORTED_GROUP;

    //Call to fcc_bundle_handler with cbor blob with wrong key name field
    fcc_status = fcc_bundle_handler(testdata_cbor_store_request_with_csr,
                                    sizeof(testdata_cbor_store_request_with_csr),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    status = tst_check_fcc_output_info(response_cbor_message,
                                       response_cbor_message_size,
                                       expected_fcc_status,
                                       g_fcc_general_status_error_str,
                                       NULL,
                                       NULL);
    free(response_cbor_message);
    TEST_ASSERT(status == true && fcc_status == expected_fcc_status);

}

TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_wrong_function_parameters)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;

    //Null response buffer pointer
    fcc_status = fcc_bundle_handler(testdata_cbor1,
                                    sizeof(testdata_cbor1),
                                    NULL,
                                    &response_cbor_message_size);


    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);


    //Null response buffer size pointer
    fcc_status = fcc_bundle_handler(testdata_cbor1,
                                    sizeof(testdata_cbor1),
                                    &response_cbor_message,
                                    NULL);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);


    //Null cbor blob
    fcc_status = fcc_bundle_handler(NULL,
                                    sizeof(testdata_cbor1),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);

    free(response_cbor_message);

    //Zero cbor blob size
    fcc_status = fcc_bundle_handler(testdata_cbor1,
                                    0,
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);

    free(response_cbor_message);

    //Wrong cbor blob size
    fcc_status = fcc_bundle_handler(testdata_cbor1,
                                    sizeof(testdata_cbor1) - sizeof(testdata_cbor1) / 2,
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    TEST_ASSERT(fcc_status == FCC_STATUS_BUNDLE_ERROR);

    free(response_cbor_message);
}


TEST(fcc_bundle_handler_test, fcc_process_cbor_blob_wrong_item_format)
{
    fcc_status_e status = FCC_STATUS_SUCCESS;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;

    //Call to fcc_bundle_handler with cbor blob with wrong key format field
    status = fcc_bundle_handler((const uint8_t*)testdata_cbor_wrong_key_format_name,
                                sizeof(testdata_cbor_wrong_key_format_name),
                                &response_cbor_message,
                                &response_cbor_message_size);

    TEST_ASSERT(status == FCC_STATUS_BUNDLE_ERROR);
    free(response_cbor_message);

    //Call to fcc_bundle_handler with cbor blob with wrong key format field
    status = fcc_bundle_handler((const uint8_t*)testdata_cbor_certificate_null_format_name,
                                sizeof(testdata_cbor_certificate_null_format_name),
                                &response_cbor_message,
                                &response_cbor_message_size);

    TEST_ASSERT(status == FCC_STATUS_BUNDLE_ERROR);
    free(response_cbor_message);

    //Call to fcc_bundle_handler with cbor blob with 'pem' format field
    status = fcc_bundle_handler((const uint8_t*)testdata_cbor_certificate_pem_format_name,
                                sizeof(testdata_cbor_certificate_pem_format_name),
                                &response_cbor_message,
                                &response_cbor_message_size);

    TEST_ASSERT(status == FCC_STATUS_BUNDLE_ERROR);
    free(response_cbor_message);

}


TEST_GROUP_RUNNER(fcc_bundle_handler_test)
{
    //negative tests
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_group_names);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_scheme_version);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_parameter_field_names);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_with_wrong_parameters);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_with_unsupported_group);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_wrong_function_parameters);
    RUN_TEST_CASE(fcc_bundle_handler_test, fcc_process_cbor_blob_wrong_item_format);
}
