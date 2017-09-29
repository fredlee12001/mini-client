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
#include "mbed-client/m2mobjectinstance.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mobservationhandler.h"
#include "mbed-client/m2mstring.h"
#include "mbed-client/m2mstringbuffer.h"
#include "include/m2mtlvserializer.h"
#include "include/m2mtlvdeserializer.h"
#include "include/m2mreporthandler.h"
#include "mbed-trace/mbed_trace.h"

#include <stdlib.h>

#define BUFFER_SIZE 10
#define TRACE_GROUP "mClt"

M2MObjectInstance::M2MObjectInstance(M2MObject& parent,
                                     uint16_t name_id,
                                     char *path,
                                     bool external_blockwise_store)
: M2MBase(M2MBase::ObjectInstance,
          name_id,
          M2MBase::Dynamic,
          NULL,
          path,
          external_blockwise_store,
          false),
  _parent(parent),
  _next(NULL),
  _resource_list(NULL)
{
    M2MBase::set_coap_content_type(COAP_CONTENT_OMA_TLV_TYPE_OLD);
    M2MBase::set_operation(M2MBase::GET_ALLOWED);
}

M2MObjectInstance::~M2MObjectInstance()
{
    M2MResource* res = _resource_list;

    while (res) {
        M2MResource *next = res->get_next();

        //Free allocated memory for resources.
        delete res;

        res = next;
    }
    // remove the nsdl structures from the nsdlinterface's lists.
    M2MObservationHandler* obs_handler = observation_handler();
    if (obs_handler) {
        tr_debug("M2MBase::free_resources()");
        obs_handler->resource_to_be_deleted(this);
    }
    free_resources();
}

M2MResource* M2MObjectInstance::create_static_resource(uint16_t resource_id,
                                                       const char *resource_type,
                                                       M2MResourceInstance::ResourceType type,
                                                       const uint8_t *value,
                                                       const uint8_t value_length,
                                                       bool multiple_instance,
                                                       bool external_blockwise_store)
{
    tr_debug("M2MObjectInstance::create_static_resource(resource_id %d)", resource_id);
    M2MResource *res = NULL;
    if(!resource(resource_id)) {

        res = new M2MResource(*this, resource_id, M2MBase::Static, resource_type, convert_resource_type(type),
                              false, NULL, multiple_instance, external_blockwise_store);
        if(res) {
            if(!res->allocate_resources(value, value_length)) {
                delete res;
                return NULL;
            }
            res->add_observation_level(observation_level());
            if (multiple_instance) {
                res->set_coap_content_type(COAP_CONTENT_OMA_TLV_TYPE_OLD);
            }
            add_resource(res);
        }
    }
    return res;
}

M2MResource* M2MObjectInstance::create_dynamic_resource(uint16_t resource_id,
                                                const char *resource_type,
                                                M2MResourceInstance::ResourceType type,
                                                bool observable,
                                                bool multiple_instance,
                                                bool external_blockwise_store)
{
    tr_debug("M2MObjectInstance::create_dynamic_resource(resource_id %d)", resource_id);
    M2MResource *res = NULL;
    if(!resource(resource_id)) {
        res = new M2MResource(*this, resource_id, M2MBase::Dynamic, resource_type, convert_resource_type(type),
                              observable, NULL,
                              multiple_instance, external_blockwise_store);
        if(res) {
            if (multiple_instance) {
                res->set_coap_content_type(COAP_CONTENT_OMA_TLV_TYPE_OLD);
            }
            res->add_observation_level(observation_level());
            add_resource(res);
        }
    }
    return res;
}

M2MResourceInstance* M2MObjectInstance::create_static_resource_instance(uint16_t resource_id,
                                                                        const char *resource_type,
                                                                        M2MResourceInstance::ResourceType type,
                                                                        const uint8_t *value,
                                                                        const uint8_t value_length,
                                                                        uint16_t instance_id,
                                                                        bool external_blockwise_store)
{
    tr_debug("M2MObjectInstance::create_static_resource_instance(resource_id %d)", resource_id);
    M2MResourceInstance *instance = NULL;
    M2MResource *res = resource(resource_id);
    if(!res) {
        res = new M2MResource(*this, resource_id, M2MBase::Static, resource_type, convert_resource_type(type),
                              false, NULL, true, external_blockwise_store);
        if(res) {
            if(!res->allocate_resources(value, value_length)) {
                delete res;
                return NULL;
            }
            add_resource(res);
            res->set_operation(M2MBase::GET_ALLOWED);
            res->set_observable(false);
            res->set_register_uri(false);
        }
    }
    if(res && res->supports_multiple_instances()&& (res->resource_instance(instance_id) == NULL)) {
        instance = new M2MResourceInstance(*res, instance_id, M2MBase::Static, resource_type, convert_resource_type(type),
                                           NULL, external_blockwise_store,true);
        if(instance) {
            if(!instance->allocate_resources(value, value_length)) {
                delete instance;
                return NULL;
            }

            instance->set_operation(M2MBase::GET_ALLOWED);
            res->add_resource_instance(instance);
        }
    }
    return instance;
}


M2MResourceInstance* M2MObjectInstance::create_dynamic_resource_instance(uint16_t resource_id,
                                                                         const char *resource_type,
                                                                         M2MResourceInstance::ResourceType type,
                                                                         bool observable,
                                                                         uint16_t instance_id,
                                                                         bool external_blockwise_store)
{
    tr_debug("M2MObjectInstance::create_dynamic_resource_instance(resource_id %d)", resource_id);
    M2MResourceInstance *instance = NULL;
    M2MResource *res = resource(resource_id);
    if(!res) {    
        res = new M2MResource(*this, resource_id, M2MBase::Dynamic, resource_type, convert_resource_type(type),
                              false, NULL, true, external_blockwise_store);
        add_resource(res);
        res->set_register_uri(false);
        res->set_operation(M2MBase::GET_ALLOWED);
    }
    if (res && res->supports_multiple_instances() && (res->resource_instance(instance_id) == NULL)) {        
        instance = new M2MResourceInstance(*res, instance_id, M2MBase::Dynamic, resource_type, convert_resource_type(type),
                                           NULL, external_blockwise_store,true);
        if(instance) {
            instance->set_operation(M2MBase::GET_ALLOWED);
            instance->set_observable(observable);
            res->add_resource_instance(instance);
        }
    }
    return instance;
}

void M2MObjectInstance::add_resource(M2MResource *resource)
{
    tr_debug("M2MObjectInstance::add_resource()");

    if (resource) {
        // verify, that the item is not already part of some list or this will create eternal loop
        assert(resource->get_next() == NULL);

        // We keep the natural order of instances by adding to the end of list, which makes
        // this a O(N) operation instead of O(1) as if we added to the head of list. This
        // is not strictly speaking necessary but it is the old behavior.
        M2MResource* last = _resource_list;

        if (last == NULL) {
            _resource_list = resource;
        } else {
            // loop until we find the last one
            while ((last != NULL) && (last->get_next() != NULL)) {
                last = last->get_next();
                assert(last != resource); // verify that we are not adding the same object there twice
            }
            last->set_next(resource);
            resource->set_next(NULL);
        }
    }
}

bool M2MObjectInstance::remove_resource(uint16_t resource_id)
{
    tr_debug("M2MObjectInstance::remove_resource(resource_id %d)", resource_id);

    bool success = false;

    M2MResource *res = _resource_list;
    M2MResource *prev = NULL;

    while (res) {
        M2MResource *next = res->get_next();

        if (res->name_id() == resource_id) {
            // Resource found so lets delete it

            if (prev == NULL) {
                // this was the first item, we need to update things differently
                _resource_list = next;
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

bool M2MObjectInstance::remove_resource_instance(uint16_t resource_id,
                                                 uint16_t inst_id)
{
    tr_debug("M2MObjectInstance::remove_resource_instance(resource_id %d inst_id %d)",
             resource_id, inst_id);
    bool success = false;
    M2MResource *res = resource(resource_id);
    if (res) {
        success = res->remove_resource_instance(inst_id);

        // Note: this also implicitly deletes the resource if it lost all its instances
        if (res->resource_instance_count() == 0) {

            remove_resource(resource_id);
        }
    }
    return success;
}

M2MResource* M2MObjectInstance::resource(uint16_t resource_id) const
{
    M2MResource *found_res = NULL;
    M2MResource *res = _resource_list;
    while (res) {
        if (res->name_id() == resource_id) {
            // Resource found.
            found_res = res;
            break;
        }
        res = res->get_next();
    }
    return found_res;
}

const M2MResource* M2MObjectInstance::resources() const
{
    return _resource_list;
}

M2MResource* M2MObjectInstance::resources()
{
    return _resource_list;
}


uint16_t M2MObjectInstance::total_resource_count() const
{
    uint16_t count = 0;
    const M2MResource *res = _resource_list;

    while (res) {

        count++;

        res = res->get_next();
    }

    return count;
}

uint16_t M2MObjectInstance::resource_count() const
{
    // since the instance count is not stored anywhere (to save memory), it needs
    // to be calculated at run time, which is a O(N) operation.

    uint16_t count = 0;
    const M2MResource *res = _resource_list;

    while (res) {

        if (res->supports_multiple_instances()) {
            count += res->resource_instance_count();
        } else {
            count++;
        }

        res = res->get_next();
    }

    return count;
}

uint16_t M2MObjectInstance::resource_count(uint16_t resource_id) const
{
    uint16_t count = 0;
    const M2MResource *res = _resource_list;

    while (res) {

        // XXX: is this really correct? There may not be multiple resources with same id?! Why not break the loop
        if (res->name_id() == resource_id) {
            if (res->supports_multiple_instances()) {
                count += res->resource_instance_count();
            } else {
                count++;
            }
        }

        res = res->get_next();
    }

    return count;
}

M2MObservationHandler* M2MObjectInstance::observation_handler() const
{
    // XXX: need to check the flag too
    return _parent.observation_handler();
}

void M2MObjectInstance::set_observation_handler(M2MObservationHandler *handler)
{
    // XXX: need to set the flag too
    _parent.set_observation_handler(handler);
}

void M2MObjectInstance::add_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::add_observation_level(observation_level);

    M2MResource *res = _resource_list;

    while (res) {

        res->add_observation_level(observation_level);

        res = res->get_next();
    }
}

void M2MObjectInstance::remove_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::remove_observation_level(observation_level);

    M2MResource *res = _resource_list;

    while (res) {

        res->remove_observation_level(observation_level);

        res = res->get_next();
    }
}

sn_coap_hdr_s* M2MObjectInstance::handle_get_request(nsdl_s *nsdl,
                                                     sn_coap_hdr_s *received_coap_header,
                                                     M2MObservationHandler *observation_handler)
{
    tr_debug("M2MObjectInstance::handle_get_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
    sn_coap_hdr_s * coap_response = sn_nsdl_build_response(nsdl,
                                                           received_coap_header,
                                                           msg_code);
    uint8_t * data = NULL;
    uint32_t  data_length = 0;

    if (received_coap_header) {
        // process the GET if we have registered a callback for it
        if ((operation() & SN_GRS_GET_ALLOWED) != 0) {
            if (coap_response) {
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
                    if (!content_type_present &&
                       (M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE ||
                        M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE_OLD)) {
                        coap_response->content_format = sn_coap_content_format_e(M2MBase::coap_content_type());
                    }

                    tr_debug("M2MObjectInstance::handle_get_request() - Request Content-Type %d", coap_response->content_format );

                    // fill in the CoAP response payload
                    if (COAP_CONTENT_OMA_TLV_TYPE == coap_response->content_format  ||
                        COAP_CONTENT_OMA_TLV_TYPE_OLD == coap_response->content_format) {
                        set_coap_content_type(coap_response->content_format);
                    data = M2MTLVSerializer::serialize_list(_resource_list, data_length);
                    }

                    coap_response->payload_len = data_length;
                    coap_response->payload_ptr = data;

                    if (data) {
                        coap_response->options_list_ptr = sn_nsdl_alloc_options_list(nsdl, coap_response);

                        coap_response->options_list_ptr->max_age = max_age();

                        if (received_coap_header->options_list_ptr) {
                            if (received_coap_header->options_list_ptr->observe != -1) {
                                if (is_observable()) {
                                    uint32_t number = 0;
                                    uint8_t observe_option = 0;
                                    observe_option = received_coap_header->options_list_ptr->observe;
                                    if (START_OBSERVATION == observe_option) {
                                        tr_debug("M2MObjectInstance::handle_get_request - Starts Observation");
                                        // If the observe length is 0 means register for observation.
                                        if (received_coap_header->options_list_ptr->observe != -1) {
                                            number = received_coap_header->options_list_ptr->observe;
                                        }

                                        // If the observe value is 0 means register for observation.
                                        if (number == 0) {
                                            tr_debug("M2MObjectInstance::handle_get_request - Put Resource under Observation");
                                            set_under_observation(true,observation_handler);
                                            add_observation_level(M2MBase::OI_Attribute);
                                            coap_response->options_list_ptr->observe = observation_number();
                                        }

                                        if (received_coap_header->token_ptr) {
                                            tr_debug("M2MObjectInstance::handle_get_request - Sets Observation Token to resource");
                                            set_observation_token(received_coap_header->token_ptr,
                                                                  received_coap_header->token_len);
                                        }

                                    } else if (STOP_OBSERVATION == observe_option) {
                                        tr_debug("M2MObjectInstance::handle_get_request - Stops Observation");
                                        set_under_observation(false,NULL);
                                        remove_observation_level(M2MBase::OI_Attribute);

                                    }
                                    msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                                }
                                else {
                                    msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
                                }
                            }
                        }
                    } else {
                        msg_code = COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT; // Content format not supported
                    }
                } else {
                    tr_error("M2MObjectInstance::handle_get_request() - Content-Type %d not supported", coap_response->content_format);
                    msg_code = COAP_MSG_CODE_RESPONSE_NOT_ACCEPTABLE;
                }
            }
        } else {
            tr_error("M2MObjectInstance::handle_get_request - Return COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
            // Operation is not allowed.
            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
        }
    } else {
        msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
    }
    if(coap_response) {
        coap_response->msg_code = msg_code;
    }
    return coap_response;
}

sn_coap_hdr_s* M2MObjectInstance::handle_put_request(nsdl_s *nsdl,
                                                     sn_coap_hdr_s *received_coap_header,
                                                     M2MObservationHandler *observation_handler,
                                                     bool &/*execute_value_updated*/)
{
    tr_debug("M2MObjectInstance::handle_put_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 2.04
    sn_coap_hdr_s * coap_response = sn_nsdl_build_response(nsdl,
                                                           received_coap_header,
                                                           msg_code);;
    if(received_coap_header) {
        uint16_t coap_content_type = 0;
        bool content_type_present = false;

        if(received_coap_header->content_format != COAP_CT_NONE) {
            content_type_present = true;
            set_coap_content_type(received_coap_header->content_format);
            if(coap_response) {
                coap_content_type = received_coap_header->content_format;
            }
        }
        if(received_coap_header->options_list_ptr &&
           received_coap_header->options_list_ptr->uri_query_ptr) {
            char *query = (char*)alloc_string_copy(received_coap_header->options_list_ptr->uri_query_ptr,
                                                    received_coap_header->options_list_ptr->uri_query_len);
            if (query){
                tr_debug("M2MObjectInstance::handle_put_request() - Query %s", query);
                // if anything was updated, re-initialize the stored notification attributes
                if (!handle_observation_attribute(query)){
                    tr_debug("M2MObjectInstance::handle_put_request() - Invalid query");
                    msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
                } else {
                    msg_code =COAP_MSG_CODE_RESPONSE_CHANGED;
                }
                memory_free(query);
            }
        } else if ((operation() & SN_GRS_PUT_ALLOWED) != 0) {
            if(!content_type_present &&
               (M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE ||
                M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE_OLD)) {
                coap_content_type = M2MBase::coap_content_type();
            }

            tr_debug("M2MObjectInstance::handle_put_request() - Request Content-Type %d", coap_content_type);

            if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type ||
               COAP_CONTENT_OMA_TLV_TYPE_OLD == coap_content_type ) {
                set_coap_content_type(coap_content_type);
                M2MTLVDeserializer::Error error = M2MTLVDeserializer::None;
                if(received_coap_header->payload_ptr) {
                    error = M2MTLVDeserializer::deserialize_resources(
                                received_coap_header->payload_ptr,
                                received_coap_header->payload_len, *this,
                                M2MTLVDeserializer::Put);
                    switch(error) {
                        case M2MTLVDeserializer::None:
                            if(observation_handler) {
                                observation_handler->value_updated(this);
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
                }
            } else {
                msg_code =COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT;
            } // if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type)
        } else {
            // Operation is not allowed.
            tr_error("M2MObjectInstance::handle_put_request() - COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
        }
    } else {
       msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
    }
    if(coap_response) {
        coap_response->msg_code = msg_code;
    }
    return coap_response;
}




sn_coap_hdr_s* M2MObjectInstance::handle_post_request(nsdl_s *nsdl,
                                                      sn_coap_hdr_s *received_coap_header,
                                                      M2MObservationHandler *observation_handler,
                                                      bool &execute_value_updated,
                                                      sn_nsdl_addr_s *)
{
    tr_debug("M2MObjectInstance::handle_post_request()");
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 2.04
    sn_coap_hdr_s * coap_response = sn_nsdl_build_response(nsdl,
                                                           received_coap_header,
                                                           msg_code);
    if(received_coap_header) {
        if ((operation() & SN_GRS_POST_ALLOWED) != 0) {
            uint16_t coap_content_type = 0;
            bool content_type_present = false;
            if(received_coap_header->content_format != COAP_CT_NONE) {
                set_coap_content_type(received_coap_header->content_format);
                content_type_present = true;
                if(coap_response) {
                    coap_content_type = received_coap_header->content_format;
                }
            }
            if(!content_type_present &&
               (M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE ||
                M2MBase::coap_content_type() == COAP_CONTENT_OMA_TLV_TYPE_OLD)) {
                coap_content_type = M2MBase::coap_content_type();
            }

            tr_debug("M2MObjectInstance::handle_post_request() - Request Content-Type %d", coap_content_type);

            if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type ||
               COAP_CONTENT_OMA_TLV_TYPE_OLD == coap_content_type) {
                set_coap_content_type(coap_content_type);
                M2MTLVDeserializer::Error error = M2MTLVDeserializer::None;
                error = M2MTLVDeserializer::deserialize_resources(
                            received_coap_header->payload_ptr,
                            received_coap_header->payload_len, *this,
                            M2MTLVDeserializer::Post);

                uint16_t instance_id = M2MTLVDeserializer::instance_id(received_coap_header->payload_ptr);
                switch(error) {
                    case M2MTLVDeserializer::None:
                        if(observation_handler) {
                            execute_value_updated = true;
                        }
                        coap_response->options_list_ptr = sn_nsdl_alloc_options_list(nsdl, coap_response);

                        if (coap_response->options_list_ptr) {

                            StringBuffer<MAX_PATH_SIZE_3> obj_name;
                            if(!build_path(obj_name, _parent.name_id(), M2MBase::instance_id(), instance_id)) {
                                msg_code = COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR;
                                break;
                            }

                            coap_response->options_list_ptr->location_path_len = obj_name.get_size();
                            coap_response->options_list_ptr->location_path_ptr =
                              alloc_string_copy((uint8_t*)obj_name.c_str(),
                                                coap_response->options_list_ptr->location_path_len);
                            // todo: handle allocation error
                        }
                        msg_code = COAP_MSG_CODE_RESPONSE_CREATED;
                        break;
                    case M2MTLVDeserializer::NotAllowed:
                        msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
                        break;
                    case M2MTLVDeserializer::NotValid:
                        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
                        break;
                    case M2MTLVDeserializer::NotFound:
                        msg_code = COAP_MSG_CODE_RESPONSE_NOT_FOUND;
                        break;
                    case M2MTLVDeserializer::OutOfMemory:
                        msg_code = COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE;
                        break;
                    default:
                        break;
                }
            } else {
                msg_code =COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT;
            } // if(COAP_CONTENT_OMA_TLV_TYPE == coap_content_type)
        } else {
            // Operation is not allowed.
            tr_error("M2MObjectInstance::handle_post_request() - COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED");
            msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
        }
    } else {
        msg_code = COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED;
    }
    if(coap_response) {
        coap_response->msg_code = msg_code;
    }
    return coap_response;
}

void M2MObjectInstance::notification_update(M2MBase::Observation observation_level)
{
    tr_debug("M2MObjectInstance::notification_update() - level(%d)", observation_level);
    if((M2MBase::O_Attribute & observation_level) == M2MBase::O_Attribute) {
        tr_debug("M2MObjectInstance::notification_update() - object callback");
        _parent.notification_update(instance_id());
    }
    if((M2MBase::OI_Attribute & observation_level) == M2MBase::OI_Attribute) {
        tr_debug("M2MObjectInstance::notification_update() - object instance callback");
        M2MReportHandler *report_handler = M2MBase::report_handler();
        if(report_handler && is_under_observation()) {
            report_handler->set_notification_trigger();
        }

    }
}

M2MBase::DataType M2MObjectInstance::convert_resource_type(M2MResourceInstance::ResourceType type)
{
    M2MBase::DataType data_type = M2MBase::OBJLINK;
    switch(type) {
        case M2MResourceInstance::STRING:
            data_type = M2MBase::STRING;
            break;
        case M2MResourceInstance::INTEGER:
            data_type = M2MBase::INTEGER;
            break;
        case M2MResourceInstance::FLOAT:
            data_type = M2MBase::FLOAT;
            break;
        case M2MResourceInstance::OPAQUE:
            data_type = M2MBase::OPAQUE;
            break;
        case M2MResourceInstance::BOOLEAN:
            data_type = M2MBase::BOOLEAN;
            break;
        case M2MResourceInstance::TIME:
            data_type = M2MBase::TIME;
            break;
        case M2MResourceInstance::OBJLINK:
            data_type = M2MBase::OBJLINK;
            break;
    }
    return data_type;
}

void M2MObjectInstance::uri_path(path_buffer &buffer) const
{
    buffer.append_int(_parent.name_id());
    buffer.append("/");
    buffer.append_int(name_id());
}
