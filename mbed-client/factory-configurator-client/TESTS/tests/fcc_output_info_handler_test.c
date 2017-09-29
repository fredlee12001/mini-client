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
#include "fcc_output_info_handler.h"
#include "fcc_defs.h"


#include "pal.h"

#ifdef USE_CBOR_CONTEXT
#define CONTEXT_NULL , NULL
#define CONTEXT_NULL_COMMA NULL,
#else
#define CONTEXT_NULL
#define CONTEXT_NULL_COMMA
#endif

char g_tst_expected_warning_string_info[100];
char g_tst_expected_error_string_info[100];



TEST_GROUP(fcc_output_info_handler_test);


TEST_SETUP(fcc_output_info_handler_test)
{
    bool status;
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    memset(g_tst_expected_error_string_info, 0, strlen(g_tst_expected_error_string_info));
}

TEST_TEAR_DOWN(fcc_output_info_handler_test)
{
    bool status;

    status = fcc_storage_delete();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

TEST(fcc_output_info_handler_test, check_fcc_status_strings)
{
    char *error_string_info = NULL;
    fcc_status_e fcc_status_index = 0;

    for (fcc_status_index = 1; fcc_status_index < FCC_MAX_STATUS; fcc_status_index++) {
        error_string_info = NULL;
        error_string_info = fcc_get_fcc_error_string((fcc_status_e)fcc_status_index);
        TEST_ASSERT(error_string_info != NULL);
    }
}

TEST(fcc_output_info_handler_test, check_kcm_status_strings)
{
    char *error_string_info = NULL;
    int kcm_status_index = 0;

    for (kcm_status_index = 1; kcm_status_index < KCM_MAX_STATUS; kcm_status_index++) {
        error_string_info = NULL;
        error_string_info = fcc_get_kcm_error_string((kcm_status_e)kcm_status_index);
        TEST_ASSERT(error_string_info != NULL);
    }
}
TEST(fcc_output_info_handler_test, store_bundle_out_info_with_wrong_params)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    char *item_name = "certificate";

    //kcm_status = KCM_STATUS_SUCCESS
    fcc_status = fcc_bundle_store_error_info((const uint8_t*)item_name, (size_t)strlen(item_name), kcm_status);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);

    fcc_status = fcc_bundle_store_error_info((const uint8_t*)item_name, (size_t)0, kcm_status);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);

}
TEST(fcc_output_info_handler_test, store_fcc_out_info_with_wrong_params)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    fcc_status_e check_output_fcc_status = FCC_STATUS_SUCCESS;
    char *item_name = "certificate";

    //fcc_status = FCC_STATUS_SUCCESS
    check_output_fcc_status = fcc_store_error_info((const uint8_t*)item_name, strlen(item_name), fcc_status);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_INVALID_PARAMETER);

    check_output_fcc_status = fcc_store_error_info((const uint8_t*)item_name, (size_t)0, fcc_status);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_INVALID_PARAMETER);
}

TEST(fcc_output_info_handler_test, store_warning_with_wrong_params)
{
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    char *item_name = "certificate";


    fcc_status = fcc_store_warning_info((const uint8_t*)item_name, (size_t)strlen(item_name), NULL);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);

    fcc_status = fcc_store_warning_info((const uint8_t*)item_name, (size_t)0, g_fcc_bootstrap_mode_false_warning_str);
    TEST_ASSERT(fcc_status == FCC_STATUS_INVALID_PARAMETER);
}


TEST(fcc_output_info_handler_test, check_out_info_with_error)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    size_t expected_number_of_warnings = 1;
    fcc_output_info_s  *output_info = NULL;

    strcat(g_tst_expected_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_use_bootstrap_parameter_name);

    strcat(g_tst_expected_error_string_info, g_fcc_item_not_exists_error_str);
    strcat(g_tst_expected_error_string_info, g_fcc_lwm2m_server_ca_certificate_name);

    fcc_status = fcc_bundle_handler(testdata_cbor_wrong_bootstrap_mode_parameter,
                                    sizeof(testdata_cbor_wrong_bootstrap_mode_parameter),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_ITEM_NOT_EXIST);

    //output_info = get_output_info();

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info != NULL);
    //error_string_info should not be empty
    TEST_ASSERT(output_info->error_string_info != NULL);
    //Check output_info->error_string_info with g_tst_expected_error_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_error_string_info, output_info->error_string_info,"Failed in check of error message");
    //head_of_warning_list should not be empty
    TEST_ASSERT(output_info->head_of_warning_list != NULL);
    //warning_info_string should not be empty
    TEST_ASSERT(output_info->head_of_warning_list->warning_info_string != NULL);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, output_info->head_of_warning_list->warning_info_string,"Failed in check of warning message");
    //check that only one warning was in the output info
    TEST_ASSERT(output_info->size_of_warning_info_list == expected_number_of_warnings);

}
TEST(fcc_output_info_handler_test, check_out_info_struct_initialization)
{
    fcc_output_info_s  *output_info = NULL;

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info != NULL);
    //error_string_info should not be empty
    TEST_ASSERT(output_info->error_string_info == NULL);
    TEST_ASSERT(output_info->head_of_warning_list == NULL);
    TEST_ASSERT(output_info->size_of_warning_info_list == 0);
}


TEST(fcc_output_info_handler_test, check_out_info_with_warnings)
{
    fcc_status_e fcc_status;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;
    size_t expected_number_of_warnings = 1;
    fcc_output_info_s  *output_info = NULL;

    fcc_status = fcc_bundle_handler(testdata_cbor_lwm2m_without_time,
                                    sizeof(testdata_cbor_lwm2m_without_time),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info != NULL);
    //error_string_info should  be empty
    TEST_ASSERT(output_info->error_string_info == NULL);
    //head_of_warning_list should not be empty
    TEST_ASSERT(output_info->head_of_warning_list != NULL);
    //check that only one warning was in the output info
    TEST_ASSERT(output_info->size_of_warning_info_list == expected_number_of_warnings);
    //warning_info_string should not be empty
    TEST_ASSERT(output_info->head_of_warning_list->warning_info_string != NULL);

    strcat(g_tst_expected_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, output_info->head_of_warning_list->warning_info_string, "Failed in check of first warning message");


    //head_of_warning_list->next-  should be empty
    TEST_ASSERT(output_info->head_of_warning_list->next == NULL);
}

TEST(fcc_output_info_handler_test, check_store_multiple_warnings)
{
    fcc_status_e check_output_fcc_status = FCC_STATUS_SUCCESS;
    size_t expected_number_of_warnings = 6;
    fcc_output_info_s  *output_info = NULL;
    fcc_warning_info_s *warning_list = NULL;

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_use_bootstrap_parameter_name,
                              (size_t)strlen(g_fcc_use_bootstrap_parameter_name),
                              g_fcc_bootstrap_mode_false_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_current_time_parameter_name,
                              (size_t)strlen(g_fcc_current_time_parameter_name),
                              g_fcc_time_is_not_set_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_bootstrap_server_ca_certificate_name,
                              (size_t)strlen(g_fcc_bootstrap_server_ca_certificate_name),
                              g_fcc_self_signed_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_manufacturer_parameter_name,
                              (size_t)strlen(g_fcc_manufacturer_parameter_name),
                              g_fcc_item_is_empty_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_bootstrap_server_uri_name,
                              (size_t)strlen(g_fcc_bootstrap_server_uri_name),
                              g_fcc_redundant_item_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);

    check_output_fcc_status = fcc_store_warning_info((const uint8_t*)g_fcc_offset_from_utc_parameter_name,
                              (size_t)strlen(g_fcc_offset_from_utc_parameter_name),
                              g_fcc_utc_offset_is_not_set_warning_str);
    TEST_ASSERT(check_output_fcc_status == FCC_STATUS_SUCCESS);


    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info != NULL);
    //error_string_info should  be empty
    TEST_ASSERT(output_info->error_string_info == NULL);
    //head_of_warning_list should not be empty
    TEST_ASSERT(output_info->head_of_warning_list != NULL);
    //check that only one warning was in the output info
    TEST_ASSERT(output_info->size_of_warning_info_list == expected_number_of_warnings);

    warning_list = output_info->head_of_warning_list;

    strcat(g_tst_expected_warning_string_info, g_fcc_bootstrap_mode_false_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_use_bootstrap_parameter_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of first warning message");

    TEST_ASSERT(warning_list->next != NULL);
    warning_list = warning_list->next;
    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    strcat(g_tst_expected_warning_string_info, g_fcc_time_is_not_set_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_current_time_parameter_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of second warning message");

    TEST_ASSERT(warning_list->next != NULL);
    warning_list = warning_list->next;
    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    strcat(g_tst_expected_warning_string_info, g_fcc_self_signed_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_bootstrap_server_ca_certificate_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of third warning message");

    TEST_ASSERT(warning_list->next != NULL);
    warning_list = warning_list->next;
    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    strcat(g_tst_expected_warning_string_info, g_fcc_item_is_empty_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_manufacturer_parameter_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of forth warning message");

    TEST_ASSERT(warning_list->next != NULL);
    warning_list = warning_list->next;
    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    strcat(g_tst_expected_warning_string_info, g_fcc_redundant_item_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_bootstrap_server_uri_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of fifth warning message");

    TEST_ASSERT(warning_list->next != NULL);
    warning_list = warning_list->next;
    memset(g_tst_expected_warning_string_info, 0, strlen(g_tst_expected_warning_string_info));
    strcat(g_tst_expected_warning_string_info, g_fcc_utc_offset_is_not_set_warning_str);
    strcat(g_tst_expected_warning_string_info, g_fcc_offset_from_utc_parameter_name);
    //Check warning_info_string with g_tst_expected_warning_string_info
    TEST_ASSERT_EQUAL_STRING_MESSAGE(g_tst_expected_warning_string_info, warning_list->warning_info_string, "Failed in check of sixth warning message");

    TEST_ASSERT(warning_list->next == NULL);
}

TEST(fcc_output_info_handler_test, check_out_info_apis_after_finalize)
{
    fcc_status_e fcc_status;
    fcc_output_info_s  *output_info = NULL;
    bool status = false;

    //finalize store
    fcc_status = fcc_finalize();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info == NULL);

    // We intentionally initializing FCC to avoid tear down error upon finalization
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}
//open after fixing of PAL bug  308
IGNORE_TEST(fcc_output_info_handler_test, check_out_info_struct_between_operations)
{
    fcc_status_e fcc_status;
    fcc_output_info_s  *output_info = NULL;
    uint8_t *response_cbor_message = NULL;
    size_t response_cbor_message_size;

    memset(g_tst_expected_error_string_info, 0, strlen(g_tst_expected_error_string_info));
    strcat(g_tst_expected_error_string_info, g_fcc_crypto_cert_md_alg_error_str);
    strcat(g_tst_expected_error_string_info, g_fcc_bootstrap_server_ca_certificate_name);

    fcc_status = fcc_bundle_handler(testdata_cbor_partial_parameters_1,
                                    sizeof(testdata_cbor_partial_parameters_1),
                                    &response_cbor_message,
                                    &response_cbor_message_size);

    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_KCM_ERROR);

    //Error from bundle handle certificate store
    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info->head_of_warning_list == NULL);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(output_info->error_string_info, g_tst_expected_error_string_info, "Failed in check of error message");

    memset(g_tst_expected_error_string_info, 0, strlen(g_tst_expected_error_string_info));
    strcat(g_tst_expected_error_string_info, g_fcc_item_not_exists_error_str);
    strcat(g_tst_expected_error_string_info, g_fcc_bootstrap_device_certificate_name);

    fcc_status = fcc_bundle_handler(testdata_cbor_partial_parameters_2,
                                    sizeof(testdata_cbor_partial_parameters_2),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_ITEM_NOT_EXIST);

    TEST_ASSERT(output_info->head_of_warning_list == NULL);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(output_info->error_string_info, g_tst_expected_error_string_info, "Failed in check of error message");

    memset(g_tst_expected_error_string_info, 0, strlen(g_tst_expected_error_string_info));

    fcc_status = fcc_bundle_handler(testdata_cbor_partial_parameters_3,
                                    sizeof(testdata_cbor_partial_parameters_3),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info->error_string_info == NULL);
    TEST_ASSERT(output_info->size_of_warning_info_list == 2);

    fcc_status = fcc_bundle_handler(testdata_cbor_partial_parameters_4,
                                    sizeof(testdata_cbor_partial_parameters_4),
                                    &response_cbor_message,
                                    &response_cbor_message_size);
    free(response_cbor_message);
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    output_info = fcc_get_error_and_warning_data();
    TEST_ASSERT(output_info->error_string_info == NULL);
    TEST_ASSERT(output_info->size_of_warning_info_list == 0);
}


TEST_GROUP_RUNNER(fcc_output_info_handler_test)
{
    //positive tests
    RUN_TEST_CASE(fcc_output_info_handler_test, check_fcc_status_strings);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_kcm_status_strings);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_store_multiple_warnings);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_out_info_struct_initialization);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_out_info_struct_between_operations);

    //Negative tests
    RUN_TEST_CASE(fcc_output_info_handler_test, store_bundle_out_info_with_wrong_params);
    RUN_TEST_CASE(fcc_output_info_handler_test, store_fcc_out_info_with_wrong_params);
    RUN_TEST_CASE(fcc_output_info_handler_test, store_warning_with_wrong_params);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_out_info_with_error);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_out_info_with_warnings);
    RUN_TEST_CASE(fcc_output_info_handler_test, check_out_info_apis_after_finalize);
}
