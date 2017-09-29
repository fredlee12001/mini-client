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

#include "pv_error_handling.h"
#include "storage.h"
#include "esfs.h"

static kcm_status_e error_handler(esfs_result_e esfs_status)
{
    switch (esfs_status) {
        case ESFS_SUCCESS:
            return KCM_STATUS_SUCCESS;
        case ESFS_INVALID_PARAMETER:
            return KCM_STATUS_INVALID_PARAMETER;
        case ESFS_BUFFER_TOO_SMALL:
            return KCM_STATUS_INSUFFICIENT_BUFFER;
        case ESFS_EXISTS:
            return KCM_STATUS_FILE_EXIST;
        case ESFS_NOT_EXISTS:
            return KCM_STATUS_ITEM_NOT_FOUND;
        case ESFS_INVALID_FILE_VERSION:
            return KCM_STATUS_INVALID_FILE_VERSION;
        case ESFS_CMAC_DOES_NOT_MATCH:
            return KCM_STATUS_FILE_CORRUPTED;
        case ESFS_ERROR:
            KCM_STATUS_STORAGE_ERROR;
        case ESFS_HASH_CONFLICT:
            KCM_STATUS_FILE_NAME_CORRUPTED;
        case ESFS_FILE_OPEN_FOR_READ:
        case ESFS_FILE_OPEN_FOR_WRITE:
            KCM_STATUS_INVALID_FILE_ACCESS_MODE;
        default:
            return  KCM_STATUS_UNKNOWN_STORAGE_ERROR;
    }
}


static bool is_file_accessible(const kcm_ctx_s *ctx, esfs_file_t *esfs_file_h)
{
    // FIXME - We need to check file access availability by comparing KCM context TLVs vs the target file header stored in ESFS that contains
    //         TLVs and access rights. In order to retrieve ESFS file TLVs and access rights we should use the following methods
    //         that are currently not implemented:
    //              - esfs_get_meta_data_qty
    //              - esfs_get_meta_data_types
    //              - esfs_get_meta_data_buffer_size
    //              - esfs_read_meta_data
    //              - esfs_get_meta_data_qty

    ctx = ctx;                 // currently unused
    esfs_file_h = esfs_file_h; // currently unused

    return true;
}

kcm_status_e storage_init()
{
    esfs_result_e esfs_status;

    SA_PV_LOG_TRACE_FUNC_ENTER_NO_ARGS();

    esfs_status = esfs_init();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed initializing ESFS (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return KCM_STATUS_SUCCESS;
}

kcm_status_e storage_finalize()
{
    esfs_result_e esfs_status;

    SA_PV_LOG_TRACE_FUNC_ENTER_NO_ARGS();

    esfs_status = esfs_finalize();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed finalizing ESFS (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return KCM_STATUS_SUCCESS;
}

kcm_status_e storage_reset()
{
    esfs_result_e esfs_status;

    SA_PV_LOG_TRACE_FUNC_ENTER_NO_ARGS();

    esfs_status = esfs_reset();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed reset ESFS (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return KCM_STATUS_SUCCESS;
}


kcm_status_e storage_factory_reset()
{
    esfs_result_e esfs_status;

    SA_PV_LOG_TRACE_FUNC_ENTER_NO_ARGS();

    esfs_status = esfs_factory_reset();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed factory reset ESFS (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return KCM_STATUS_SUCCESS;
}

kcm_status_e storage_file_write(const kcm_ctx_s *ctx, const uint8_t *file_name, size_t file_name_length, const uint8_t *data, size_t data_length, bool is_factory, bool is_encrypted)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    esfs_result_e esfs_status;
    esfs_tlv_item_t esfs_meta_data[1];  // FIXME - Unused, yet implemented
    esfs_file_t esfs_file_h;
    uint16_t esfs_flags_bitmap = 0;

    SA_PV_LOG_TRACE_FUNC_ENTER("file_name_length=%" PRIu32 ", data_length=%" PRIu32 "", (uint32_t)file_name_length, (uint32_t)data_length);

    // FIXME - check context is NULL until implemented
    SA_PV_ERR_RECOVERABLE_RETURN_IF((ctx != NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid KCM context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid file name context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name_length == 0), KCM_STATUS_INVALID_PARAMETER, "Got empty file name");
    SA_PV_ERR_RECOVERABLE_RETURN_IF(((data == NULL) && (data_length > 0)), KCM_STATUS_INVALID_PARAMETER, "Provided NULL data buffer and data_length greater than 0");

    // FIXME - currently "ctx" is ignored since there is no logic yet
    if (is_factory) {
        esfs_flags_bitmap |= ESFS_FACTORY_VAL;
    }
    if (is_encrypted) {
        esfs_flags_bitmap |= ESFS_ENCRYPTED;
    }

    esfs_status = esfs_create(file_name, file_name_length, &esfs_meta_data[0], 0, esfs_flags_bitmap, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status == ESFS_EXISTS), KCM_STATUS_FILE_EXIST, "File already exist in ESFS (esfs_status %" PRIu32 ")", (uint32_t)esfs_status);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed creating file (esfs_status %" PRIu32 ")", (uint32_t)esfs_status);

    if (data_length != 0) {
        esfs_status = esfs_write(&esfs_file_h, data, data_length);
        SA_PV_ERR_RECOVERABLE_GOTO_IF((esfs_status != ESFS_SUCCESS), (kcm_status = error_handler(esfs_status)), out, "Failed writing (%" PRIu32 " B) size to file (esfs_status %" PRIu32 ")", (uint32_t)data_length, (uint32_t)esfs_status);
    }

out:
    // Data is only guaranteed to be flushed to the media on efs_close.
    esfs_status = esfs_close(&esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed closing file (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return kcm_status;
}

kcm_status_e storage_file_size_get(const kcm_ctx_s *ctx, const uint8_t *file_name, size_t file_name_length, size_t *file_size_out)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    esfs_result_e esfs_status;
    uint16_t esfs_mode = 0;        // FIXME - Unused, yet implemented
    esfs_file_t esfs_file_h;
    size_t file_size = 0;
    bool success;

    SA_PV_LOG_TRACE_FUNC_ENTER("file_name_length=%" PRIu32 "", (uint32_t)file_name_length);

    // FIXME - check context is NULL until implemented
    SA_PV_ERR_RECOVERABLE_RETURN_IF((ctx != NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid KCM context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid file name context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name_length == 0), KCM_STATUS_INVALID_PARAMETER, "Got empty file name");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_size_out == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid file_size_out pointer");

    esfs_status = esfs_open(file_name, file_name_length, &esfs_mode, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed opening file (esfs_status %d)", esfs_status);

    success = is_file_accessible(ctx, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((!success), (kcm_status = KCM_STATUS_NOT_PERMITTED), out, "Caller has no access rights to the given file");

    esfs_status = esfs_file_size(&esfs_file_h, &file_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((esfs_status != ESFS_SUCCESS), (kcm_status = error_handler(esfs_status)), out, "Failed getting file size (esfs_status %d)", esfs_status);

out:
    esfs_status = esfs_close(&esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed closing file (esfs_status %d)", esfs_status);

    if (kcm_status == KCM_STATUS_SUCCESS) {
        *file_size_out = file_size;
    }

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return kcm_status;
}

kcm_status_e storage_file_read(const kcm_ctx_s *ctx, const uint8_t *file_name, size_t file_name_length, uint8_t *buffer_out, size_t buffer_size, size_t *buffer_actual_size_out)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    esfs_result_e esfs_status;
    uint16_t esfs_mode = 0;        // FIXME - Unused, yet implemented
    esfs_file_t esfs_file_h;
    size_t file_size_out;
    size_t buffer_actual_size = 0;

    SA_PV_LOG_TRACE_FUNC_ENTER("file_name_length=%" PRIu32 ", buffer_size=%" PRIu32 "", (uint32_t)file_name_length, (uint32_t)buffer_size);


    // FIXME - check context is NULL until implemented
    SA_PV_ERR_RECOVERABLE_RETURN_IF((ctx != NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid KCM context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid file name context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name_length == 0), KCM_STATUS_INVALID_PARAMETER, "Got empty file name");
    SA_PV_ERR_RECOVERABLE_RETURN_IF(((buffer_out == NULL) && (buffer_size != 0)), KCM_STATUS_INVALID_PARAMETER, "Got invalid buffer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((buffer_actual_size_out == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid buffer_actual_size_out pointer");

    // storage_file_data_size_get() also checks file permissions
    kcm_status = storage_file_size_get(ctx, file_name, file_name_length, &file_size_out);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "Failed getting file data size (kcm_status %d)", kcm_status);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((buffer_size < file_size_out), KCM_STATUS_INSUFFICIENT_BUFFER, "Buffer too small");

    // If file is of size 0 don't continue reading.
    if (file_size_out == 0) {
        buffer_actual_size = file_size_out;
        goto ExitGetSizeOnly;
    }


    esfs_status = esfs_open(file_name, file_name_length, &esfs_mode, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed opening file (esfs_status %d)", esfs_status);

    esfs_status = esfs_read(&esfs_file_h, buffer_out, file_size_out, &buffer_actual_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((esfs_status != ESFS_SUCCESS), (kcm_status = error_handler(esfs_status)), out, "Failed reading file data (esfs_status %d)", esfs_status);

out:
    esfs_status = esfs_close(&esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed closing file (esfs_status %d)", esfs_status);

ExitGetSizeOnly:
    if (kcm_status == KCM_STATUS_SUCCESS) {
        *buffer_actual_size_out = buffer_actual_size;
    }

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return kcm_status;
}

kcm_status_e storage_file_delete(const kcm_ctx_s *ctx, const uint8_t *file_name, size_t file_name_length)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    esfs_result_e esfs_status;
    uint16_t esfs_mode = 0;        // FIXME - Unused, yet implemented
    esfs_file_t esfs_file_h;
    bool success;

    SA_PV_LOG_TRACE_FUNC_ENTER("file_name_length=%" PRIu32 "", (uint32_t)file_name_length);

    // FIXME - check context is NULL until implemented
    SA_PV_ERR_RECOVERABLE_RETURN_IF((ctx != NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid KCM context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid file name context");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((file_name_length == 0), KCM_STATUS_INVALID_PARAMETER, "Got empty file name");

    esfs_status = esfs_open(file_name, file_name_length, &esfs_mode, &esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed opening file (esfs_status %d)", esfs_status);

    success = is_file_accessible(ctx, &esfs_file_h);
    if (!success) {
        SA_PV_LOG_ERR("Caller has no access rights to the given file");
        kcm_status = KCM_STATUS_NOT_PERMITTED;
    }

    esfs_status = esfs_close(&esfs_file_h);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed closing file (esfs_status %d)", esfs_status);

    if (kcm_status == KCM_STATUS_NOT_PERMITTED) {
        return kcm_status;
    }

    // Success case
    esfs_status = esfs_delete(file_name, file_name_length);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((esfs_status != ESFS_SUCCESS), error_handler(esfs_status), "Failed deleting file (esfs_status %d)", esfs_status);

    SA_PV_LOG_TRACE_FUNC_EXIT_NO_ARGS();

    return kcm_status;
}
