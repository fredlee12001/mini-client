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
#include "pv_log.h"
#include "pv_macros.h"
#include "fcc_output_info_handler.h"


TEST_GROUP(fcc_developer_flow_test);

TEST_SETUP(fcc_developer_flow_test)
{
    fcc_status_e status;

    //init store
    status = fcc_init();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    status = fcc_storage_delete();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

TEST_TEAR_DOWN(fcc_developer_flow_test)
{
    fcc_status_e status;

    status = fcc_storage_delete();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    //finalize store
    status = fcc_finalize();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}


TEST(fcc_developer_flow_test, fcc_dev_flow_mandatory_items)
{
    fcc_status_e status;
    fcc_output_info_s *output_info = NULL;

    status = fcc_developer_flow();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    status = fcc_verify_device_configured_4mbed_cloud();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    output_info = fcc_get_error_and_warning_data( );
    TEST_ASSERT(output_info != NULL);
    //error_string_info should  be empty
    TEST_ASSERT(output_info->error_string_info == NULL);
    //optional parameters are missing ->head_of_warning_list should not be empty
    TEST_ASSERT(output_info->head_of_warning_list != NULL);
}

TEST(fcc_developer_flow_test, fcc_dev_flow_multiple)
{
    fcc_status_e status;

    status = fcc_developer_flow();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);

    //should fail , can not save the same files twice
    status = fcc_developer_flow();
    TEST_ASSERT(status == FCC_STATUS_KCM_FILE_EXIST_ERROR);

    status = fcc_verify_device_configured_4mbed_cloud();
    TEST_ASSERT(status == FCC_STATUS_SUCCESS);
}

TEST(fcc_developer_flow_test, fcc_dev_flow_after_storage_reset)
{
    fcc_status_e status;
    int i ;

    for (i = 0; i < 3; i++) {
        status = fcc_developer_flow();
        TEST_ASSERT_EQUAL(status, FCC_STATUS_SUCCESS);

        status = fcc_storage_delete();
        TEST_ASSERT_EQUAL(status, FCC_STATUS_SUCCESS);
    }

    status = fcc_developer_flow();
    TEST_ASSERT_EQUAL(status, FCC_STATUS_SUCCESS);

    status = fcc_verify_device_configured_4mbed_cloud();
    TEST_ASSERT_EQUAL(status, FCC_STATUS_SUCCESS);
}

TEST_GROUP_RUNNER(fcc_developer_flow_test)
{
    //positive test
    RUN_TEST_CASE(fcc_developer_flow_test, fcc_dev_flow_mandatory_items);
    RUN_TEST_CASE(fcc_developer_flow_test, fcc_dev_flow_multiple);
    RUN_TEST_CASE(fcc_developer_flow_test, fcc_dev_flow_after_storage_reset);
}
