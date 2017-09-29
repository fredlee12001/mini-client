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

#ifndef __FCC_TEST_COMMON_HELPERS_H__
#define __FCC_TEST_COMMON_HELPERS_H__

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "key_config_manager.h"
#include "fcc_status.h"

#define CERTIFICATE_TEST_ITERATIONS 3
#define NUMBER_OF_CERTIFICATES_IN_CHAIN  5
typedef struct test_kcm_item_data_template_ {
    const uint8_t *kcm_item_name;
    const uint8_t *kcm_item_data;
    size_t kcm_item_data_size;
    kcm_item_type_e kcm_item_type;
    const char *kcm_prefix;
} test_kcm_item_data_template_s;

typedef struct test_certificate_chain_data_ {
    const uint8_t *certificate;
    size_t certificate_size;
} test_certificate_chain_data_s;

typedef struct test_kcm_item_data_template_w_expected_ {

    test_kcm_item_data_template_s item;
    kcm_status_e status_expected;
} test_kcm_item_data_template_w_expected_s;

uint32_t tst_rand_int(uint32_t min, uint32_t max);
void tst_rand_string(char *string_out, size_t string_length);
bool tst_create_different_string(const char *orig_string, char *new_string);
kcm_status_e check_encryption_settings(const uint8_t *file_name, size_t file_name_length, bool *encryption_status);
/** The function checks response message status.
*
* The functions parses response message, checks response status number, stataus string info and failed item name.
* uint8_t *response_message[in] -   the response message
* size_t size_of_response_message[in] - the size of response message
* fcc_status_e expected_status[in] - expected status number
* uint8_t *expected_failed_item_name - expected name of failed item
* @return
*       true/false
*/
bool tst_check_fcc_output_info(uint8_t *response_message,
                               size_t size_of_response_message,
                               fcc_status_e expected_status,
                               const char *error_string,
                               const char *expected_failed_item_name,
                               const char *expected_warning_string);
#ifdef PV_TEST_REPRODUCE_RANDOM_STATE
//The function TestSetReproducedSeed() should be run only if we want to set fixed seed to pseudo random function
bool tst_set_reproduced_seed(void);
#endif

#endif  // __FCC_TEST_COMMON_HELPERS_H__

