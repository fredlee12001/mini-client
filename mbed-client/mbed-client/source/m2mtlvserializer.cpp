/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "include/m2mtlvserializer.h"
#include "mbed-client/m2mconstants.h"

#include <stdlib.h>
#include "common_functions.h"

#define TRACE_GROUP "mClt"

#define MAX_TLV_LENGTH_SIZE 3
#define MAX_TLV_ID_SIZE 2
#define TLV_TYPE_SIZE 1

uint8_t* M2MTLVSerializer::serialize(const M2MObjectInstance *object_instance_list, uint32_t &size)
{
    return serialize_object_instances(object_instance_list, size);
}

uint8_t* M2MTLVSerializer::serialize_list(const M2MResource *resource_list, uint32_t &size)
{
    bool valid = true;
    return serialize_resources(resource_list, size,valid);
}

uint8_t* M2MTLVSerializer::serialize(const M2MResource *resource, uint32_t &size)
{
    uint8_t* data = NULL;
    serialize(resource, data, size);
    return data;
}

uint8_t* M2MTLVSerializer::serialize_object_instances(const M2MObjectInstance *object_instance_list, uint32_t &size)
{
    uint8_t *data = NULL;
    const M2MObjectInstance *instance = object_instance_list;

    for (; instance != NULL; instance = instance->get_next()) {
        uint16_t id = instance->instance_id();
        serialize(id, instance, data, size);
    }
    return data;
}

uint8_t* M2MTLVSerializer::serialize_resources(const M2MResource *resource_list, uint32_t &size, bool &valid)
{
    uint8_t *data = NULL;

    const M2MResource *resource = resource_list;

    for (; resource != NULL; resource = resource->get_next()) {
        if ((resource->operation() & M2MBase::GET_ALLOWED) == M2MBase::GET_ALLOWED) {
            if(!serialize(resource, data, size)) {
                /* serializing has failed */
                /* free data so far */
                        memory_free(data);
                /* invalidate */
                valid = false;
                /* return NULL immediately */
                return NULL;
            }
        }
    }
    return data;
}

bool M2MTLVSerializer::serialize(uint16_t id, const M2MObjectInstance *object_instance, uint8_t *&data, uint32_t &size)
{
    uint8_t *resource_data = NULL;
    uint32_t resource_size = 0;
    bool success;

    bool valid = true;
    resource_data = serialize_resources(object_instance->resources(),resource_size,valid);
    if(valid) {
        if(serialize_TILV(TYPE_OBJECT_INSTANCE, id, resource_data, resource_size, data, size)) {
            success = true;
        } else {
            /* serializing object instance failed */
            success = false;
        }
        memory_free(resource_data);
    } else {
        /* serializing resources failed */
        success = false;
    }
    return success;
}

bool M2MTLVSerializer::serialize(const M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success = resource->supports_multiple_instances() ?
            serialize_multiple_resource(resource, data, size) :
            serialize_resource(resource, data, size);

    return success;
}

bool M2MTLVSerializer::serialize_resource(const M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success;

    const M2MResourceBase::ResourceType resource_type = resource->resource_instance_type();

    if ( (resource_type == M2MResourceBase::INTEGER) ||
         (resource_type == M2MResourceBase::BOOLEAN) ||
         (resource_type == M2MResourceBase::TIME) ) {
        success = serialize_TLV_binary_int(resource, TYPE_RESOURCE, resource->name_id(), data, size);
    } else {
         success = serialize_TILV(TYPE_RESOURCE, resource->name_id(),
                  resource->value(), resource->value_length(), data, size);
    }
    return success;
}

bool M2MTLVSerializer::serialize_multiple_resource(const M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success = false;
    uint8_t *nested_data = NULL;
    uint32_t nested_data_size = 0;

    const M2MResourceInstance *instance = resource->resource_instances();

    for (; instance != NULL; instance = instance->get_next()) {

        uint16_t id = instance->instance_id();
        if ((instance->operation() & M2MBase::GET_ALLOWED) == M2MBase::GET_ALLOWED) {
            if(!serialize_resource_instance(id, instance, nested_data, nested_data_size)) {
                /* serializing instance has failed */
                /* free data so far allocated */
                    memory_free(nested_data);
                /* return fail immediately*/
                success = false;
                return success;
            }
        }
    }
    if ((resource->operation() & M2MBase::GET_ALLOWED) == M2MBase::GET_ALLOWED) {
        success = serialize_TILV(TYPE_MULTIPLE_RESOURCE, resource->name_id(),
                                    nested_data, nested_data_size, data, size);
    }

    memory_free(nested_data);
    return success;
}

// TODO: this could be combined with serialize_resource(M2MResource*) to a single serialize_resource(M2MResourceBase*)
bool M2MTLVSerializer::serialize_resource_instance(uint16_t id, const M2MResourceInstance *resource, uint8_t *&data, uint32_t &size)
{
    bool success;

    const M2MResourceBase::ResourceType resource_type = resource->resource_instance_type();

    if ( (resource_type == M2MResourceBase::INTEGER) ||
         (resource_type == M2MResourceBase::BOOLEAN) ||
         (resource_type == M2MResourceBase::TIME) ) {
        success=serialize_TLV_binary_int(resource, TYPE_RESOURCE_INSTANCE, id, data, size);
    }
    else {
        success=serialize_TILV(TYPE_RESOURCE_INSTANCE, id, resource->value(), resource->value_length(), data, size);
    }

    return success;
}

/* See, OMA-TS-LightweightM2M-V1_0-20170208-A, Appendix C,
 * Data Types, Integer, Boolean and TY
 * Yime, TLV Format */
bool M2MTLVSerializer::serialize_TLV_binary_int(const M2MResourceBase *resource, uint8_t type, uint16_t id, uint8_t *&data, uint32_t &size)
{
    int64_t valueInt = resource->get_value_int();
    uint32_t buffer_size;
    /* max len 8 bytes */
    uint8_t buffer[8];

    if (resource->resource_instance_type() == M2MResourceBase::BOOLEAN) {
        buffer_size = 1;
        buffer[0] = valueInt;
    } else {
        buffer_size = 8;
        common_write_64_bit(valueInt, buffer);
    }

    return serialize_TILV(type, id, buffer, buffer_size, data, size);
}


bool M2MTLVSerializer::serialize_TILV(uint8_t type, uint16_t id, uint8_t *value, uint32_t value_length, uint8_t *&data, uint32_t &size)
{
    uint8_t *tlv = 0;
    const uint32_t type_length = TLV_TYPE_SIZE;
    type += id < 256 ? 0 : ID16;
    type += value_length < 8 ? value_length :
            value_length < 256 ? LENGTH8 :
            value_length < 65536 ? LENGTH16 : LENGTH24;
    uint8_t tlv_type;
    tlv_type = type & 0xFF;

    uint32_t id_size;
    uint8_t id_array[MAX_TLV_ID_SIZE];
    serialize_id(id, id_size, id_array);

    uint32_t length_size;
    uint8_t length_array[MAX_TLV_LENGTH_SIZE];
    serialize_length(value_length, length_size, length_array);

    tlv = (uint8_t*)memory_alloc(size + type_length + id_size + length_size + value_length);
    if (!tlv) {
        /* memory allocation has failed */
        /* return failure immediately */
        return false;
        /* eventually NULL will be returned to serializer public method caller */
    }
    if(data) {
        memcpy(tlv, data, size);
        memory_free(data);
    }
    memcpy(tlv+size, &tlv_type, type_length);
    memcpy(tlv+size+type_length, id_array, id_size);
    memcpy(tlv+size+type_length+id_size, length_array, length_size);
    memcpy(tlv+size+type_length+id_size+length_size, value, value_length);

    data = tlv;
    size += type_length + id_size + length_size + value_length;
    return true;
}

void M2MTLVSerializer::serialize_id(uint16_t id, uint32_t &size, uint8_t *id_ptr)
{
    if(id > 255) {
        size=2;
        id_ptr[0] = (id & 0xFF00) >> 8;
        id_ptr[1] = id & 0xFF;
    } else {
        size=1;
        id_ptr[0] = id & 0xFF;
    }
}

void M2MTLVSerializer::serialize_length(uint32_t length, uint32_t &size, uint8_t *length_ptr)
{
    if (length > 65535) {
        size = 3;
        length_ptr[0] = (length & 0xFF0000) >> 16;
        length_ptr[1] = (length & 0xFF00) >> 8;
        length_ptr[2] = length & 0xFF;
    } else if (length > 255) {
        size = 2;
        length_ptr[0] = (length & 0xFF00) >> 8;
        length_ptr[1] = length & 0xFF;
    } else if (length > 7) {
        size = 1;
        length_ptr[0] = length & 0xFF;
    } else {
        size=0;
    }
}

