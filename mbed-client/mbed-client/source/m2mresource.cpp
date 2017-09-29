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
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mobservationhandler.h"
#include "include/m2mreporthandler.h"
#include "include/m2mtlvserializer.h"
#include "include/m2mtlvdeserializer.h"
#include "mbed-trace/mbed_trace.h"

#include <stdlib.h>

#define TRACE_GROUP "mClt"

bool M2MResource::allocate_resources(const uint8_t *value,
                                     const uint8_t value_length)
{
    if(M2MResourceBase::allocate_resources(value, value_length)) {
        return true;
    } else {
        return false;
    }
}

M2MResource::M2MResource(M2MObjectInstance &parent,
                         uint16_t name_id,    
                         M2MBase::Mode resource_mode,
                         const char *resource_type,
                         M2MBase::DataType type,
                         bool observable,
                         char *path,
                         bool multiple_instance,
                         bool external_blockwise_store)
: M2MResourceBase(M2MBase::Resource, name_id, resource_mode, resource_type, type,
                  path, external_blockwise_store, multiple_instance),
  _parent(parent),
  _next(NULL),
  _resource_instance_list(NULL)
{
    M2MBase::set_operation(M2MBase::GET_ALLOWED);
    M2MBase::set_observable(observable);
    if (multiple_instance) {
        M2MBase::set_coap_content_type(COAP_CONTENT_OMA_TLV_TYPE_OLD);
    }
}

M2MResource::~M2MResource()
{
    M2MResourceInstance* res = _resource_instance_list;

    while (res) {
        M2MResourceInstance* next = res->get_next();

        //Free allocated memory for resources.
        delete res;

        res = next;
    }

    // remove the nsdl structures from the nsdlinterface's lists.
    M2MObservationHandler* obs_handler = observation_handler();
    if (obs_handler) {
        tr_debug("M2MResource::free_resources()");
        obs_handler->resource_to_be_deleted(this);
    }
    free_resources();
}

bool M2MResource::supports_multiple_instances() const
{
    const M2MBase::lwm2m_parameters_s& param = M2MBase::get_lwm2m_parameters();
    return param.multiple_instance;
}

void M2MResource::send_post_response(const uint8_t* token, uint8_t token_length)
{
    // At least on some unit tests the resource object is not fully constructed, which would
    // cause issues if the observation_handler is NULL. So do the check before dereferencing pointer.
    M2MObservationHandler* obs = observation_handler();
    if (obs) {
        obs->send_post_response(this, token, token_length);
    }
}

bool M2MResource::remove_resource_instance(uint16_t inst_id)
{
    tr_debug("M2MResource::remove_resource(inst_id %d)", inst_id);
    bool success = false;

    M2MResourceInstance* res = _resource_instance_list;
    M2MResourceInstance* prev = NULL;

    while (res) {
        M2MResourceInstance* next = res->get_next();

        if (res->instance_id() == inst_id) {
            // Resource found so lets delete it

            if (prev == NULL) {
                // this was the first item, we need to update things differently
                _resource_instance_list = next;
            } else {
                prev->set_next(next);
            }
            //Free allocated memory for resources.
            delete res;
            success = true;
            break;
        }

        prev = res;
        res = next;
    }

    return success;
}

M2MResourceInstance* M2MResource::resource_instance(uint16_t inst_id) const
{
    tr_debug("M2MResource::resource(resource_name inst_id %d)", inst_id);

    M2MResourceInstance *found_res = NULL;

    M2MResourceInstance* res = _resource_instance_list;

    while (res) {

        if (res->instance_id() == inst_id) {
            // Resource found.
            found_res = res;
            break;
        }

        res = res->get_next();
    }

    return found_res;
}

const M2MResourceInstance* M2MResource::resource_instances() const
{
    return _resource_instance_list;
}

M2MResourceInstance* M2MResource::resource_instances()
{
    return _resource_instance_list;
}

uint16_t M2MResource::resource_instance_count() const
{
    // since the instance count is not stored anywhere (to save memory), it needs
    // to be calculated at run time, which is a O(N) operation.

    uint16_t count = 0;
    const M2MResourceInstance* res = _resource_instance_list;

    while (res) {

        count++;

        res = res->get_next();
    }

    return count;
}

M2MObservationHandler* M2MResource::observation_handler() const
{
    const M2MObjectInstance& parent_object_instance = get_parent_object_instance();

    // XXX: need to check the flag too
    return parent_object_instance.observation_handler();
}

void M2MResource::set_observation_handler(M2MObservationHandler *handler)
{
    M2MObjectInstance& parent_object_instance = get_parent_object_instance();

    // XXX: need to set the flag too
    parent_object_instance.set_observation_handler(handler);
}

bool M2MResource::handle_observation_attribute(const char *query)
{
    tr_debug("M2MResource::handle_observation_attribute - is_under_observation(%d)", is_under_observation());
    bool success = false;
    M2MReportHandler *handler = M2MBase::report_handler();
    if (!handler) {
        handler = M2MBase::create_report_handler();
    }

    if (handler) {
        success = handler->parse_notification_attribute(query,
                M2MBase::base_type(), resource_instance_type());
        if (success) {
            if (is_under_observation()) {
                handler->set_under_observation(true);
            }
        }
        else {
            handler->set_default_values();
        }

        if (success) {

            M2MResourceInstance* res = _resource_instance_list;

            while (res) {

                 M2MReportHandler *report_handler = res->report_handler();
                 if (report_handler && is_under_observation()) {
                     report_handler->set_notification_trigger();
                 }

                 res = res->get_next();
             }
        }
    }
    return success;
}

void M2MResource::add_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::add_observation_level(observation_level);

    M2MResourceInstance* res = _resource_instance_list;

    while (res) {

        res->add_observation_level(observation_level);

        res = res->get_next();
    }
}

void M2MResource::remove_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::remove_observation_level(observation_level);

    M2MResourceInstance* res = _resource_instance_list;

    while (res) {

        res->remove_observation_level(observation_level);

        res = res->get_next();
    }
}

void M2MResource::add_resource_instance(M2MResourceInstance *res)
{
    tr_debug("M2MResource::add_resource_instance()");

    if (res) {
        // verify, that the item is not already part of some list or this will create eternal loop
        assert(res->get_next() == NULL);

        // We keep the natural order of instances by adding to the end of list, which makes
        // this a O(N) operation instead of O(1) as if we added to the head of list. This
        // is not strictly speaking necessary but it is the old behaviour.
        M2MResourceInstance* last = _resource_instance_list;

        if (last == NULL) {
            _resource_instance_list = res;
        } else {
            // loop until we find the last one
            while ((last != NULL) && (last->get_next() != NULL)) {
                last = last->get_next();
                assert(last != res); // verify that we are not adding the same object there twice
            }
            last->set_next(res);
            res->set_next(NULL);
        }
    }
}

sn_coap_hdr_s* M2MResource::handle_get_request(nsdl_s *nsdl,
                                               sn_coap_hdr_s *received_coap_header,
                                               M2MObservationHandler *observation_handler)
{
    tr_debug("M2MResource::handle_get_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
    sn_coap_hdr_s * coap_response = NULL;
    if(supports_multiple_instances()) {
        coap_response = sn_nsdl_build_response(nsdl,
                                               received_coap_header,
                                               msg_code);
        if(received_coap_header) {
            // process the GET if we have registered a callback for it
            if ((operation() & SN_GRS_GET_ALLOWED) != 0) {
                if(coap_response) {
                    bool content_type_present = false;
                    bool is_content_type_supported = true;

                    if (received_coap_header->options_list_ptr &&
                            received_coap_header->options_list_ptr->accept != COAP_CT_NONE) {
                        content_type_present = true;
                        coap_response->content_format = received_coap_header->options_list_ptr->accept;
                    }

                    // Check if preferred content type is supported
                    if (content_type_present) {
                        if (coap_response->content_format != COAP_CONTENT_OMA_TLV_TYPE_OLD &&
                            coap_response->content_format != COAP_CONTENT_OMA_TLV_TYPE) {
                            is_content_type_supported = false;
                        }
                    }

                    if (is_content_type_supported) {
                        if(!content_type_present &&
                           (M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE ||
                            M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE_OLD)) {
                            coap_response->content_format = sn_coap_content_format_e(M2MBase::coap_content_type());
                        }

                        tr_debug("M2MResource::handle_get_request() - Request Content-Type %d", coap_response->content_format);

                        uint8_t *data = NULL;
                        uint32_t data_length = 0;
                        // fill in the CoAP response payload
                        if(COAP_CONTENT_OMA_TLV_TYPE == coap_response->content_format ||
                           COAP_CONTENT_OMA_TLV_TYPE_OLD == coap_response->content_format) {
                            set_coap_content_type(coap_response->content_format);
                            data = M2MTLVSerializer::serialize(this, data_length);
                        }

                        coap_response->payload_len = data_length;
                        coap_response->payload_ptr = data;

                        coap_response->options_list_ptr = sn_nsdl_alloc_options_list(nsdl, coap_response);

                        coap_response->options_list_ptr->max_age = max_age();

                        if(received_coap_header->options_list_ptr) {
                            if(received_coap_header->options_list_ptr->observe != -1) {
                                if (is_observable()) {
                                    uint32_t number = 0;
                                    uint8_t observe_option = 0;
                                    observe_option = received_coap_header->options_list_ptr->observe;
                                    if(START_OBSERVATION == observe_option) {
                                        tr_debug("M2MResource::handle_get_request - Starts Observation");
                                        // If the observe length is 0 means register for observation.
                                        if(received_coap_header->options_list_ptr->observe != -1) {
                                            number = received_coap_header->options_list_ptr->observe;
                                        }

                                        // If the observe value is 0 means register for observation.
                                        if(number == 0) {
                                            tr_debug("M2MResource::handle_get_request - Put Resource under Observation");

                                        // add observation for this resource and all of its resource instances
                                            set_under_observation(true,observation_handler);
                                        add_observation_level(M2MBase::R_Attribute);
                                            coap_response->options_list_ptr->observe = observation_number();
                                        }

                                        if(received_coap_header->token_ptr) {
                                            tr_debug("M2MResource::handle_get_request - Sets Observation Token to resource");
                                            set_observation_token(received_coap_header->token_ptr,
                                                                  received_coap_header->token_len);
                                        }

                                    } else if (STOP_OBSERVATION == observe_option) {
                                        tr_debug("M2MResource::handle_get_request - Stops Observation");
                                        set_under_observation(false,NULL);

                                    // remove observation from this resource and all of its resource instances
                                    remove_observation_level(M2MBase::R_Attribute);
                                    }
                                    msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                                } else {
                                    msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
                                }
                            }
                        }
                } else {
                        tr_error("M2MResource::handle_get_request() - Content-Type %d not supported", coap_response->content_format);
                        msg_code = COAP_MSG_CODE_RESPONSE_NOT_ACCEPTABLE;
                    }
                }
            } else {
                tr_error("M2MResource::handle_get_request - Return COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
                // Operation is not allowed.
                msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
            }
        }
        if(coap_response) {
            coap_response->msg_code = msg_code;
        }
    } else {
        coap_response = M2MResourceBase::handle_get_request(nsdl,
                            received_coap_header,
                            observation_handler);
    }
    return coap_response;
}

sn_coap_hdr_s* M2MResource::handle_put_request(nsdl_s *nsdl,
                                               sn_coap_hdr_s *received_coap_header,
                                               M2MObservationHandler *observation_handler,
                                               bool &execute_value_updated)
{
    tr_debug("M2MResource::handle_put_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 2.04
    sn_coap_hdr_s * coap_response = NULL;
    if(supports_multiple_instances()) {
        coap_response = sn_nsdl_build_response(nsdl,
                                               received_coap_header,
                                               msg_code);
        // process the PUT if we have registered a callback for it
        if(received_coap_header) {
            uint16_t coap_content_type = 0;
            bool content_type_present = false;
            if(received_coap_header->content_format != COAP_CT_NONE && coap_response) {
                content_type_present = true;
                coap_content_type = received_coap_header->content_format;
            }
            if(received_coap_header->options_list_ptr &&
               received_coap_header->options_list_ptr->uri_query_ptr) {
                char *query = (char*)alloc_string_copy(received_coap_header->options_list_ptr->uri_query_ptr,
                                                        received_coap_header->options_list_ptr->uri_query_len);
                if (query){
                    msg_code = COAP_MSG_CODE_RESPONSE_CHANGED;
                    tr_debug("M2MResource::handle_put_request() - Query %s", query);
                    // if anything was updated, re-initialize the stored notification attributes
                    if (!handle_observation_attribute(query)){
                        tr_debug("M2MResource::handle_put_request() - Invalid query");
                        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
                    }
                    memory_free(query);
                }
            } else if ((operation() & SN_GRS_PUT_ALLOWED) != 0) {
                if(!content_type_present &&
                   (M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE ||
                    M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE_OLD)) {
                    coap_content_type = COAP_CONTENT_OMA_TLV_TYPE;
                }

                tr_debug("M2MResource::handle_put_request() - Request Content-Type %d", coap_content_type);

                if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type ||
                   COAP_CONTENT_OMA_TLV_TYPE_OLD == coap_content_type) {
                    set_coap_content_type(coap_content_type);
                    M2MTLVDeserializer::Error error = M2MTLVDeserializer::None;
                    error = M2MTLVDeserializer::deserialize_resource_instances(received_coap_header->payload_ptr,
                                                                         received_coap_header->payload_len,
                                                                         *this,
                                                                         M2MTLVDeserializer::Put);
                    switch(error) {
                        case M2MTLVDeserializer::None:
                            if(observation_handler) {
                                execute_value_updated = true;
                            }
                            msg_code = COAP_MSG_CODE_RESPONSE_CHANGED;
                            break;
                        case M2MTLVDeserializer::NotFound:
                            msg_code = COAP_MSG_CODE_RESPONSE_NOT_FOUND;
                            break;
                        case M2MTLVDeserializer::NotAllowed:
                            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
                            break;
                        case M2MTLVDeserializer::NotValid:
                            msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
                            break;
                        case M2MTLVDeserializer::OutOfMemory:
                            msg_code = COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE;
                            break;
                    }
                } else {
                    msg_code =COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT;
                } // if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type)
            } else {
                // Operation is not allowed.
                tr_error("M2MResource::handle_put_request() - COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
                msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
            }
        } else {
            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
        }
        if(coap_response) {
            coap_response->msg_code = msg_code;
        }
    } else {
        coap_response = M2MResourceBase::handle_put_request(nsdl,
                            received_coap_header,
                            observation_handler,
                            execute_value_updated);
    }
    return coap_response;
}


sn_coap_hdr_s* M2MResource::handle_post_request(nsdl_s *nsdl,
                                                sn_coap_hdr_s *received_coap_header,
                                                M2MObservationHandler */*observation_handler*/,
                                                bool &/*execute_value_updated*/,
                                                sn_nsdl_addr_s *address)
{
    tr_debug("M2MResource::handle_post_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 2.04
    sn_coap_hdr_s * coap_response = sn_nsdl_build_response(nsdl,
                                                           received_coap_header,
                                                           msg_code);
    // process the POST if we have registered a callback for it
    if(received_coap_header) {
        if ((operation() & SN_GRS_POST_ALLOWED) != 0) {
            M2MResource::M2MExecuteParameter exec_params(object_id(), object_instance_id(), name_id());

            uint16_t coap_content_type = 0;
            exec_params._token = received_coap_header->token_ptr;
            exec_params._token_length = received_coap_header->token_len;
            if(received_coap_header->payload_ptr) {
                if(received_coap_header->content_format != COAP_CT_NONE) {
                    coap_content_type = received_coap_header->content_format;
                }

                if(coap_content_type == COAP_CT_TEXT_PLAIN) {
                    exec_params._value = received_coap_header->payload_ptr;
                    if (exec_params._value) {
                        exec_params._value_length = received_coap_header->payload_len;
                    }
                } else {
                    msg_code = COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT;
                }                
            }
            if(COAP_MSG_CODE_RESPONSE_CHANGED == msg_code) {
                tr_debug("M2MResource::handle_post_request - Execute resource function");
                msg_code = COAP_MSG_CODE_EMPTY;
                coap_response->msg_type = COAP_MSG_TYPE_ACKNOWLEDGEMENT;
                coap_response->msg_code = msg_code;
                coap_response->msg_id = received_coap_header->msg_id;
                if(received_coap_header->token_len) {
                    sn_nsdl_send_coap_message(nsdl, address, coap_response);
                }
                execute(&exec_params);
            }

        } else { // if ((object->operation() & SN_GRS_POST_ALLOWED) != 0)
            tr_error("M2MResource::handle_post_request - COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED; // 4.05
        }
    } else { //if(object && received_coap_header)
        tr_error("M2MResource::handle_post_request - COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
        msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED; // 4.01
    }
    if(coap_response) {
        coap_response->msg_code = msg_code;
    }
    return coap_response;
}

M2MObjectInstance& M2MResource::get_parent_object_instance() const
{
    return _parent;
}

uint16_t M2MResource::object_instance_id() const
{
    const M2MObjectInstance& parent_object_instance = get_parent_object_instance();
    return parent_object_instance.instance_id();
}

M2MResource& M2MResource::get_parent_resource() const
{
    return (M2MResource&)*this;
}

uint16_t M2MResource::object_id() const
{
    const M2MObjectInstance& parent_object_instance = _parent;
    const M2MObject& parent_object = parent_object_instance.get_parent_object();

    return parent_object.name_id();
}

void M2MResource::uri_path(path_buffer &buffer) const
{
    buffer.append_int(object_id());
    buffer.append("/");
    buffer.append_int(object_instance_id());
    buffer.append("/");
    buffer.append_int(name_id());
}

M2MResource::M2MExecuteParameter::M2MExecuteParameter(uint16_t object_id, uint16_t object_instance_id,
                                                        uint16_t resource_id) :
_value(NULL),
_value_length(0),
_object_id(object_id),
_object_instance_id(object_instance_id),
_resource_id(resource_id)
{
}

// These could be actually changed to be inline ones, as it would likely generate
// smaller code at application side.

const uint8_t *M2MResource::M2MExecuteParameter::get_argument_value() const
{
    return _value;
}

const uint8_t *M2MResource::M2MExecuteParameter::get_token() const
{
    return _token;
}

uint8_t M2MResource::M2MExecuteParameter::get_token_length() const
{
    return _token_length;
}

uint16_t M2MResource::M2MExecuteParameter::get_argument_value_length() const
{
    return _value_length;
}

uint16_t M2MResource::M2MExecuteParameter::get_argument_object_id() const
{
    return _object_id;
}

uint16_t M2MResource::M2MExecuteParameter::get_argument_object_instance_id() const
{
    return _object_instance_id;
}

uint16_t M2MResource::M2MExecuteParameter::get_argument_resource_id() const
{
    return _resource_id;
}

