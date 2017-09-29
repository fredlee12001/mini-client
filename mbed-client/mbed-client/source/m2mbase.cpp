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

#include "mbed-client/m2mbase.h"
#include "mbed-client/m2mobservationhandler.h"
#include "mbed-client/m2mtimer.h"

#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mobjectinstance.h"
#include "mbed-client/m2mresource.h"

#include "include/m2mreporthandler.h"
#include "include/nsdlaccesshelper.h"
#include "include/m2mcallbackstorage.h"
#include "mbed-trace/mbed_trace.h"

#include "sn_nsdl_lib.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define TRACE_GROUP "mClt"

M2MBase::M2MBase(M2MBase::BaseType base_type,
                 uint16_t name_id,
                 M2MBase::Mode mode,
                 const char *resource_type,
                 char *path,
                 bool external_blockwise_store,
                 bool multiple_instance,
                 M2MBase::DataType type)
:
  _report_handler(NULL)
{
    // Checking the name range properly, i.e returning error is impossible from constructor without exceptions
    // XXX: is this max id value really needed for something?
    assert(name_id <= MAX_UNINT_16_COUNT);

    memset(&_sn_resource, 0, sizeof(lwm2m_parameters_s));

    _sn_resource.multiple_instance = multiple_instance;
    _sn_resource.base_type = base_type;
    _sn_resource.data_type = type;

    set_resource_type(resource_type);
    _sn_resource.dynamic_resource_params.mode = (unsigned)mode;
    _sn_resource.dynamic_resource_params.external_memory_block = external_blockwise_store;
    _sn_resource.name_id = name_id;
    _sn_resource.dynamic_resource_params.publish_uri = true;    
}

M2MBase::~M2MBase()
{
    tr_debug("M2MBase::~M2MBase()");
    delete _report_handler;

    value_updated_callback* callback = (value_updated_callback*)M2MCallbackStorage::remove_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback);
    delete callback;

    M2MCallbackStorage::remove_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback2);
}

void M2MBase::set_operation(M2MBase::Operation opr)
{
    // If the mode is Static, there is only GET_ALLOWED supported.
    if(M2MBase::Static == mode()) {
        _sn_resource.dynamic_resource_params.access = M2MBase::GET_ALLOWED;
    } else {
        _sn_resource.dynamic_resource_params.access = opr;
    }
}

void M2MBase::set_interface_description(const char *desc)
{
#ifdef RESOURCE_ATTRIBUTES_LIST
    if(desc) {
        const size_t len = strlen(desc);
        if (len > 0 ) {
            sn_nsdl_attribute_item_s item;
            item.attribute_name = ATTR_INTERFACE_DESCRIPTION;
            item.value = (char*)alloc_string_copy((uint8_t*) desc, len);
            sn_nsdl_set_resource_attribute(&_sn_resource.dynamic_resource_params, &item);
        }
    }
#endif
}

void M2MBase::set_resource_type(const char *res_type)
{
#ifdef RESOURCE_ATTRIBUTES_LIST
    if(res_type) {
        const size_t len = strlen(res_type);
        if (len > 0) {
            sn_nsdl_attribute_item_s item;
            item.attribute_name = ATTR_RESOURCE_TYPE;
            item.value = (char*)alloc_string_copy((uint8_t*) res_type, len);
            sn_nsdl_set_resource_attribute(&_sn_resource.dynamic_resource_params, &item);
        }
    }
#endif
}

void M2MBase::set_coap_content_type(const uint16_t con_type)
{
    _sn_resource.dynamic_resource_params.coap_content_type = con_type;
}

void M2MBase::set_observable(bool observable)
{
    _sn_resource.dynamic_resource_params.observable = observable;
}

void M2MBase::add_observation_level(M2MBase::Observation obs_level)
{

    if(_report_handler) {
        _report_handler->add_observation_level(obs_level);
    }
}

void M2MBase::remove_observation_level(M2MBase::Observation obs_level)
{
    if(_report_handler) {
        _report_handler->remove_observation_level(obs_level);
    }
}


void M2MBase::set_under_observation(bool observed,
                                    M2MObservationHandler *handler)
{
    tr_debug("M2MBase::set_under_observation - observed: %d", observed);
    tr_debug("M2MBase::set_under_observation - base_type: %d", base_type());
    if(_report_handler) {
        _report_handler->set_under_observation(observed);
    }

    set_observation_handler(handler);

    if (handler) {
        if (base_type() != M2MBase::ResourceInstance) {
            // Create report handler only if it does not exist and one wants observation
            // This saves 76 bytes of memory on most usual case.
            if (observed) {
                if(!_report_handler) {
                    _report_handler = new M2MReportHandler(*this);
                }
            }
            if (_report_handler) {
                _report_handler->set_under_observation(observed);
            }
        }
    } else {
        delete _report_handler;
        _report_handler = NULL;
    }
}

void M2MBase::set_observation_token(const uint8_t *token, const uint8_t length)
{
    if(_report_handler) {
        _report_handler->set_observation_token(token, length);
    }
}

void M2MBase::set_max_age(const uint32_t max_age)
{
    _sn_resource.max_age = max_age;
}

M2MBase::BaseType M2MBase::base_type() const
{
    return (M2MBase::BaseType)_sn_resource.base_type;
}

M2MBase::Operation M2MBase::operation() const
{
    return (M2MBase::Operation)_sn_resource.dynamic_resource_params.access;
}

// XXX: finally we can combine the name_id and name_instance_id
int32_t M2MBase::name_id() const
{
    return instance_id();
}

uint16_t M2MBase::instance_id() const
{
    return _sn_resource.name_id;
}


const char* M2MBase::interface_description() const
{
#ifdef RESOURCE_ATTRIBUTES_LIST
    return sn_nsdl_get_resource_attribute(&_sn_resource.dynamic_resource_params, ATTR_INTERFACE_DESCRIPTION);
#else
    return NULL;
#endif
}
const char* M2MBase::resource_type() const
{
#ifdef RESOURCE_ATTRIBUTES_LIST
    return sn_nsdl_get_resource_attribute(&_sn_resource.dynamic_resource_params, ATTR_RESOURCE_TYPE);
#else
    return NULL;
#endif
}

void M2MBase::uri_path(path_buffer &buffer) const
{
    buffer.append_int(name_id());
}

uint16_t M2MBase::coap_content_type() const
{
    return _sn_resource.dynamic_resource_params.coap_content_type;
}

bool M2MBase::is_observable() const
{
    return _sn_resource.dynamic_resource_params.observable;
}

M2MBase::Observation M2MBase::observation_level() const
{
    M2MBase::Observation obs_level = M2MBase::None;
    if(_report_handler) {
        obs_level = _report_handler->observation_level();
    }
    return obs_level;
}

void M2MBase::get_observation_token(uint8_t *&token, uint8_t &token_length) const
{
    if(_report_handler) {
        token = (uint8_t*)_report_handler->get_observation_token();
        token_length = _report_handler->get_observation_token_length();
    }
}

M2MBase::Mode M2MBase::mode() const
{
    return (M2MBase::Mode)_sn_resource.dynamic_resource_params.mode;
}

uint16_t M2MBase::observation_number() const
{
    uint16_t obs_number = 0;
    if(_report_handler) {
        obs_number = _report_handler->observation_number();
    }
    return obs_number;
}

uint32_t M2MBase::max_age() const
{
    return _sn_resource.max_age;
}

bool M2MBase::handle_observation_attribute(const char *query)
{
    tr_debug("M2MBase::handle_observation_attribute - under observation(%d)", is_under_observation());
    bool success = false;
    // Create handler if not already exists. Client must able to parse write attributes even when
    // observation is not yet set
    if (!_report_handler) {
        _report_handler = new M2MReportHandler(*this);
    }

    success = _report_handler->parse_notification_attribute(query,base_type());
    if (success) {
        if (is_under_observation()) {
            _report_handler->set_under_observation(true);
        }
     } else {
        _report_handler->set_default_values();
    }
    return success;
}

void M2MBase::observation_to_be_sent(const m2m::Vector<uint16_t> &changed_instance_ids,
                                     uint16_t obs_number,
                                     bool send_object)
{
    //TODO: Move this to M2MResourceInstance
    M2MObservationHandler *obs_handler = observation_handler();
    if (obs_handler) {
        obs_handler->observation_to_be_sent(this,
                                            obs_number,
                                            changed_instance_ids,
                                            send_object);
    }
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
                                           bool &)
{
    //Handled in M2MResource, M2MObjectInstance and M2MObject classes
    return NULL;
}

sn_coap_hdr_s* M2MBase::handle_post_request(nsdl_s */*nsdl*/,
                                            sn_coap_hdr_s */*received_coap_header*/,
                                            M2MObservationHandler */*observation_handler*/,
                                            bool &,
                                            sn_nsdl_addr_s *)
{
    //Handled in M2MResource, M2MObjectInstance and M2MObject classes
    return NULL;
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
    if (!_report_handler) {
        _report_handler = new M2MReportHandler(*this);
    }
    return _report_handler;
}

M2MReportHandler* M2MBase::report_handler() const
{
    return _report_handler;
}

void M2MBase::set_register_uri(bool register_uri)
{
    _sn_resource.dynamic_resource_params.publish_uri = register_uri;
}

bool M2MBase::register_uri() const
{
    return _sn_resource.dynamic_resource_params.publish_uri;
}

bool M2MBase::is_integer(const char *value)
{
    assert(value != NULL);

    if((strlen(value) < 1) || ((!isdigit(value[0])) && (value[0] != '-') && (value[0] != '+'))) {
        return false;
    }
    char * p;
    strtol(value, &p, 10);
    return (*p == 0);
}

bool M2MBase::is_under_observation() const
{
   bool under_observation = false;
    if(_report_handler) {
        under_observation = _report_handler->is_under_observation();
    }
    return under_observation;
}

bool M2MBase::set_value_updated_function(value_updated_callback callback)
{
    value_updated_callback* old_callback = (value_updated_callback*)M2MCallbackStorage::remove_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback);
    delete old_callback;
    // XXX: create a copy of the copy of callback object. Perhaps it would better to
    // give a reference as parameter and just store that, as it would save some memory.
    value_updated_callback* new_callback = new value_updated_callback(callback);

    return M2MCallbackStorage::add_callback(*this, new_callback, M2MCallbackAssociation::M2MBaseValueUpdatedCallback);
}

bool M2MBase::set_value_updated_function(value_updated_callback2 callback)
{
    M2MCallbackStorage::remove_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback2);

    return M2MCallbackStorage::add_callback(*this, (void*)callback, M2MCallbackAssociation::M2MBaseValueUpdatedCallback2);
}

bool M2MBase::is_value_updated_function_set() const
{
    bool func_set = false;
    if ((M2MCallbackStorage::does_callback_exist(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback) == true) ||
        (M2MCallbackStorage::does_callback_exist(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback2) == true)) {

        func_set = true;
    }
    return func_set;
}

// TODO: should the parameter really be the uri_path, as the object/resource -id really does not help too much?
void M2MBase::execute_value_updated(const char *name)
{
    // Q: is there a point to call both callback types? Or should we call just one of them?

    value_updated_callback* callback = (value_updated_callback*)M2MCallbackStorage::get_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback);
    if (callback) {
        (*callback)(name);
    }

    value_updated_callback2 callback2 = (value_updated_callback2)M2MCallbackStorage::get_callback(*this, M2MCallbackAssociation::M2MBaseValueUpdatedCallback2);
    if (callback2) {
        (*callback2)(name);
    }
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

char* M2MBase::stringdup(const char* src)
{
    assert(src != NULL);

    const size_t len = strlen(src) + 1;

    char *dest = (char*)memory_alloc(len);

    if (dest) {
        memcpy(dest, src, len);
    }
    return dest;
}

void M2MBase::free_resources()
{
    // remove the nsdl structures from the nsdlinterface's lists.
    M2MObservationHandler *obs_handler = observation_handler();
    if (obs_handler) {
        tr_debug("M2MBase::free_resources()");
        obs_handler->resource_to_be_deleted(this);
    }
#ifdef RESOURCE_ATTRIBUTES_LIST
    sn_nsdl_free_resource_attributes_list(&_sn_resource.dynamic_resource_params);
#endif
    memory_free(_sn_resource.dynamic_resource_params.resource);
}

const sn_nsdl_dynamic_resource_parameters_s& M2MBase::get_nsdl_resource() const
{
    return _sn_resource.dynamic_resource_params;
}

sn_nsdl_dynamic_resource_parameters_s& M2MBase::get_nsdl_resource()
{
    return _sn_resource.dynamic_resource_params;
}

const M2MBase::lwm2m_parameters_s& M2MBase::get_lwm2m_parameters() const
{
    return _sn_resource;
}

uint16_t M2MBase::get_notification_msgid() const
{
    return _sn_resource.dynamic_resource_params.msg_id;
}

void M2MBase::set_notification_msgid(uint16_t msgid)
{
    _sn_resource.dynamic_resource_params.msg_id = msgid;
}
