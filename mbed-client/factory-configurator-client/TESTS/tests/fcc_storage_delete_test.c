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
#include "testdata.h"
#include "pv_log.h"
#include "pv_macros.h"
#include "kcm_file_prefix_defs.h"

extern test_kcm_item_data_template_s test_kcm_vector[];




static void check_data(test_kcm_item_data_template_s *kcm_vector, kcm_status_e expected_get_size_status, kcm_status_e expected_get_data_status)
{
    kcm_status_e kcm_status;
    size_t expected_kcm_item_data_size;
    size_t act_kcm_item_data_size;
    uint8_t expected_kcm_item_data[kcm_vector->kcm_item_data_size];

    //Check that the item stored by getting it size and data
    kcm_status = kcm_item_get_data_size(kcm_vector->kcm_item_name, strlen((char*)kcm_vector->kcm_item_name), kcm_vector->kcm_item_type,
                                        &expected_kcm_item_data_size);
    TEST_ASSERT_TRUE(kcm_status == expected_get_size_status);
    if (kcm_status == KCM_STATUS_SUCCESS) {
        TEST_ASSERT_EQUAL(kcm_vector->kcm_item_data_size, expected_kcm_item_data_size);
    }

    kcm_status = kcm_item_get_data(kcm_vector->kcm_item_name, strlen((char*)kcm_vector->kcm_item_name), kcm_vector->kcm_item_type,
                                   expected_kcm_item_data, kcm_vector->kcm_item_data_size, &act_kcm_item_data_size);
    TEST_ASSERT_TRUE(kcm_status == expected_get_data_status);
    if (kcm_status == KCM_STATUS_SUCCESS) {
        //Compare returned data with the actual data
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(kcm_vector->kcm_item_data, expected_kcm_item_data, kcm_vector->kcm_item_data_size, "Inconsistent kcm data");
    }

}
static void store_and_check_data(test_kcm_item_data_template_s *kcm_vector, kcm_status_e expected_store_status, kcm_status_e expected_get_size_status, kcm_status_e expected_get_data_status)
{
    kcm_status_e kcm_status;

    //Save an item to storage
    kcm_status = kcm_item_store(kcm_vector->kcm_item_name, strlen((char*)kcm_vector->kcm_item_name), kcm_vector->kcm_item_type, true,
                                kcm_vector->kcm_item_data, kcm_vector->kcm_item_data_size, NULL);
    TEST_ASSERT_TRUE(kcm_status == expected_store_status);


    check_data(kcm_vector, expected_get_size_status, expected_get_data_status);
    TEST_SKIP_EXECUTION_ON_FAILURE();
}
TEST_GROUP(fcc_storage_delete_test);


TEST_SETUP(fcc_storage_delete_test)
{
    fcc_status_e status;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    status = fcc_storage_delete();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

TEST_TEAR_DOWN(fcc_storage_delete_test)
{
    fcc_status_e status;

    status = fcc_storage_delete();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}


TEST(fcc_storage_delete_test, fcc_storage_delete_one_item)
{
    fcc_status_e fcc_status;
    test_kcm_item_data_template_s kcm_vector;

    kcm_vector = test_kcm_vector[0];

    //Save an item to storage
    store_and_check_data(&kcm_vector,KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store the same item again => should fail only in store
    store_and_check_data(&kcm_vector, KCM_STATUS_FILE_EXIST, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    check_data(&kcm_vector, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store and check the same item again, after the storage was deleted
    store_and_check_data(&kcm_vector, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
}

TEST(fcc_storage_delete_test, fcc_storage_delete_multiple_items)
{
    fcc_status_e fcc_status;

    test_kcm_item_data_template_s kcm_vector_1 = test_kcm_vector[0];
    test_kcm_item_data_template_s kcm_vector_2 = test_kcm_vector[1];
    test_kcm_item_data_template_s kcm_vector_3 = test_kcm_vector[2];

    //Save first item to storage
    store_and_check_data(&kcm_vector_1, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    //Save second item to storage
    store_and_check_data(&kcm_vector_2, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    //Save third item to storage
    store_and_check_data(&kcm_vector_3, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store the same items again => should fail only in store
    store_and_check_data(&kcm_vector_1, KCM_STATUS_FILE_EXIST, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    store_and_check_data(&kcm_vector_2, KCM_STATUS_FILE_EXIST, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    store_and_check_data(&kcm_vector_3, KCM_STATUS_FILE_EXIST, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();


    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Check that all items were deleted
    check_data(&kcm_vector_1, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_2, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_3, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store the second item again
    store_and_check_data(&kcm_vector_2, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Check that all items were deleted
    check_data(&kcm_vector_1, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_2, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_3, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store the first and third items again
    store_and_check_data(&kcm_vector_1, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    store_and_check_data(&kcm_vector_3, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Check that second item deleted
    check_data(&kcm_vector_2, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Try to store the second item again
    store_and_check_data(&kcm_vector_2, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();


    //Check that all items were deleted
    check_data(&kcm_vector_1, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_2, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_3, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    TEST_SKIP_EXECUTION_ON_FAILURE();

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Check that all items were deleted
    check_data(&kcm_vector_1, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_2, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
    check_data(&kcm_vector_3, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);
    TEST_SKIP_EXECUTION_ON_FAILURE();
}

TEST(fcc_storage_delete_test, fcc_storage_delete_multiple_times)
{
    fcc_status_e fcc_status;
    test_kcm_item_data_template_s kcm_vector;

    kcm_vector = test_kcm_vector[0];

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    //Save an item to storage
    store_and_check_data(&kcm_vector, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);

    //Try to store the same item again => should fail only in store
    store_and_check_data(&kcm_vector, KCM_STATUS_FILE_EXIST, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);

    //Delete storage data
    fcc_status = fcc_storage_delete();
    TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

    check_data(&kcm_vector, KCM_STATUS_ITEM_NOT_FOUND, KCM_STATUS_ITEM_NOT_FOUND);

    //Try to store and check the same item again, after the storage was deleted
    store_and_check_data(&kcm_vector, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);

}

TEST(fcc_storage_delete_test, fcc_storage_delete_repeat)
{
    fcc_status_e fcc_status;
    test_kcm_item_data_template_s kcm_vector;
    int i = 0;
    int num_of_test_iterations = 15;
    kcm_vector = test_kcm_vector[0];

    for (i = 0; i < num_of_test_iterations; i++) {
        //Delete storage data
        fcc_status = fcc_storage_delete();
        TEST_ASSERT(fcc_status == FCC_STATUS_SUCCESS);

        //Save an item to storage
        store_and_check_data(&kcm_vector, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS, KCM_STATUS_SUCCESS);
    }
}

TEST_GROUP_RUNNER(fcc_storage_delete_test)
{
    RUN_TEST_CASE(fcc_storage_delete_test, fcc_storage_delete_one_item);
    RUN_TEST_CASE(fcc_storage_delete_test, fcc_storage_delete_multiple_items);
    RUN_TEST_CASE(fcc_storage_delete_test, fcc_storage_delete_multiple_times);
    RUN_TEST_CASE(fcc_storage_delete_test, fcc_storage_delete_repeat);
}
