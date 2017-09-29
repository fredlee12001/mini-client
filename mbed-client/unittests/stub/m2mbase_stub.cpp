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
#include "m2mbase_stub.h"
#include "m2mstringbufferbase_stub.h"
#include <assert.h>

uint16_t m2mbase_stub::uint16_value;
uint32_t m2mbase_stub::uint32_value;
uint16_t m2mbase_stub::int_value;
int32_t m2mbase_stub::name_id_value;

bool m2mbase_stub::bool_value;
const char *m2mbase_stub::string_value;
const char *m2mbase_stub::object_instance_name;
const char *m2mbase_stub::resource_name;
const char *m2mbase_stub::resource_name_inst;
char *m2mbase_stub::path;

// if set to true, the destructor will free() the path
bool m2mbase_stub::free_path_member_on_delete;

M2MBase::BaseType m2mbase_stub::base_type;
M2MBase::Operation m2mbase_stub::operation;
M2MBase::Mode m2mbase_stub::mode_value;
M2MBase::Observation m2mbase_stub::observation_level_value;

void *m2mbase_stub::void_value;
M2MObservationHandler *m2mbase_stub::observe;
M2MReportHandler *m2mbase_stub::report;
bool m2mbase_stub::is_value_updated_function_set;
uint16_t m2mbase_stub::object_msg_id;
uint16_t m2mbase_stub::resource_msg_id;
uint16_t m2mbase_stub::object_instance_msg_id;

bool m2mbase_stub::use_real_uri_path;
int32_t m2mbase_stub::ret_counter;

void m2mbase_stub::clear()
{
    int_value = 0;
    uint16_value = 0;
    uint32_value = 0;
    string_value = NULL;
    object_instance_name = NULL;
    resource_name = NULL;
    resource_name_inst = NULL;
    free_path_member_on_delete = false;
    name_id_value = -1;
    mode_value = M2MBase::Static;
    base_type = M2MBase::Object;
    observation_level_value = M2MBase::None;
    bool_value = false;
    m2mbase_stub::operation = M2MBase::NOT_ALLOWED;
    void_value = NULL;
    observe = NULL;
    report = NULL;
    is_value_updated_function_set = false;
    use_real_uri_path = false;
    ret_counter = 0;
    object_msg_id = 0;
    resource_msg_id = 0;
    object_instance_msg_id = 0;
}



M2MBase::M2MBase(M2MBase::BaseType base_type,
                 uint16_t name_id,
                 M2MBase::Mode mode,
#ifndef DISABLE_RESOURCE_TYPE
                 const char *resource_type,
#endif
                 char *path,
                 bool external_blockwise_store,
                 bool multiple_instance,
                 M2MBase::DataType type)
:
  _report_handler(NULL)
{
    m2mbase_stub::base_type = base_type; // XXX?

    // We need at least a basic context on each object even if it is stubbed, as each
    // one of them carries the id of it (and path, which is mostly unused now in tests).
    memset(&_sn_resource, 0, sizeof(lwm2m_parameters_s));
    _sn_resource.free_on_delete = true;

    // This needs to be stored per object basis as global m2mbase_stub's id does not work when the id queries
    // have to work at each level (object, obj-inst, resource, res-inst)
    _sn_resource.name_id = name_id;

    _sn_resource.base_type = base_type;

#ifndef DISABLE_RESOURCE_TYPE
    _sn_resource.dynamic_resource_params.resource_type_ptr = resource_type;
#endif

    // NOTE: this is stored just in case the free_on_delete() is ran and free_path_member_on_delete is set
    // This system is needed as the stub is used from the inheriting classes, which do have a dynamic copy of string but
    // also a lot of tests pass a string literal as a path, which can NOT be freed.
    _sn_resource.dynamic_resource_params.path = path;
}


M2MBase::~M2MBase()
{
}

char* M2MBase::create_path(const M2MObject &parent, uint16_t object_instance)
{
    return m2mbase_stub::path;
}

char* M2MBase::create_path(const M2MResource &parent, uint16_t resource_instance)
{
    return m2mbase_stub::path;
}

char* M2MBase::create_path(const M2MObjectInstance &parent, uint16_t resource_id)
{
    return m2mbase_stub::path;
}


void M2MBase::free_resources()
{
    if (m2mbase_stub::free_path_member_on_delete) {
        free(_sn_resource.dynamic_resource_params.path);
    }
    free(_sn_resource.dynamic_resource_params.resource);
}

void M2MBase::set_operation(M2MBase::Operation opr)
{
    m2mbase_stub::operation = opr;
}

void M2MBase::set_interface_description(const char */*desc*/)
{
}

void M2MBase::set_resource_type(const char */*res_type*/)
{
}

void M2MBase::set_coap_content_type(const uint16_t /*con_type*/)
{
}

void M2MBase::set_max_age(const uint32_t /*max_age*/)
{
}

M2MBase::Operation M2MBase::operation() const
{
    return m2mbase_stub::operation;
}

// TODO: merge name_id() and instance_id()
int32_t M2MBase::name_id() const
{
    int32_t id = _sn_resource.name_id;
    return id;
}

uint16_t M2MBase::instance_id() const
{
    return name_id();
}

const char* M2MBase::interface_description() const
{
    return m2mbase_stub::string_value;
}

#ifndef DISABLE_RESOURCE_TYPE
const char* M2MBase::resource_type() const
{

    return _sn_resource.dynamic_resource_params.resource_type_ptr;
}
#endif

uint16_t M2MBase::coap_content_type() const
{
    return m2mbase_stub::uint16_value;
}

uint32_t M2MBase::max_age() const
{
    return m2mbase_stub::uint32_value;
}

void M2MBase::set_observable(bool /*observable*/)
{
}

void M2MBase::add_observation_level(M2MBase::Observation)
{
}

void M2MBase::remove_observation_level(M2MBase::Observation)
{
}

void M2MBase::set_under_observation(bool /*observed*/,
                                   M2MObservationHandler *handler)
{
    m2mbase_stub::observe = handler;
}

void M2MBase::set_observation_token(const uint8_t */*token*/,
                                    const uint8_t /*length*/)
{
}

bool M2MBase::is_observable() const
{
    return m2mbase_stub::bool_value;
}

M2MBase::Observation M2MBase::observation_level() const
{
    return m2mbase_stub::observation_level_value;
}

void M2MBase::get_observation_token(uint8_t *&token,
                                    uint32_t &length) const
{

}

void M2MBase::get_observation_token(const uint8_t *&token, uint32_t &token_length) const
{
    // XXX
}

M2MBase::BaseType M2MBase::base_type() const
{
    return _sn_resource.base_type;
}

M2MBase::Mode M2MBase::mode() const
{
    return m2mbase_stub::mode_value;
}

uint16_t M2MBase::observation_number() const
{
    return m2mbase_stub::uint16_value;
}

bool M2MBase::handle_observation_attribute(const char *query)
{
    return m2mbase_stub::bool_value;
}

void M2MBase::observation_to_be_sent(const m2m::Vector<uint16_t>&,
                                        uint16_t,
                                        bool send_object)
{
}

char* M2MBase::alloc_string_copy(const char* source)
{
    assert(source != NULL);

    // Note: the armcc's libc does not have strdup, so we need to implement it here
    const size_t len = strlen(source);

    return (char*)alloc_string_copy((uint8_t*)source, len);
}

uint8_t* M2MBase::alloc_string_copy(const uint8_t* source, uint32_t size)
{
    assert(source != NULL);

    uint8_t* result = (uint8_t*)memory_alloc(size + 1);
    if (result) {
        memcpy(result, source, size);
        result[size] = '\0';
    }
    return result;
}

uint8_t* M2MBase::alloc_copy(const uint8_t* source, uint32_t size)
{
    assert(source != NULL);

    uint8_t* result = (uint8_t*)memory_alloc(size);
    if (result) {
        memcpy(result, source, size);
    }
    return result;
}

M2MReportHandler* M2MBase::report_handler() const
{
    return m2mbase_stub::report;
}

M2MObservationHandler* M2MBase::observation_handler() const
{
    return m2mbase_stub::observe;
}

sn_coap_hdr_s* M2MBase::handle_get_request(nsdl_s */*nsdl*/,
                                           sn_coap_hdr_s */*received_coap_header*/,
                                           M2MObservationHandler */*observation_handler*/)
{
    //Handled in M2MResource, M2MObjectInstance and M2MObject classes
    return NULL;
}

sn_coap_hdr_s* M2MBase::handle_put_request(nsdl_s */*nsdl*/,
                                           sn_coap_hdr_s */*received_coap_header*/,
                                           M2MObservationHandler */*observation_handler*/,
                                           bool &execute_value_updated)
{
    //Handled in M2MResource, M2MObjectInstance and M2MObject classes
    return NULL;
}

sn_coap_hdr_s* M2MBase::handle_post_request(nsdl_s */*nsdl*/,
                                            sn_coap_hdr_s */*received_coap_header*/,
                                            M2MObservationHandler */*observation_handler*/,
                                            bool &, sn_nsdl_addr_s *address)
{
    //Handled in M2MResource, M2MObjectInstance and M2MObject classes
    return NULL;
}

void M2MBase::set_register_uri( bool register_uri)
{
}

bool M2MBase::register_uri() const
{
    return m2mbase_stub::bool_value;
}

const char* M2MBase::uri_path() const
{
    if (m2mbase_stub::use_real_uri_path) {
        return _sn_resource.dynamic_resource_params.path;
    } else {
        return m2mbase_stub::string_value;
    }
}

bool M2MBase::is_under_observation() const
{
    return m2mbase_stub::bool_value;
}

bool M2MBase::set_value_updated_function(value_updated_callback callback)
{

}

bool M2MBase::set_value_updated_function(value_updated_callback2 callback)
{

}

bool M2MBase::is_value_updated_function_set() const
{
    return m2mbase_stub::is_value_updated_function_set;
}

void M2MBase::execute_value_updated(const char *name)
{

}

const sn_nsdl_dynamic_resource_parameters_s& M2MBase::get_nsdl_resource() const
{
    return _sn_resource.dynamic_resource_params;
}

sn_nsdl_dynamic_resource_parameters_s& M2MBase::get_nsdl_resource()
{
    return _sn_resource.dynamic_resource_params;
}

char* M2MBase::stringdup(const char* src)
{
    assert(src != NULL);

    const size_t len = strlen(src) + 1;

    char *dest = (char*)malloc(len);

    if (dest) {
        memcpy(dest, src, len);
    }
    return dest;
}

bool M2MBase::build_path(StringBuffer<MAX_PATH_SIZE_3> &buffer, uint16_t i1, uint16_t i2, uint16_t i3)
{
    if (!buffer.ensure_space((MAX_INSTANCE_SIZE * 3) + 2 + 1)){
        return false;
    }

    buffer.append_int(i1);
    buffer.append('/');
    buffer.append_int(i2);
    buffer.append('/');
    buffer.append_int(i3);

    return true;
}

char* M2MBase::make_string(uint16_t src_int)
{
    StringBuffer<6> src;

    src.append_int(src_int);

    return M2MBase::stringdup(src.c_str());
}

void M2MBase::set_observation_handler(M2MObservationHandler *handler)
{

}

size_t M2MBase::resource_name_length() const
{

}

const M2MBase::lwm2m_parameters_s &M2MBase::get_lwm2m_parameters() const
{
    // We now use real ref to _sn_resource in the object itself.
    return _sn_resource;
}

// There needs to be a real implementation for this as it is used from the input validation
// on higher levels. Having a stubbed 'true' result really does not help in functionality validation.
bool M2MBase::validate_string_length(const char* str, size_t min_length, size_t max_length)
{
    bool valid = false;

    if (str != NULL) {
        const size_t len = strlen(str);
        if ((len >= min_length) && (len <= max_length)) {
            valid = true;
        }
    }
    return valid;
}

M2MReportHandler* M2MBase::create_report_handler()
{
    return NULL;
}

uint16_t M2MBase::get_notification_msgid() const
{
    return _sn_resource.dynamic_resource_params.msg_id;
}

void M2MBase::set_notification_msgid(uint16_t msgid)
{
    _sn_resource.dynamic_resource_params.msg_id = msgid;
}
