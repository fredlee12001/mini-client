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
#include "fcc_test_common_helpers.h"
#include "key_config_manager.h"
#include "storage.h"
#include "esfs.h"
#include "testdata.h"
#include "pv_log.h"
#include "pv_macros.h"
#include "kcm_file_prefix_defs.h"


extern test_kcm_item_data_template_s test_kcm_vector[];
extern size_t g_size_of_test_kcm_vector;
//FIXME: add more tests
//-call to store priv and get public
//--call to get Size without get + heap allocation


#define TEST_KCM_ZERO_SIZE   0
#define TEST_KCM_FILE_PATH_MAX_LEN   63

static void kcm_test_store_data()
{
    uint32_t i;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    test_kcm_item_data_template_s kcm_vector;
    size_t act_kcm_item_data_size;
    bool kcm_is_factory = true;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        uint8_t expected_kcm_item_data[kcm_vector.kcm_item_data_size];

        SA_PV_LOG_INFO("Store test loop %" PRIu32 "", i);

        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &expected_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        TEST_ASSERT_EQUAL(kcm_vector.kcm_item_data_size, expected_kcm_item_data_size);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(kcm_vector.kcm_item_data, expected_kcm_item_data, kcm_vector.kcm_item_data_size, "Inconsistent kcm data");
    }

}


TEST_GROUP(key_config_manager);


TEST_SETUP(key_config_manager)
{
    fcc_status_e status;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    fcc_storage_delete();
    TEST_SKIP_EXECUTION_ON_FAILURE();
}

TEST_TEAR_DOWN(key_config_manager)
{
    fcc_status_e status;

    fcc_storage_delete();
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}


/**
* =====   Positive Tests  ======
*/

TEST(key_config_manager, kcm_calling_init_multiple_times)
{
    fcc_status_e status;

    // Potentially, initializing multiple times should make no harm
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

TEST(key_config_manager, kcm_calling_kcm_services_before_init)
{
    fcc_status_e fcc_status;
    kcm_status_e kcm_status;
    test_kcm_item_data_template_s kcm_vector;

    fcc_status = fcc_finalize();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    // Calling any KCM API should fail since fcc_init() should firstly called
    kcm_vector = test_kcm_vector[0];
    kcm_status = kcm_item_store(NULL, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, true,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
    TEST_ASSERT(kcm_status == KCM_STATUS_NOT_INITIALIZED);

    // Finalizing multiple times are not allowed either
    fcc_status = fcc_finalize();
    TEST_ASSERT(fcc_status == FCC_STATUS_NOT_INITIALIZED);

    // Init KCM
    fcc_status = fcc_init();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    // process some data on - should succeed
    kcm_test_store_data();
    TEST_SKIP_EXECUTION_ON_FAILURE();

    // tear down will finalize FCC
}

TEST(key_config_manager, kcm_store_positive)
{
    kcm_test_store_data();
    TEST_SKIP_EXECUTION_ON_FAILURE();
}


/**
* =====   Input Parameters Validity  ======
*/
TEST(key_config_manager, kcm_param_validation)
{
    kcm_status_e status;
    size_t size;
    uint8_t buffer[100];
    size_t act_key_size;
    uint32_t i;
    test_kcm_item_data_template_s kcm_vector;
    bool kcm_is_factory = true;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        SA_PV_LOG_INFO("Param validation test loop %" PRIu32 "", i);

        status = kcm_item_store(NULL, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_store(kcm_vector.kcm_item_name, 0, kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                NULL, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data_size(NULL, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, 0, kcm_vector.kcm_item_type,
                                        &size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data(NULL, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   buffer, kcm_vector.kcm_item_data_size, &act_key_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, 0, kcm_vector.kcm_item_type,
                                   buffer, kcm_vector.kcm_item_data_size, &act_key_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   NULL, kcm_vector.kcm_item_data_size, &act_key_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   buffer, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_delete(NULL, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);

        status = kcm_item_delete(kcm_vector.kcm_item_name, 0, kcm_vector.kcm_item_type);
        TEST_ASSERT_TRUE(status == KCM_STATUS_INVALID_PARAMETER);
    }
}

/**
* Get/Get size/Delete items that weren't stored
*/
TEST(key_config_manager, kcm_get_delete_negative)
{
    kcm_status_e status;
    size_t size;
    size_t act_kcm_item_data_size;
    uint32_t i;
    test_kcm_item_data_template_s kcm_vector;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        SA_PV_LOG_INFO("Get negative test loop %" PRIu32 "", i);

        uint8_t expected_kcm_item_data[kcm_vector.kcm_item_data_size];

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
        TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &size);
        TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status);

        status = kcm_item_delete(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type);
        TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);
    }
}

/**
* =====   File Prefixes  ======
*/
TEST(key_config_manager, kcm_read_names_with_prefix)
{
    uint32_t i, j;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    size_t act_kcm_item_data_size;
    char file_complete_name[TEST_KCM_FILE_PATH_MAX_LEN];
    size_t file_complete_name_length;
    kcm_ctx_s *ctx = NULL; // FIXME - Currently not implemented

    /* First write all files from test vector  */
    kcm_test_store_data();
    TEST_SKIP_EXECUTION_ON_FAILURE();

    /* Get complete filenames */
    /* Try to read files with wrong prefix and fail*/
    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        for (j = 0; j < g_size_of_test_kcm_vector; j++) {
            uint8_t expected_kcm_item_data[test_kcm_vector[j].kcm_item_data_size];

            /* Check the conventional way in kcm_store_positive test. Here we access the file with complete file name*/
            if (i == j) {
                strcpy(file_complete_name, test_kcm_vector[j].kcm_prefix);
                strcat(file_complete_name, (char *)test_kcm_vector[j].kcm_item_name);
                file_complete_name_length = strlen(file_complete_name);

                status = storage_file_size_get(ctx, (const uint8_t *)file_complete_name, file_complete_name_length, &act_kcm_item_data_size);
                TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
                TEST_ASSERT_EQUAL(test_kcm_vector[j].kcm_item_data_size, act_kcm_item_data_size);

                status = storage_file_read(ctx, (const uint8_t *)file_complete_name, file_complete_name_length, expected_kcm_item_data, test_kcm_vector[j].kcm_item_data_size, &act_kcm_item_data_size);
                TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
                TEST_ASSERT_EQUAL_MEMORY_MESSAGE(test_kcm_vector[j].kcm_item_data, expected_kcm_item_data, test_kcm_vector[j].kcm_item_data_size, "Inconsistent kcm data");

            } else {

                /* Try to get size and fail because no file with same name and prefix is stored */
                status = kcm_item_get_data_size(test_kcm_vector[j].kcm_item_name, strlen((char*)test_kcm_vector[j].kcm_item_name), test_kcm_vector[j].kcm_item_type, &expected_kcm_item_data_size);
                //FIXME - should be TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status)
                TEST_ASSERT_NOT_EQUAL(KCM_STATUS_ERROR, status);

                /* Try to get data and fail because no file with same name and prefix is stored */
                status = kcm_item_get_data(test_kcm_vector[j].kcm_item_name, strlen((char*)test_kcm_vector[j].kcm_item_name), test_kcm_vector[j].kcm_item_type, expected_kcm_item_data, sizeof(expected_kcm_item_data), &act_kcm_item_data_size);
                //FIXME - should be TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status)
                TEST_ASSERT_NOT_EQUAL(KCM_STATUS_ERROR, status);
            }
        }
    }
}

TEST(key_config_manager, store_delete_twice)
{
    uint32_t i;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    test_kcm_item_data_template_s kcm_vector;
    size_t act_kcm_item_data_size;
    bool kcm_is_factory = true;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        uint8_t expected_kcm_item_data[kcm_vector.kcm_item_data_size];

        SA_PV_LOG_INFO("Store test loop %" PRIu32 "", i);

        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_FILE_EXIST);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &expected_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        TEST_ASSERT_EQUAL(kcm_vector.kcm_item_data_size, expected_kcm_item_data_size);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(kcm_vector.kcm_item_data, expected_kcm_item_data, kcm_vector.kcm_item_data_size, "Inconsistent kcm data");

        status = kcm_item_delete(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        status = kcm_item_delete(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type);
        TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &expected_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);
    }
}

TEST(key_config_manager, kcm_empty_file)
{
    uint32_t i;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    size_t act_kcm_item_data_size;
    bool kcm_is_factory = true;
    //Just some useless buffer. We don't write it anyways.
    uint8_t cfg_data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    test_kcm_item_data_template_w_expected_s kcm_empty_vector[] = {
        {
            .item = {
                .kcm_item_name = (uint8_t*)"config_param1",
                .kcm_item_data = cfg_data,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_CONFIG_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_CONFIG_PARAM
            },

            .status_expected = KCM_STATUS_SUCCESS
        },

        {
            .item = {
                .kcm_item_name = (uint8_t*)"config_param2",
                .kcm_item_data = NULL,
                .kcm_item_data_size = sizeof(cfg_data),
                .kcm_item_type = KCM_CONFIG_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_CONFIG_PARAM
            },

            .status_expected = KCM_STATUS_INVALID_PARAMETER
        },

        {
            .item = {
                .kcm_item_name = (uint8_t*)"config_param3",
                .kcm_item_data = NULL,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_CONFIG_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_CONFIG_PARAM
            },

            .status_expected = KCM_STATUS_SUCCESS
        },
        {
            .item = {
                .kcm_item_name = (uint8_t*)"prv_key",
                .kcm_item_data = NULL,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_PRIVATE_KEY_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_PRIVATE_KEY
            },

            .status_expected = KCM_STATUS_ITEM_IS_EMPTY
        },
        {
            .item = {
                .kcm_item_name = (uint8_t*)"pub_key",
                .kcm_item_data = testdata_pub_ecc_key1der,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_PUBLIC_KEY_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_PUBLIC_KEY
            },

            .status_expected = KCM_STATUS_ITEM_IS_EMPTY
        },
        {
            .item = {
                .kcm_item_name = (uint8_t*)"sym_key",
                .kcm_item_data = NULL,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_SYMMETRIC_KEY_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_SYMMETRIC_KEY
            },
            //Currently possible to write a symmetric key of size 0 since we do not check format
            .status_expected = KCM_STATUS_ITEM_IS_EMPTY
        },
        {
            .item = {
                .kcm_item_name = (uint8_t*)"crt",
                .kcm_item_data = NULL,
                .kcm_item_data_size = 0,
                .kcm_item_type = KCM_CERTIFICATE_ITEM,
                .kcm_prefix = KCM_FILE_PREFIX_CERTIFICATE
            },

            .status_expected = KCM_STATUS_ITEM_IS_EMPTY
        }
    };

    size_t kcm_empty_vector_size = sizeof(kcm_empty_vector) / sizeof(test_kcm_item_data_template_w_expected_s);

    for (i = 0; i < kcm_empty_vector_size; i++) {
        SA_PV_LOG_INFO("kcm_empty_file test loop %" PRIu32 "", i);

        status = kcm_item_store(kcm_empty_vector[i].item.kcm_item_name, strlen((char*)kcm_empty_vector[i].item.kcm_item_name), kcm_empty_vector[i].item.kcm_item_type, kcm_is_factory,
                                kcm_empty_vector[i].item.kcm_item_data, kcm_empty_vector[i].item.kcm_item_data_size, NULL);

        TEST_ASSERT_TRUE(status == kcm_empty_vector[i].status_expected);

        if (status == KCM_STATUS_SUCCESS) {
            status = kcm_item_get_data_size(kcm_empty_vector[i].item.kcm_item_name, strlen((char*)kcm_empty_vector[i].item.kcm_item_name), kcm_empty_vector[i].item.kcm_item_type,
                                            &expected_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == kcm_empty_vector[i].status_expected);
            TEST_ASSERT_EQUAL(kcm_empty_vector[i].item.kcm_item_data_size, expected_kcm_item_data_size);
            status = kcm_item_get_data(kcm_empty_vector[i].item.kcm_item_name, strlen((char*)kcm_empty_vector[i].item.kcm_item_name), kcm_empty_vector[i].item.kcm_item_type,
                                       NULL, kcm_empty_vector[i].item.kcm_item_data_size, &act_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
            TEST_ASSERT_EQUAL(kcm_empty_vector[i].item.kcm_item_data_size, expected_kcm_item_data_size);
        }
    }
}

TEST(key_config_manager, kcm_factory_reset_basic)
{
    uint32_t i;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    test_kcm_item_data_template_s kcm_vector;
    size_t act_kcm_item_data_size;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        uint8_t expected_kcm_item_data[kcm_vector.kcm_item_data_size];

        SA_PV_LOG_INFO("test loop %" PRIu32 "", i);

        //store non factory item
        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, false,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        //perform factory reset
        status = kcm_factory_reset();
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        //verify that the item is deleted
        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &expected_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

        //store factory item
        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, true,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        //perform factory reset
        status = kcm_factory_reset();
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        //get the data and verify that it is the same as stored
        status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                        &expected_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        TEST_ASSERT_EQUAL(kcm_vector.kcm_item_data_size, expected_kcm_item_data_size);

        status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                   expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(kcm_vector.kcm_item_data, expected_kcm_item_data, kcm_vector.kcm_item_data_size, "Inconsistent kcm data");
    }
}


TEST(key_config_manager, kcm_factory_reset_advanced)
{
    uint32_t i;
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    test_kcm_item_data_template_s kcm_vector;
    size_t act_kcm_item_data_size;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        SA_PV_LOG_INFO("test loop %" PRIu32 "", i);

        if ((i % 2) == 0) {
            //store every even item as factory
            status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, true,
                                    kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
            TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        } else {
            // store every odd item as  non factory item
            status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, false,
                                    kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
            TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
        }
    }

    //perform factory reset
    status = kcm_factory_reset();
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);


    for (i = 0; i < g_size_of_test_kcm_vector; i++) {

        kcm_vector = test_kcm_vector[i];
        uint8_t expected_kcm_item_data[kcm_vector.kcm_item_data_size];

        SA_PV_LOG_INFO("test loop %" PRIu32 "", i);

        if ((i % 2) == 0) {
            //get the data and verify that it is the same as stored
            status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                            &expected_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
            TEST_ASSERT_EQUAL(kcm_vector.kcm_item_data_size, expected_kcm_item_data_size);

            status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                       expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(kcm_vector.kcm_item_data, expected_kcm_item_data, kcm_vector.kcm_item_data_size, "Inconsistent kcm data");
        } else {
            //verify that the data does not exist any more
            status = kcm_item_get_data_size(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                            &expected_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

            status = kcm_item_get_data(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type,
                                       expected_kcm_item_data, kcm_vector.kcm_item_data_size, &act_kcm_item_data_size);
            TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);
        }
    }
}


TEST(key_config_manager, kcm_factory_reset_w_delete)
{
    kcm_status_e status;
    size_t expected_kcm_item_data_size;
    size_t act_kcm_item_data_size;
    uint8_t expected_kcm_item_data[test_kcm_vector[0].kcm_item_data_size];

    //store factory item
    status = kcm_item_store(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type, true,
                            test_kcm_vector[0].kcm_item_data, test_kcm_vector[0].kcm_item_data_size, NULL);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

    //store non factory item with same name, but different type
    status = kcm_item_store(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[1].kcm_item_type, false,
                            test_kcm_vector[1].kcm_item_data, test_kcm_vector[1].kcm_item_data_size, NULL);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

    //try to delete the  factory data
    status = kcm_item_delete(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

    //try to delete the non factory data
    status = kcm_item_delete(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[1].kcm_item_type);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

    //try to delete the non factory data again
    status = kcm_item_delete(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[1].kcm_item_type);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

    //delete the factory data
    status = kcm_item_delete(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

    //try to read the data
    status = kcm_item_get_data_size(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type,
                                    &expected_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

    status = kcm_item_get_data(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type,
                               expected_kcm_item_data, test_kcm_vector[0].kcm_item_data_size, &act_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

    //perform factory reset
    status = kcm_factory_reset();
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

    //read the factory data
    status = kcm_item_get_data_size(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type,
                                    &expected_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
    TEST_ASSERT_EQUAL(test_kcm_vector[0].kcm_item_data_size, expected_kcm_item_data_size);

    status = kcm_item_get_data(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type,
                               expected_kcm_item_data, test_kcm_vector[0].kcm_item_data_size, &act_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(test_kcm_vector[0].kcm_item_data, expected_kcm_item_data, test_kcm_vector[0].kcm_item_data_size, "Inconsistent kcm data");

    //check that the non factory data was not restored
    status = kcm_item_get_data_size(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[1].kcm_item_type,
                                    &expected_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);

    status = kcm_item_get_data(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[1].kcm_item_type,
                               expected_kcm_item_data, test_kcm_vector[0].kcm_item_data_size, &act_kcm_item_data_size);
    TEST_ASSERT_TRUE(status == KCM_STATUS_ITEM_NOT_FOUND);
}


TEST(key_config_manager, kcm_validate_esfs_encryption_bit)
{
    uint32_t i;
    kcm_status_e status;
    test_kcm_item_data_template_s kcm_vector;
    bool kcm_is_factory = false;
    char file_complete_name[TEST_KCM_FILE_PATH_MAX_LEN];
    uint32_t file_complete_name_length;
    bool encryption_status;

    for (i = 0; i < g_size_of_test_kcm_vector; i++) {
        kcm_vector = test_kcm_vector[i];

        SA_PV_LOG_INFO("test loop %" PRIu32 "", i);

        //store non factory item
        status = kcm_item_store(kcm_vector.kcm_item_name, strlen((char*)kcm_vector.kcm_item_name), kcm_vector.kcm_item_type, kcm_is_factory,
                                kcm_vector.kcm_item_data, kcm_vector.kcm_item_data_size, NULL);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        strcpy(file_complete_name, kcm_vector.kcm_prefix);
        strcat(file_complete_name, (char *)kcm_vector.kcm_item_name);
        file_complete_name_length = strlen(file_complete_name);

        status = check_encryption_settings((const uint8_t*)file_complete_name, file_complete_name_length, &encryption_status);
        TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);

        if (kcm_vector.kcm_item_type != KCM_CERTIFICATE_ITEM) {
            TEST_ASSERT_EQUAL(encryption_status, true);
        } else {   //KCM_CERTIFICATE_ITEM
            TEST_ASSERT_EQUAL(encryption_status, false);
        }
    }
}

TEST(key_config_manager, kcm_store_invalid_key)
{
    kcm_status_e status;

    // ECPrivate Key - Store factory item with invalid item size (one byte more)
    status = kcm_item_store(test_kcm_vector[0].kcm_item_name, strlen((char*)test_kcm_vector[0].kcm_item_name), test_kcm_vector[0].kcm_item_type, true,
                            test_kcm_vector[0].kcm_item_data, (test_kcm_vector[0].kcm_item_data_size + 1), NULL);
    TEST_ASSERT_TRUE(status != KCM_STATUS_SUCCESS);

    // ECPublic Key - Store factory item with invalid item size (one byte more)
    status = kcm_item_store(test_kcm_vector[1].kcm_item_name, strlen((char*)test_kcm_vector[1].kcm_item_name), test_kcm_vector[1].kcm_item_type, true,
                            test_kcm_vector[1].kcm_item_data, (test_kcm_vector[1].kcm_item_data_size + 1), NULL);
    TEST_ASSERT_TRUE(status != KCM_STATUS_SUCCESS);
}

TEST(key_config_manager, kcm_store_invalid_certificate)
{
    kcm_status_e status;

    // Store factory item with invalid item size (one byte more)
    status = kcm_item_store(test_kcm_vector[3].kcm_item_name, strlen((char*)test_kcm_vector[3].kcm_item_name), test_kcm_vector[3].kcm_item_type, true,
                            test_kcm_vector[3].kcm_item_data, (test_kcm_vector[3].kcm_item_data_size + 1), NULL);
    TEST_ASSERT_TRUE(status != KCM_STATUS_SUCCESS);

}

TEST_GROUP_RUNNER(key_config_manager)
{
    // FIXME - before running those couple of test issue IOTPAL-478 should be resolved.
    //RUN_TEST_CASE(key_config_manager, kcm_store_invalid_key);
    //RUN_TEST_CASE(key_config_manager, kcm_store_invalid_certificate);

    RUN_TEST_CASE(key_config_manager, kcm_calling_init_multiple_times);
    RUN_TEST_CASE(key_config_manager, kcm_calling_kcm_services_before_init);
    RUN_TEST_CASE(key_config_manager, kcm_store_positive);
    RUN_TEST_CASE(key_config_manager, kcm_param_validation);
    RUN_TEST_CASE(key_config_manager, kcm_get_delete_negative);
    RUN_TEST_CASE(key_config_manager, kcm_read_names_with_prefix);
    RUN_TEST_CASE(key_config_manager, store_delete_twice);
    RUN_TEST_CASE(key_config_manager, kcm_empty_file);
    RUN_TEST_CASE(key_config_manager, kcm_factory_reset_basic);
    RUN_TEST_CASE(key_config_manager, kcm_factory_reset_advanced);
    RUN_TEST_CASE(key_config_manager, kcm_factory_reset_w_delete);
    RUN_TEST_CASE(key_config_manager, kcm_validate_esfs_encryption_bit);
}

