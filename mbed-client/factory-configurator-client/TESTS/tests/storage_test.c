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
#include "fcc_test_common_helpers.h"
#include "pv_log.h"
#include "pv_macros.h"
#include "pv_error_handling.h"
#include "storage.h"
#include "esfs.h"


const uint8_t data1[] = { 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0x11, 0x22 };
const uint8_t data2[] = { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
kcm_ctx_s *kcm_ctx = NULL; // FIXME - currently not implemented



TEST_GROUP(Storage);

TEST_SETUP(Storage)
{
    // Make sure storage is clean...
    kcm_status_e status = storage_reset();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_init();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST_TEAR_DOWN(Storage)
{
    // Clean at end of each test (although we also cleaning storage at setup)
    kcm_status_e status = storage_reset();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_finalize();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST(Storage, check_storage_persist_after_init)
{
    kcm_status_e status;
    // byte array equals to 'testfile'
    const uint8_t file_name[] = {0x74,0x65,0x73,0x74,0x66,0x69,0x6c,0x65};
    size_t file_size = 0;

    status = storage_reset();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data1, sizeof(data1), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_init();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data1), file_size);
}

TEST(Storage, check_storage_cleared_after_reset)
{
    kcm_status_e status;
    // byte array equals to 'testfile'
    const uint8_t file_name[] = {0x74,0x65,0x73,0x74,0x66,0x69,0x6c,0x65};
    size_t file_size = 0;

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data1, sizeof(data1), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_reset();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST(Storage, query_nonexisting_file_size)
{
    kcm_status_e status;
    // byte array equals to 'wont-find-me-file-name'
    const uint8_t file_name[] = {0x77,0x6f,0x6e,0x74,0x2d,0x66,0x69,0x6e,0x64,0x2d,0x6d,0x65,0x2d,0x66,0x69,0x6c,0x65,0x2d,0x6e,0x61,0x6d,0x65};
    size_t file_size = 0;

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(0, file_size);
}

TEST(Storage, write_read_verify)
{
    kcm_status_e status;
    // byte array equals to 'testfile'
    const uint8_t file_name[] = {0x74,0x65,0x73,0x74,0x66,0x69,0x6c,0x65};
    size_t file_size = 0;
    uint8_t *buffer = NULL;
    size_t buffer_actual_read = 0;

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data1, sizeof(data1), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data1), file_size);

    buffer = malloc(file_size);
    TEST_ASSERT_NOT_NULL(buffer);

    // supply insufficient buffer size - expect an error
    status = storage_file_read(kcm_ctx, file_name, sizeof(file_name), buffer, (file_size - 1), &buffer_actual_read);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);

    status = storage_file_read(kcm_ctx, file_name, sizeof(file_name), buffer, file_size, &buffer_actual_read);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data1), buffer_actual_read);

    // verify same data returned
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data1, buffer, sizeof(data1));

    // clear buffer
    buffer_actual_read = 0;
    memset(buffer, 0, file_size);

    // Read again to check robustness
    status = storage_file_read(kcm_ctx, file_name, sizeof(file_name), buffer, file_size, &buffer_actual_read);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data1), buffer_actual_read);

    // verify same data returned (again)
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data1, buffer, sizeof(data1));

    free(buffer);

    // try to re-write with same file name but different data - should fail
    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data2, sizeof(data2), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_FILE_EXIST, status);
}

TEST(Storage, write_delete_verify)
{
    kcm_status_e status;
    // byte array equals to 'delete-this-file'
    const uint8_t file_name[] = {0x64,0x65,0x6c,0x65,0x74,0x65,0x2d,0x74,0x68,0x69,0x73,0x2d,0x66,0x69,0x6c,0x65};
    size_t file_size = 0;

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data2, sizeof(data2), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data2), file_size);

    status = storage_file_delete(kcm_ctx, file_name, sizeof(file_name));
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    // second time should prompt an error
    status = storage_file_delete(kcm_ctx, file_name, sizeof(file_name));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);

    // try to query size - should fail
    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);

    // re-creating same file should be okay
    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data2, sizeof(data2), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST(Storage, read_not_exist_file)
{
    kcm_status_e status;
    // byte array equals to 'testfile'
    const uint8_t file_name[] = { 0x74,0x65,0x73,0x74,0x66,0x69,0x6c,0x65 };
    const uint8_t file_name_not_exist[] = { 0x66,0x69,0x6c,0x65, 0x74,0x65};
    uint8_t data_to_read[50];

    size_t file_size = 0;

    status = storage_file_read(kcm_ctx, file_name_not_exist, sizeof(file_name_not_exist), data_to_read, sizeof(data_to_read), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status);

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data1, sizeof(data1), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_delete(kcm_ctx, file_name, sizeof(file_name));
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_read(kcm_ctx, file_name, sizeof(file_name), data_to_read, sizeof(data_to_read), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status);

    status = storage_file_write(kcm_ctx, file_name, sizeof(file_name), data1, sizeof(data1), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = storage_file_size_get(kcm_ctx, file_name, sizeof(file_name), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL(sizeof(data1), file_size);
}

TEST(Storage, empty_file)
{
    kcm_status_e status;
    uint8_t data_to_write[] = "data to write";
    uint8_t file_names[][2] = { { 0x00, 0x00 },{ 0x01, 0x00 },{ 0x00, 0x01 },{ 0x01, 0x01 } };
    uint8_t *data[] = {NULL, data_to_write, NULL, data_to_write};
    uint32_t size[] = { 0, 0, sizeof(data_to_write), sizeof(data_to_write) };
    kcm_status_e expected_status[] = { KCM_STATUS_SUCCESS,  KCM_STATUS_SUCCESS, KCM_STATUS_INVALID_PARAMETER, KCM_STATUS_SUCCESS };
    uint8_t data_to_read[sizeof(data_to_write)];
    size_t file_size = 0;
    uint32_t i, number_of_files;

    number_of_files = sizeof(expected_status) / sizeof(kcm_status_e);

    for (i = 0; i < number_of_files; i++) {
        status = storage_file_write(kcm_ctx, file_names[i], sizeof(file_names[i]), data[i], size[i], false, false);
        TEST_ASSERT_EQUAL(expected_status[i], status);

        if (status == KCM_STATUS_SUCCESS) {
            status = storage_file_read(kcm_ctx, file_names[i], sizeof(file_names[i]), data_to_read, size[i], &file_size);
            TEST_ASSERT_EQUAL(expected_status[i], status);
            TEST_ASSERT_EQUAL(size[i], file_size);

            //Try to read (in some cases empty) file with buffer of size greater than 0 and succeed
            status = storage_file_read(kcm_ctx, file_names[i], sizeof(file_names[i]), data_to_read, sizeof(data_to_read), &file_size);
            TEST_ASSERT_EQUAL(expected_status[i], status);
            TEST_ASSERT_EQUAL(size[i], file_size);

            status = storage_file_size_get(kcm_ctx, file_names[i], sizeof(file_names[i]), &file_size);
            TEST_ASSERT_EQUAL(expected_status[i], status);
            TEST_ASSERT_EQUAL(size[i], file_size);

            status = storage_file_delete(kcm_ctx, file_names[i], sizeof(file_names[i]));
            TEST_ASSERT_EQUAL(expected_status[i], status);
            TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
        }
    }

    // Write non empty file, read with buffer of size 0 and fail.
    uint8_t non_empty_file_name[3] = { 0x01, 0x01, 0x01 }; // Some meaningless new name.

    status = storage_file_write(kcm_ctx, non_empty_file_name, sizeof(non_empty_file_name), data_to_write, sizeof(data_to_write), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    // Read with valid buffer of size 0
    status = storage_file_read(kcm_ctx, non_empty_file_name, sizeof(non_empty_file_name), data_to_read, 0, &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);

    // Read with invalid buffer of size 0
    status = storage_file_read(kcm_ctx, non_empty_file_name, sizeof(non_empty_file_name), NULL, 0, &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);

    // Read with invalid buffer of sufficient size
    status = storage_file_read(kcm_ctx, non_empty_file_name, sizeof(non_empty_file_name), NULL, sizeof(data_to_write), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_INVALID_PARAMETER, status);
}


TEST(Storage, storage_check_factory_reset)
{
    const char factory_file[] = "factory_file";
    const char non_factory_file[] = "non_factory_file";
    const char factory_data[] = "factory_data";
    const char non_factory_data[] = "non_factory_data";

    kcm_status_e status;
    size_t file_size = 0;

    //store factory file
    status = storage_file_write(kcm_ctx, (const uint8_t*)factory_file, strlen(factory_file), (const uint8_t*)factory_data, strlen(factory_data), true, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //store non-factory file
    status = storage_file_write(kcm_ctx, (const uint8_t*)non_factory_file, strlen(non_factory_file), (const uint8_t*)non_factory_data, strlen(non_factory_data), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //get factory file size
    status = storage_file_size_get(kcm_ctx, (const uint8_t*)factory_file, strlen(factory_file), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //get non factory file size
    status = storage_file_size_get(kcm_ctx, (const uint8_t*)non_factory_file, strlen(non_factory_file), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //do factory reset
    status = storage_factory_reset();

    //check that the factory file still exists
    status = storage_file_size_get(kcm_ctx, (const uint8_t*)factory_file, strlen(factory_file), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //check that the factory file does not exits.
    status = storage_file_size_get(kcm_ctx, (const uint8_t*)non_factory_file, strlen(non_factory_file), &file_size);
    TEST_ASSERT_EQUAL(KCM_STATUS_ITEM_NOT_FOUND, status);
}


TEST(Storage, storage_check_encryption_setting)
{
    const char file_to_encrypt[] = "file_to_encrypt";
    const char file_not_for_encryption[] = "file_not_for_encryption";
    const char data_to_encrypt[] = "data_to_encrypt";
    const char data_not_for_encryption[] = "data_not_for_encryption";

    kcm_status_e status;
    bool encryption_status;

    //store file to encrypt
    status = storage_file_write(kcm_ctx, (const uint8_t*)file_to_encrypt, strlen(file_to_encrypt), (const uint8_t*)data_to_encrypt, strlen(data_to_encrypt), false, true);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //store file not for encryption
    status = storage_file_write(kcm_ctx, (const uint8_t*)file_not_for_encryption, strlen(file_not_for_encryption), (const uint8_t*)data_not_for_encryption, strlen(data_not_for_encryption), false, false);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    status = check_encryption_settings((const uint8_t*)file_to_encrypt, strlen(file_to_encrypt), &encryption_status);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
    TEST_ASSERT_EQUAL(true, encryption_status);

    status = check_encryption_settings((const uint8_t*)file_not_for_encryption, strlen(file_not_for_encryption), &encryption_status);
    TEST_ASSERT_TRUE(status == KCM_STATUS_SUCCESS);
    TEST_ASSERT_EQUAL(false, encryption_status);
}


TEST_GROUP_RUNNER(Storage)
{
    RUN_TEST_CASE(Storage, check_storage_persist_after_init);
    RUN_TEST_CASE(Storage, check_storage_cleared_after_reset);
    RUN_TEST_CASE(Storage, query_nonexisting_file_size);
    RUN_TEST_CASE(Storage, write_read_verify);
    RUN_TEST_CASE(Storage, write_delete_verify);
    RUN_TEST_CASE(Storage, read_not_exist_file);
    RUN_TEST_CASE(Storage, empty_file);
    RUN_TEST_CASE(Storage, storage_check_factory_reset);
    RUN_TEST_CASE(Storage, storage_check_encryption_setting);

}
