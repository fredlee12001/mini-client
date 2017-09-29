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


#include "general_tests_runner.h"
#include "unity_fixture.h"

void RunAllGeneralTests(void)
{
    RUN_TEST_GROUP(UnitySelftest);
    RUN_TEST_GROUP(Storage);
    RUN_TEST_GROUP(key_config_manager);
    RUN_TEST_GROUP(fcc_verify_device_4mbed_test);
    RUN_TEST_GROUP(fcc_verify_device_4mbed_python_certs_test);
    RUN_TEST_GROUP(fcc_bundle_handler_test);
    RUN_TEST_GROUP(fcc_storage_delete_test);
    RUN_TEST_GROUP(fcc_developer_flow_test);
    RUN_TEST_GROUP(fcc_output_info_handler_test);
    RUN_TEST_GROUP(crypto_service);
}
