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

// Note: this macro is needed on armcc to get the the limit macros like UINT16_MAX
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/nsdlaccesshelper.h"
#include "include/m2mnsdlobserver.h"
#include "include/m2mtlvdeserializer.h"
#include "include/m2mtlvserializer.h"
#include "include/m2mnsdlinterface.h"
#include "include/uriqueryparser.h"
#include "mbed-client/m2mbase.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mobjectinstance.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mblockmessage.h"
#include "mbed-client/m2mstringbuffer.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-client/m2minterfacefactory.h"
#include "m2mconfig.h"
#include "mbed-client/m2mdevice.h"

#define TRACE_GROUP "mClt"
#define MAX_QUERY_COUNT 10

M2MNsdlInterface::M2MNsdlInterface(M2MNsdlObserver &observer, M2MConnectionHandler &connection_handler)
: _observer(observer),
  _object_list(NULL),
  _nsdl_handle(NULL),
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
  _security(NULL),
#endif
  _server(NULL),
  _nsdl_exceution_timer(*this),
  _registration_timer(*this),
  _connection_handler(connection_handler),
  _counter_for_nsdl(0),
  _bootstrap_id(0),
  _unregister_ongoing(false),
  _identity_accepted(false),
  _nsdl_exceution_timer_running(false),
  _binding_mode(M2MInterface::NOT_SET)
{
    tr_debug("M2MNsdlInterface::M2MNsdlInterface()");
    memset(&_endpoint, 0, sizeof(sn_nsdl_ep_parameters_s));
    memset(_endpoint_name,0, sizeof(_endpoint_name));
    memset(_endpoint_name_ptr,0, sizeof(_endpoint_name_ptr));
    memset(_endpoint_type_ptr,0, sizeof(_endpoint_type_ptr));
    memset(_domain_ptr,0, sizeof(_domain_ptr));
}

M2MNsdlInterface::~M2MNsdlInterface()
{
    tr_debug("M2MNsdlInterface::~M2MNsdlInterface() - IN");
    _object_list = NULL;
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    delete _security;
#endif
    sn_nsdl_destroy(_nsdl_handle);
    _nsdl_handle = NULL;
    delete _server;
    tr_debug("M2MNsdlInterface::~M2MNsdlInterface() - OUT");
}

bool M2MNsdlInterface::create_endpoint(const char *name,
                                       const char *type,
                                       const int32_t life_time,
                                       const char *domain,
                                       const uint8_t mode,
                                       const char */*context_address*/)
{
    tr_debug("M2MNsdlInterface::create_endpoint( name %s type %s lifetime %" PRId32 ", domain %s, mode %d)",
              name, type, life_time, domain, mode);
    // This initializes libCoap and libNsdl
    // Parameters are function pointers to used memory allocation
    // and free functions in structure and used functions for sending
    // and receiving purposes.
    _nsdl_handle = sn_nsdl_init(&(__nsdl_c_send_to_server), &(__nsdl_c_received_from_server),
                 &(__nsdl_c_memory_alloc), &(__nsdl_c_memory_free),
                 &(__get_first), &(__get_next), &(__find_resource),&(__nsdl_c_callback));

    if(_nsdl_handle) {

#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
        _security = M2MInterfaceFactory::create_security(M2MSecurity::M2MServer);
        if(!_security) {
            return false;
        }
        _server = M2MInterfaceFactory::create_server();
        if(!_server) {
            delete _security;
            return false;
        }
#endif
        sn_nsdl_set_context(_nsdl_handle, this);
        //Sets the packet retransmission attempts and time interval
        sn_nsdl_set_retransmission_parameters(_nsdl_handle,
                                              MBED_CLIENT_RECONNECTION_COUNT,
                                              MBED_CLIENT_RECONNECTION_INTERVAL);

        _binding_mode = mode;

        memcpy(_endpoint_name, (uint8_t*)name, strlen(name));

        memcpy(_endpoint_name_ptr, (uint8_t*)name, strlen(name));
        _endpoint.endpoint_name_ptr = _endpoint_name_ptr;
        _endpoint.endpoint_name_len = strlen(name);

        if(type) {
            memcpy(_endpoint_type_ptr, (uint8_t*)type, strlen(type));
            _endpoint.type_ptr = _endpoint_type_ptr;
            _endpoint.type_len = strlen(type);
        }
        if(domain) {
            memcpy(_domain_ptr, (uint8_t*)domain, strlen(domain));
            _endpoint.domain_name_ptr = _domain_ptr;
            _endpoint.domain_name_len = strlen(domain);
        }

        // nsdl binding mode is only 3 least significant bits
        _endpoint.binding_and_mode = (sn_nsdl_oma_binding_and_mode_t)((uint8_t)mode & 0x07);

        // If lifetime is less than zero then leave the field empty
        if( life_time > 0) {
            set_endpoint_lifetime_buffer(life_time);
            return true;
        }

    }
    return false;
}

void M2MNsdlInterface::update_endpoint(const char *name)
{
    memcpy(_endpoint_name, (uint8_t*)name, strlen(name));
    memcpy(_endpoint_name_ptr, (uint8_t*) name, strlen(name));
    _endpoint.endpoint_name_ptr = _endpoint_name_ptr;
    _endpoint.endpoint_name_len = strlen(name);
}

void M2MNsdlInterface::update_domain(const char *domain)
{
    memcpy(_domain_ptr, (uint8_t*)domain, strlen(domain));
    _endpoint.domain_name_ptr = _domain_ptr;
    _endpoint.domain_name_len = strlen(domain);
}

void M2MNsdlInterface::set_endpoint_lifetime_buffer(int lifetime)
{
    uint32_t size = m2m::itoa_c(lifetime,(char*)_lifetime);
    tr_debug("M2MNsdlInterface::set_endpoint_lifetime_buffer, lifetime %d", lifetime);

    _endpoint.lifetime_ptr = _lifetime;
    _endpoint.lifetime_len =  size;
}


bool M2MNsdlInterface::create_nsdl_list_structure(const M2MObject *object_list)
{
    tr_debug("M2MNsdlInterface::create_nsdl_list_structure()");
    bool success = false;
    const M2MObject *object = object_list;
    add_object_to_list((M2MObject*)object);
    while (object) {
        const M2MObject* next = object->get_next();
        success = create_nsdl_object_structure(object);
        object = next;
    }
    return success;
}

bool M2MNsdlInterface::remove_nsdl_resource(M2MBase */*base*/)
{    
    return true;
}

bool M2MNsdlInterface::create_bootstrap_resource(sn_nsdl_addr_s *address)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MNsdlInterface::create_bootstrap_resource()");
    _identity_accepted = false;
    bool success = false;
    sn_nsdl_bs_ep_info_t bootstrap_endpoint;
    tr_debug("M2MNsdlInterface::create_bootstrap_resource() - endpoint name: %.*s", _endpoint.endpoint_name_len,
             _endpoint.endpoint_name_ptr);

    if(_bootstrap_id == 0) {
        // Take copy of the address, uri_query_parameters() will modify the source buffer
        bool msg_sent = false;
        char address_copy[MAX_VALUE_LENGTH];
        strncpy(address_copy, _server_address, MAX_VALUE_LENGTH);
        char* query = query_string(_server_address);
        if (query != NULL) {
            int param_count = query_param_count(query);
            if (param_count) {
                char* uri_query_params[MAX_QUERY_COUNT];
                if (uri_query_parameters(query, uri_query_params)) {
                    msg_sent = true;
                    _bootstrap_id = sn_nsdl_oma_bootstrap(_nsdl_handle,
                                                          address,
                                                          &_endpoint,
                                                          &bootstrap_endpoint,
                                                          uri_query_params,
                                                          param_count);
                }
                strncpy(_server_address, address_copy, MAX_VALUE_LENGTH);
            }
        }
        if (!msg_sent) {
            _bootstrap_id = sn_nsdl_oma_bootstrap(_nsdl_handle,
                                                  address,
                                                  &_endpoint,
                                                  &bootstrap_endpoint,
                                                  NULL,
                                                  0);
        }

        success = _bootstrap_id != 0;
        tr_debug("M2MNsdlInterface::create_bootstrap_resource - _bootstrap_id %d", _bootstrap_id);
    }
    return success;
#else
    (void)address;
    return false;
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MNsdlInterface::set_server_address(uint8_t* address,
                                          uint8_t address_length,
                                          const uint16_t port,
                                          sn_nsdl_addr_type_e address_type)
{
    tr_debug("M2MNsdlInterface::set_server_address()");
    set_NSP_address(_nsdl_handle, address, address_length, port, address_type);
}

bool M2MNsdlInterface::send_register_message()
{
    tr_debug("M2MNsdlInterface::send_register_message()");

    bool success = false;
    if (_server_address) {
        success = parse_and_send_uri_query_parameters();
    }
    // If URI parsing fails or there is no parameters, try again without parameters
    if (!success) {
        sn_nsdl_clear_coap_resending_queue(_nsdl_handle);
        success = sn_nsdl_register_endpoint(_nsdl_handle,&_endpoint, NULL, 0) != 0;
    }
    return success;
}

bool M2MNsdlInterface::send_update_registration(const uint32_t lifetime, bool clear_queue)
{
    tr_debug("M2MNsdlInterface::send_update_registration( lifetime %" PRIu32 ")", lifetime);
    bool success = false;
    create_nsdl_list_structure(_object_list);

    _registration_timer.stop_timer();

    //If Lifetime value is 0, then don't change the existing lifetime value
    if(lifetime != 0) {
        set_endpoint_lifetime_buffer(lifetime);

        tr_debug("M2MNsdlInterface::send_update_registration - new lifetime value");
        if(clear_queue) {
            sn_nsdl_clear_coap_resending_queue(_nsdl_handle);
        }
        success = sn_nsdl_update_registration(_nsdl_handle,
                                              _endpoint.lifetime_ptr,
                                              _endpoint.lifetime_len) != 0;
    } else {
        tr_debug("M2MNsdlInterface::send_update_registration - regular update");
        if(clear_queue) {
            sn_nsdl_clear_coap_resending_queue(_nsdl_handle);
        }
        success = sn_nsdl_update_registration(_nsdl_handle, NULL, 0) != 0;
    }

    _registration_timer.start_timer(registration_time() * 1000,
                                     M2MTimerObserver::Registration,
                                     false);

    return success;
}

bool M2MNsdlInterface::send_unregister_message()
{
    tr_debug("M2MNsdlInterface::send_unregister_message");
    if (_unregister_ongoing) {
        tr_debug("M2MNsdlInterface::send_unregister_message - unregistration already in progress");
        return true;
    }

    bool success = false;
    _unregister_ongoing = true;
    success = sn_nsdl_unregister_endpoint(_nsdl_handle) != 0;
    return success;
}

uint8_t M2MNsdlInterface::send_to_server_callback(struct nsdl_s * /*nsdl_handle*/,
                                                  sn_nsdl_capab_e /*protocol*/,
                                                  uint8_t *data_ptr,
                                                  uint16_t data_len,
                                                  sn_nsdl_addr_s *address)
{
    tr_debug("M2MNsdlInterface::send_to_server_callback()");
    _observer.coap_message_ready(data_ptr,data_len,address);
    return 1;
}

uint8_t M2MNsdlInterface::received_from_server_callback(struct nsdl_s *nsdl_handle,
                                                        sn_coap_hdr_s *coap_header,
                                                        sn_nsdl_addr_s *address)
{
    _observer.coap_data_processed();
    uint8_t value = 0;
    if(nsdl_handle && coap_header) {
        tr_debug("M2MNsdlInterface::received_from_server_callback - endpoint name : %s",_endpoint.endpoint_name_ptr);
        tr_debug("M2MNsdlInterface::received_from_server_callback - domain name :%s", _endpoint.domain_name_ptr);
        tr_debug("M2MNsdlInterface::received_from_server_callback - incoming msg id:%" PRIu16, coap_header->msg_id);
        tr_debug("M2MNsdlInterface::received_from_server_callback - incoming msg code:%d", coap_header->msg_code);
        tr_debug("M2MNsdlInterface::received_from_server_callback - incoming msg type:%d", coap_header->msg_type);
        if (coap_header->uri_path_ptr) {
            tr_debug("M2MNsdlInterface::received_from_server_callback - incoming msg uri:%.*s", coap_header->uri_path_len, coap_header->uri_path_ptr);
        }
        tr_debug("M2MNsdlInterface::received_from_server_callback - registration id:%" PRIu16, nsdl_handle->register_msg_id);
        tr_debug("M2MNsdlInterface::received_from_server_callback - unregistration id:%" PRIu16, nsdl_handle->unregister_msg_id);
        tr_debug("M2MNsdlInterface::received_from_server_callback - update registration id:%" PRIu16, nsdl_handle->update_register_msg_id);
        bool is_bootstrap_msg = address && (nsdl_handle->oma_bs_address_len == address->addr_len) &&
                                   (nsdl_handle->oma_bs_port == address->port) &&
                                   !memcmp(nsdl_handle->oma_bs_address_ptr, address->addr_ptr, nsdl_handle->oma_bs_address_len);

        if(coap_header->msg_id == nsdl_handle->register_msg_id) {
            if(coap_header->msg_code == COAP_MSG_CODE_RESPONSE_CREATED) {
                tr_debug("M2MNsdlInterface::received_from_server_callback - registration callback");

                // If lifetime is less than zero then leave the field empty
                if(coap_header->options_list_ptr) {
                    uint32_t max_time = coap_header->options_list_ptr->max_age;

                    // If a sufficiently-large Max-Age option is present, we interpret it as registration lifetime;
                    // mbed server (mDS) reports lifetime this way as a non-standard extension. Other servers
                    // would likely not include an explicit Max-Age option, in which case we'd see the default 60 seconds.
                    if( max_time >= MINIMUM_REGISTRATION_TIME) {
                        set_endpoint_lifetime_buffer(max_time);
                    }
                    if(coap_header->options_list_ptr->location_path_ptr) {
                        sn_nsdl_set_endpoint_location(_nsdl_handle,
                                                      coap_header->options_list_ptr->location_path_ptr,
                                                      coap_header->options_list_ptr->location_path_len);
                    }

                }
                _registration_timer.stop_timer();
                _registration_timer.start_timer(registration_time() * 1000,
                                                 M2MTimerObserver::Registration,
                                                 false);
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
                _observer.client_registered(_server);
#else
                _observer.client_registered(NULL);
#endif
            } else {
                tr_error("M2MNsdlInterface::received_from_server_callback - registration error %d", coap_header->msg_code);
                if(coap_header->coap_status == COAP_STATUS_BUILDER_MESSAGE_SENDING_FAILED) {
                    tr_error("M2MNsdlInterface::received_from_server_callback - message sending failed !!!!");
                }
                // Try to do clean register again
                if(COAP_MSG_CODE_RESPONSE_BAD_REQUEST == coap_header->msg_code ||
                   COAP_MSG_CODE_RESPONSE_FORBIDDEN == coap_header->msg_code) {
                    _observer.registration_error(M2MInterface::InvalidParameters, false);
                } else {
                    _observer.registration_error(M2MInterface::NetworkError, true);
                }

            }
        } else if(coap_header->msg_id == nsdl_handle->unregister_msg_id) {
            _unregister_ongoing = false;
            tr_debug("M2MNsdlInterface::received_from_server_callback - unregistration callback");
            if(coap_header->msg_code == COAP_MSG_CODE_RESPONSE_DELETED) {
                _registration_timer.stop_timer();
                _observer.client_unregistered();
            } else {
                tr_error("M2MNsdlInterface::received_from_server_callback - unregistration error %d", coap_header->msg_code);
                M2MInterface::Error error = M2MInterface::UnregistrationFailed;
                if (coap_header->msg_code == COAP_MSG_CODE_RESPONSE_NOT_FOUND) {
                    _observer.registration_error(error, false);
                } else {
                    _observer.registration_error(error, true);
                }
            }
        } else if(coap_header->msg_id == nsdl_handle->update_register_msg_id) {
            if(coap_header->msg_code == COAP_MSG_CODE_RESPONSE_CHANGED) {
                tr_debug("M2MNsdlInterface::received_from_server_callback - registration_updated successfully");
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
                _observer.registration_updated(*_server);
#else
                M2MServer server;
                _observer.registration_updated(server);
#endif
            } else {
                tr_error("M2MNsdlInterface::received_from_server_callback - registration_updated failed %d, %d", coap_header->msg_code, coap_header->coap_status);

                _registration_timer.stop_timer();
                if ((_binding_mode == M2MInterface::UDP ||
                    _binding_mode == M2MInterface::UDP_QUEUE ||
                    _binding_mode == M2MInterface::UDP_SMS_QUEUE) &&
                    coap_header->msg_code != COAP_MSG_CODE_RESPONSE_NOT_FOUND) {
                    _observer.registration_error(M2MInterface::NetworkError, true);
                } else {
                    bool msg_sent = false;
                    if (_server_address) {
                        msg_sent = parse_and_send_uri_query_parameters();
                    }
                    if (!msg_sent) {
                        sn_nsdl_clear_coap_resending_queue(_nsdl_handle);
                    sn_nsdl_register_endpoint(_nsdl_handle,&_endpoint, NULL, 0);
                    }
                }
            }
        }
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
        else if(coap_header->msg_id == nsdl_handle->bootstrap_msg_id) {
            tr_debug("M2MNsdlInterface::received_from_server_callback - bootstrap");
            _bootstrap_id = 0;
            M2MInterface::Error error = interface_error(*coap_header);
            if(error != M2MInterface::ErrorNone) {
                char buffer[MAX_ALLOWED_STRING_LENGTH];
                memset(buffer, 0, MAX_ALLOWED_STRING_LENGTH);

                const char* error= coap_error(*coap_header);
                memcpy(buffer,error,strlen(error));
                if(coap_header->payload_ptr) {
                    strncat(buffer,":",1);
                    strncat(buffer,(char*)coap_header->payload_ptr,coap_header->payload_len);
                }

                handle_bootstrap_error(buffer, false);
            } else {
                _identity_accepted = true;
            }
        }
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
        else {

            sn_coap_hdr_s *coap_response = NULL;
            bool execute_value_updated = false;
            M2MObjectInstance *obj_instance = NULL;
            char resource_name[MAX_VALUE_LENGTH];

            if(COAP_MSG_CODE_REQUEST_PUT == coap_header->msg_code) {
                if (is_bootstrap_msg) {
                    handle_bootstrap_put_message(coap_header, address);
                }
                else{
                    tr_debug("M2MNsdlInterface::received_from_server_callback - Method not allowed (PUT).");
                    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                           coap_header,
                                                           COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED);
                }
            }
            else if(COAP_MSG_CODE_REQUEST_DELETE == coap_header->msg_code) {
                if (is_bootstrap_msg) {
                    handle_bootstrap_delete(coap_header, address);
                }
                else{
                    tr_debug("M2MNsdlInterface::received_from_server_callback - Method not allowed (DELETE).");
                    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                           coap_header,
                                                           COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED);
                }
            }
            else if(COAP_MSG_CODE_REQUEST_POST == coap_header->msg_code) {
                if(is_bootstrap_msg) {
                    handle_bootstrap_finished(coap_header, address);
                }
                else if(coap_header->uri_path_ptr) {

                    memcpy(resource_name, (char*)coap_header->uri_path_ptr, coap_header->uri_path_len);

                    char *slash = strrchr(resource_name, '/');
                    int slash_found = slash - resource_name + 1;
                    char object_name[MAX_ALLOWED_STRING_LENGTH];
                    memcpy(object_name, coap_header->uri_path_ptr, coap_header->uri_path_len - strlen(slash));

                    //The POST operation here is only allowed for non-existing object instances
                    if(slash_found != -1) {
                        slash += 1;
                        char *last_slash = strrchr(slash, '/');
                        slash_found = last_slash - slash + 1;

                        if( slash_found != -1){
                            coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                                   coap_header,
                                                                   COAP_MSG_CODE_RESPONSE_NOT_FOUND);
                        } else {
                            last_slash += 1;
                            int32_t instance_id = atoi(last_slash);
                            M2MBase* base = find_resource(object_name, 0);
                            if(base && (instance_id >= 0) && (instance_id < UINT16_MAX)) {
                                if(coap_header->payload_ptr) {
                                    M2MObject* object = static_cast<M2MObject*> (base);
                                    obj_instance = object->create_object_instance(instance_id);
                                    if(obj_instance) {
                                        obj_instance->set_operation(M2MBase::GET_PUT_POST_ALLOWED);
                                        coap_response = obj_instance->handle_post_request(_nsdl_handle,
                                                                                          coap_header,
                                                                                          this,
                                                                                          execute_value_updated);
                                    }
                                    if(coap_response && coap_response->msg_code != COAP_MSG_CODE_RESPONSE_CREATED) {
                                        //Invalid request so remove created ObjectInstance
                                        object->remove_object_instance(instance_id);
                                    } else  {
                                        tr_debug("M2MNsdlInterface::received_from_server_callback - Send Update registration for Create");
                                        send_update_registration();
                                    }
                                } else {
                                    tr_debug("M2MNsdlInterface::received_from_server_callback - Missing Payload - Cannot create");
                                    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                                           coap_header,
                                                                           COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
                                }
                            } else { //if(base)
                                tr_debug("M2MNsdlInterface::received_from_server_callback - Missing BASE - Cannot create");
                                coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                                       coap_header,
                                                                       COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED);
                            }
                        }
                    } else{ // if(slash_found != -1)
                        tr_debug("M2MNsdlInterface::received_from_server_callback - slash_found - Cannot create");
                        coap_response = sn_nsdl_build_response(_nsdl_handle,
                                                               coap_header,
                                                               COAP_MSG_CODE_RESPONSE_NOT_FOUND);
                    }

                }
            }
            else if(COAP_MSG_CODE_EMPTY == coap_header->msg_code) {
                if (COAP_MSG_TYPE_RESET == coap_header->msg_type) {
                    // Cancel ongoing observation
                    tr_debug("M2MNsdlInterface::received_from_server_callback() - RESET message - id %d", coap_header->msg_id);
                    M2MBase *base = find_resource("", coap_header->msg_id);
                    if (base) {
                        M2MBase::BaseType type = base->base_type();
                        switch (type) {
                            case M2MBase::Object:
                                base->remove_observation_level(M2MBase::O_Attribute);
                                break;
                            case M2MBase::Resource:
                                base->remove_observation_level(M2MBase::R_Attribute);
                                break;
                            case M2MBase::ObjectInstance:
                                base->remove_observation_level(M2MBase::OI_Attribute);
                                break;
                            default:
                                break;
                        }
                        base->set_under_observation(false, this);
                    }
                } else {
                    tr_debug("M2MNsdlInterface::received_from_server_callback - Empty ACK, msg id: %d", coap_header->msg_id);
                    M2MBase *base = find_resource("", coap_header->msg_id);
                    if (base) {
                        // Supported only in Resource level
                        if (M2MBase::Resource == base->base_type()) {
                            M2MResource *resource = static_cast<M2MResource *> (base);
                            resource->notification_sent();
                        }
                    }
                }
            }

            if(coap_response) {
                tr_debug("M2MNsdlInterface::received_from_server_callback - send CoAP response");
                (sn_nsdl_send_coap_message(_nsdl_handle, address, coap_response) == 0) ? value = 0 : value = 1;
                sn_nsdl_release_allocated_coap_msg_mem(_nsdl_handle, coap_response);
            }

            if (execute_value_updated) {
                value_updated(obj_instance);
            }

        }
    }
    return value;
}

uint8_t M2MNsdlInterface::resource_callback(struct nsdl_s */*nsdl_handle*/,
                                            sn_coap_hdr_s *received_coap_header,
                                            sn_nsdl_addr_s *address,
                                            sn_nsdl_capab_e /*nsdl_capab*/)
{
    tr_debug("M2MNsdlInterface::resource_callback()");

    _observer.coap_data_processed();
    uint8_t result = 1;
    uint8_t *payload = NULL;
    bool free_payload = true;
    sn_coap_hdr_s *coap_response = NULL;
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 4.00
    char resource_name[MAX_VALUE_LENGTH];
    memset(resource_name, 0, MAX_VALUE_LENGTH);
    memcpy(resource_name, (char*)received_coap_header->uri_path_ptr,
           received_coap_header->uri_path_len);
    tr_debug("M2MNsdlInterface::resource_callback() - resource_name %s", resource_name);
    tr_debug("M2MNsdlInterface::resource_callback() - msg id:%" PRIu16, received_coap_header->msg_id);
    tr_debug("M2MNsdlInterface::resource_callback() - msg code:%d", received_coap_header->msg_code);
    tr_debug("M2MNsdlInterface::resource_callback() - msg type:%d", received_coap_header->msg_type);

    bool execute_value_updated = false;
    M2MBase* base = find_resource(resource_name, 0);
    if (base) {
        if (COAP_MSG_CODE_REQUEST_GET == received_coap_header->msg_code) {
            coap_response = base->handle_get_request(_nsdl_handle, received_coap_header,this);
        } else if (COAP_MSG_CODE_REQUEST_PUT == received_coap_header->msg_code) {
            coap_response = base->handle_put_request(_nsdl_handle, received_coap_header, this, execute_value_updated);
        } else if (COAP_MSG_CODE_REQUEST_POST == received_coap_header->msg_code) {
            if (base->base_type() == M2MBase::ResourceInstance) {
                msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
            } else {
                coap_response = base->handle_post_request(_nsdl_handle,
                                                          received_coap_header,
                                                          this,
                                                          execute_value_updated,
                                                          address);
            }
        } else if (COAP_MSG_CODE_REQUEST_DELETE == received_coap_header->msg_code) {
            // Delete the object instance
            M2MBase::BaseType type = base->base_type();
            if(M2MBase::ObjectInstance == type) {
                path_buffer buffer;
                base->uri_path(buffer);
                M2MBase* base_object = find_resource(buffer.c_str(), 0);

                if(base_object) {
                    M2MObject &object = ((M2MObjectInstance*)base_object)->get_parent_object();
                    char* slash = strrchr(resource_name, '/');
                    int slash_found = slash - resource_name + 1;
                    // Object instance validty checks done in upper level, no need for error handling
                    if (slash_found != -1) {
                        slash += 1;
                        if (object.remove_object_instance(strtoul(
                                slash,
                                NULL,
                                10))) {
                            msg_code = COAP_MSG_CODE_RESPONSE_DELETED;
                        }
                    }
                }
            } else {
                msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
            }
        }
    } else  {
        tr_debug("M2MNsdlInterface::resource_callback() - Resource NOT FOUND");
        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
    }

    if (!coap_response) {
        coap_response = sn_nsdl_build_response(_nsdl_handle,
                                               received_coap_header,
                                               msg_code);
    }

    // This copy will be passed to resource instance
    payload = (uint8_t*)memory_alloc(received_coap_header->payload_len + 1);
    if (payload) {
        memcpy(payload, received_coap_header->payload_ptr, received_coap_header->payload_len);
        payload[received_coap_header->payload_len] = '\0';
    }
    else {
        coap_response->msg_code = COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE;
    }

    if (coap_response &&
        coap_response->coap_status != COAP_STATUS_PARSER_BLOCKWISE_MSG_RECEIVING &&
        coap_response->msg_code != COAP_MSG_CODE_EMPTY) {
        tr_debug("M2MNsdlInterface::resource_callback() - response code: %d", coap_response->msg_code);
        (sn_nsdl_send_coap_message(_nsdl_handle, address, coap_response) == 0) ? result = 0 : result = 1;
        /*if(coap_response->payload_ptr) {
            memory_free(coap_response->payload_ptr);
            coap_response->payload_ptr = NULL;
        }*/
    }

    // If the external blockwise storing is enabled call value updated once all the blocks have been received
    if (execute_value_updated &&
            coap_response &&
            coap_response->coap_status != COAP_STATUS_PARSER_BLOCKWISE_MSG_RECEIVING) {
        if ((COAP_MSG_CODE_REQUEST_PUT == received_coap_header->msg_code) &&
            (base->base_type() == M2MBase::Resource ||
             base->base_type() == M2MBase::ResourceInstance)) {
            M2MResourceBase* res = (M2MResourceBase*)base;
            bool external_block_store = false;
#ifndef DISABLE_BLOCK_MESSAGE
            if (res->block_message() && res->block_message()->is_block_message()) {
                external_block_store = true;
            }
#endif
            if (!external_block_store) {
                // Ownership of payload moved to resource, skip the freeing.
                free_payload = false;
                res->set_value_raw(payload, received_coap_header->payload_len);
            }
        }

        if (coap_response->msg_code != COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE) {
            value_updated(base);
        }
    }

    if (free_payload) {
        free(payload);
    }

    sn_nsdl_release_allocated_coap_msg_mem(_nsdl_handle, coap_response);
    return result;
}

bool M2MNsdlInterface::process_received_data(uint8_t *data,
                                             uint16_t data_size,
                                             sn_nsdl_addr_s *address)
{
    tr_debug("M2MNsdlInterface::process_received_data( data size %d)", data_size);
    return (0 == sn_nsdl_process_coap(_nsdl_handle,
                                      data,
                                      data_size,
                                      address)) ? true : false;
}

void M2MNsdlInterface::stop_timers()
{
    tr_debug("M2MNsdlInterface::stop_timers()");
    _registration_timer.stop_timer();
    _nsdl_exceution_timer.stop_timer();
    _nsdl_exceution_timer_running = false;
    _bootstrap_id = 0;
    _unregister_ongoing = false;
}

void M2MNsdlInterface::timer_expired(M2MTimerObserver::Type type)
{
    if(M2MTimerObserver::NsdlExecution == type) {
        sn_nsdl_exec(_nsdl_handle, _counter_for_nsdl);
        _counter_for_nsdl++;
    } else if((M2MTimerObserver::Registration) == type && (_unregister_ongoing==false)) {
        tr_debug("M2MNsdlInterface::timer_expired - M2MTimerObserver::Registration - Send update registration");
        send_update_registration();
    }
}

void M2MNsdlInterface::observation_to_be_sent(M2MBase *object,
                                              uint16_t obs_number,
                                              const m2m::Vector<uint16_t> &changed_instance_ids,
                                              bool send_object)
{
    claim_mutex();
    if(object) {
        tr_debug("M2MNsdlInterface::observation_to_be_sent()");
        M2MBase::BaseType type = object->base_type();
        if(type == M2MBase::Object) {
            send_object_observation(static_cast<M2MObject*> (object),
                                    obs_number,
                                    changed_instance_ids,
                                    send_object);
        } else if(type == M2MBase::ObjectInstance) {
            send_object_instance_observation(static_cast<M2MObjectInstance*> (object), obs_number);
        } else if(type == M2MBase::Resource) {
            send_resource_observation(static_cast<M2MResource*> (object), obs_number);
        }
    }
    release_mutex();
}

void M2MNsdlInterface::send_post_response(M2MBase *base, const uint8_t *token, uint8_t token_length)
{
    claim_mutex();
    tr_debug("M2MNsdlInterface::send_post_response()");
    M2MResource *resource = NULL;
    if(base) {
        if(M2MBase::Resource == base->base_type()) {
            resource = static_cast<M2MResource *> (base);
        }
        if(resource) {
            uint8_t send_token[8];
            memcpy(send_token, token, token_length);
            sn_coap_hdr_s coap_response;

            memset(&coap_response,0,sizeof(sn_coap_hdr_s));

            coap_response.msg_type = COAP_MSG_TYPE_CONFIRMABLE;
            coap_response.msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
            coap_response.token_ptr = send_token;
            coap_response.token_len = token_length;

            coap_response.payload_ptr = resource->value();
            coap_response.payload_len = resource->value_length();

            sn_nsdl_send_coap_message(_nsdl_handle, _nsdl_handle->nsp_address_ptr->omalw_address_ptr, &coap_response);
       }
    }
    release_mutex();
}

void M2MNsdlInterface::resource_to_be_deleted(M2MBase *base)
{
    tr_debug("M2MNsdlInterface::resource_to_be_deleted()");
    claim_mutex();
    remove_nsdl_resource(base);

    // Since the M2MObject's are stored in _object_list, they need to be removed from there also.
    if ((base) && (base->base_type() == M2MBase::Object)) {
        remove_object(base);
    }
    release_mutex();
}

void M2MNsdlInterface::value_updated(M2MBase *base)
{
    tr_debug("M2MNsdlInterface::value_updated()");

    // get buffer for the object/obj-inst/resource/resource-inst id plus trailing zero.
    StringBuffer<M2MBase::MAX_INSTANCE_SIZE + 1> name;

    if(base) {
        switch(base->base_type()) {
            case M2MBase::Object:
                create_nsdl_object_structure(static_cast<M2MObject*> (base));
                name.append_int(base->name_id());
            break;
            case M2MBase::ObjectInstance:
                create_nsdl_object_instance_structure(static_cast<M2MObjectInstance*> (base));
                name.append_int(static_cast<M2MObjectInstance*> (base)->get_parent_object().name_id());

            break;
            case M2MBase::Resource: {
                M2MResource* resource = static_cast<M2MResource*> (base);
                create_nsdl_resource_structure(resource,
                                           resource->supports_multiple_instances());
                name.append_int(base->name_id());
            }
            break;
            case M2MBase::ResourceInstance: {
                M2MResourceInstance* instance = static_cast<M2MResourceInstance*> (base);
                create_nsdl_resource(instance);
                // XXX: is the parent resource id a correct information to be given!?
                name.append_int(static_cast<M2MResourceInstance*> (base)->get_parent_resource().name_id());
            }
            break;
        }
    }

    if (base && base->is_value_updated_function_set()) {

        base->execute_value_updated(name.c_str());
    }
    else {
        _observer.value_updated(base);
    }
}

void M2MNsdlInterface::remove_object(M2MBase *object)
{
    claim_mutex();
    tr_debug("M2MNsdlInterface::remove_object()");
    M2MObject *inst = _object_list;
    M2MObject *prev = NULL;

    while (inst) {
        M2MObject *next = inst->get_next();

        if (inst == object) {
            // Object found so lets remove it

            if (prev == NULL) {
                // this was the first item, we need to update things differently
                _object_list = next;
            } else {
                prev->set_next(next);
            }
            break;
        }

        prev = inst;
        inst = next;
    }
    release_mutex();
}

bool M2MNsdlInterface::create_nsdl_object_structure(const M2MObject *object)
{
    tr_debug("M2MNsdlInterface::create_nsdl_object_structure()");
    bool success = false;
    if(object) {
        const M2MObjectInstance *instance = object->instances();
        //tr_debug("M2MNsdlInterface::create_nsdl_object_structure - Object Instance count %d", instance_list.size());
        while (instance) {
            // Create NSDL structure for all resources inside
            success = create_nsdl_object_instance_structure(instance);
            instance = instance->get_next();
        }
    }
    if(object && object->operation() != M2MBase::NOT_ALLOWED) {
        success = create_nsdl_resource((M2MBase*)object);
    }
    return success;
}

bool M2MNsdlInterface::create_nsdl_object_instance_structure(const M2MObjectInstance *object_instance)
{
    tr_debug("M2MNsdlInterface::create_nsdl_object_instance_structure()");
    bool success = false;
    if( object_instance) {
        const M2MResource *res = object_instance->resources();
        //tr_debug("M2MNsdlInterface::create_nsdl_object_instance_structure - ResourceBase count %d", res_list.size());
        while (res) {

            // Create NSDL structure for all resources inside
            success = create_nsdl_resource_structure(res, res->supports_multiple_instances());

            res = res->get_next();
        }
        if(object_instance->operation() != M2MBase::NOT_ALLOWED) {
            success = create_nsdl_resource((M2MBase*)object_instance);
        }
    }
    return success;
}

bool M2MNsdlInterface::create_nsdl_resource_structure(const M2MResource *res,
                                                      bool multiple_instances)
{
    tr_debug("M2MNsdlInterface::create_nsdl_resource_structure()");
    bool success = false;
    if(res) {
        // if there are multiple instances supported
        if(multiple_instances) {
            const M2MResourceInstance* res_instance = res->resource_instances();
            //tr_debug("M2MNsdlInterface::create_nsdl_resource_structure - ResourceInstance count %d", res_list.size());
            if (res_instance) {
                while (res_instance) {
                    success = create_nsdl_resource((M2MBase*)res_instance);
                    if (!success) {
                        tr_error("M2MNsdlInterface::create_nsdl_resource_structure - instance creation failed");
                        return false;
                    }
                    res_instance = res_instance->get_next();
                }
                // Register the main Resource as well along with ResourceInstances
                success = create_nsdl_resource((M2MBase*)res);
            }
        } else {
            success = create_nsdl_resource((M2MBase*)res);
        }
    }
    return success;
}

bool M2MNsdlInterface::create_nsdl_resource(M2MBase *base)
{
    claim_mutex();
    tr_debug("M2MNsdlInterface::create_nsdl_resource");
    bool success = false;
    if(base) {
        // needed on deletion
        if (base->observation_handler() == NULL) {
            base->set_observation_handler(this);
        }
        success = true;
    }
    release_mutex();
    return success;
}

uint64_t M2MNsdlInterface::registration_time() const
{
    uint64_t value = 0;
    value = atol((const char*)_endpoint.lifetime_ptr);

    if(value >= OPTIMUM_LIFETIME) {
        value = value - REDUCE_LIFETIME;
    } else {
        value = REDUCTION_FACTOR * value;
    }
    tr_debug("M2MNsdlInterface::registration_time - value (in seconds) %ld", value);
    return value;
}

M2MBase* M2MNsdlInterface::find_resource(const char *object_name,
                                         const uint16_t msg_id) const
{
    tr_debug("M2MNsdlInterface::find_resource(object level) - name (%s)", object_name);
    M2MBase *object = NULL;
    if(_object_list) {
        const M2MObject *obj = _object_list;
        while (obj) {
            if (!msg_id) {
                path_buffer buffer;
                obj->uri_path(buffer);
                tr_debug("M2MNsdlInterface::find_resource(object level) - path (%s)",
                         buffer.c_str());
                if (strcmp(buffer.c_str(), object_name) == 0) {
                    object = (M2MObject*)obj;
                    tr_debug("M2MNsdlInterface::find_resource(%s) found", object_name);
                    break;
                }
            } else {
                uint16_t stored_msg_id = obj->get_notification_msgid();
                tr_debug("M2MNsdlInterface::find_resource(object level) - stored msgid (%d)", stored_msg_id);
                if (stored_msg_id == msg_id) {
                    object = (M2MObject*)obj;
                    tr_debug("M2MNsdlInterface::find_resource - msg id found");
                    break;
                }
            }
            object = find_resource(obj, object_name, msg_id);
            if(object != NULL) {
                break;
            }
            obj = obj->get_next();
        }
    }
    return object;
}

M2MBase* M2MNsdlInterface::find_resource(const M2MObject *object,
                                         const char *object_instance,
                                         const uint16_t msg_id) const
{
    tr_debug("M2MNsdlInterface::find_resource(object instance level) - name (%s)", object_instance);
    M2MBase *instance = NULL;
    if(object) {
        const M2MObjectInstance *inst = object->instances();
        while (inst) {
            if (!msg_id) {
                path_buffer buffer;
                inst->uri_path(buffer);
                tr_debug("M2MNsdlInterface::find_resource(object instance level) - path (%s)",
                         buffer.c_str());
                if(!strcmp(buffer.c_str(), object_instance)){
                    instance = (M2MObjectInstance*)inst;
                    break;
                }
            } else {
                uint16_t stored_msg_id = inst->get_notification_msgid();
                tr_debug("M2MNsdlInterface::find_resource(object instance level) - stored msgid (%d)", stored_msg_id);
                if (stored_msg_id == msg_id) {
                    instance = (M2MObjectInstance*)inst;
                    break;
                }
            }
            instance = find_resource(inst, object_instance, msg_id);
            if(instance != NULL){
                break;
            }
            inst = inst->get_next();
        }
    }
    return instance;
}

M2MBase* M2MNsdlInterface::find_resource(const M2MObjectInstance *object_instance,
                                         const char *resource_instance,
                                         const uint16_t msg_id) const
{
    tr_debug("M2MNsdlInterface::find_resource(resource level) - name (%s)", resource_instance);
    M2MBase *instance = NULL;
    if(object_instance) {

        const M2MResource *res = object_instance->resources();

        while (res) {
            path_buffer buffer;
            res->uri_path(buffer);
            if (!msg_id) {
                if (!strcmp(buffer.c_str(), resource_instance)) {
                    instance = (M2MResource*)res;
                    break;
                }
                else if (res->supports_multiple_instances()) {
                    instance = find_resource(res, buffer.c_str(),
                                             resource_instance);
                    if (instance != NULL) {
                        break;
                    }
                }
            } else {
                uint16_t stored_msg_id = res->get_notification_msgid();
                tr_debug("M2MNsdlInterface::find_resource(resource level) - stored msgid (%d)", stored_msg_id);
                if (stored_msg_id == msg_id) {
                    instance = (M2MResource*)res;
                    break;
                }
            }

            res = res->get_next();
        }
    }
    return instance;
}

M2MBase* M2MNsdlInterface::find_resource(const M2MResource *resource,
                                         const char *object_name,
                                         const char *resource_instance) const
{
    tr_debug("M2MNsdlInterface::find_resource(resource instance level)");
    M2MBase *res = NULL;
    if (resource) {
        if (resource->supports_multiple_instances()) {
            const M2MResourceInstance *instance = resource->resource_instances();
            for (; instance != NULL; instance = instance->get_next()) {
                path_buffer buffer;
                instance->uri_path(buffer);
                if (!strcmp(buffer.c_str(), resource_instance)){
                    // Ugh, the M2MResource is not technically modified by this method, yet
                    // it returns a non-const -pointer to its instances. So a cast (or API change)
                    // is needed somewhere.
                    res = (M2MResourceInstance *)instance;
                    break;
                }
            }
        }
    }
    return res;
}

bool M2MNsdlInterface::add_object_to_list(M2MObject *object)
{
    bool success = true;
    _object_list = object;
//    if (object) {
//        // verify, that the item is not already part of some list or this will create eternal loop
//        assert(object->get_next() == NULL);

//        // We keep the natural order of instances by adding to the end of list, which makes
//        // this a O(N) operation instead of O(1) as if we added to the head of list. This
//        // is not strictly speaking necessary but it is the old behavior.
//        M2MObject* last = _object_list;

//        if (last == NULL) {
//            _object_list = object;
//            success = true;
//        } else {
//            // loop until we find the last one
//            while ((last != NULL) && (last->get_next() != NULL)) {
//                last = last->get_next();
//                assert(last != object); // verify that we are not adding the same object there twice
//            }
//            last->set_next(object);
//            success = true;
//            object->set_next(NULL);
//        }
//    }
    return success;
}

M2MInterface::Error M2MNsdlInterface::interface_error(const sn_coap_hdr_s &coap_header)
{
    M2MInterface::Error error;
    switch(coap_header.msg_code) {
        case COAP_MSG_CODE_RESPONSE_BAD_REQUEST:
        case COAP_MSG_CODE_RESPONSE_BAD_OPTION:
        case COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_INCOMPLETE:
        case COAP_MSG_CODE_RESPONSE_PRECONDITION_FAILED:
        case COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE:
        case COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT:
            error = M2MInterface::InvalidParameters;
            break;
        case COAP_MSG_CODE_RESPONSE_UNAUTHORIZED:
        case COAP_MSG_CODE_RESPONSE_FORBIDDEN:
        case COAP_MSG_CODE_RESPONSE_NOT_ACCEPTABLE:
        case COAP_MSG_CODE_RESPONSE_NOT_FOUND:
        case COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED:
            error = M2MInterface::NotAllowed;
            break;
        case COAP_MSG_CODE_RESPONSE_CREATED:
        case COAP_MSG_CODE_RESPONSE_DELETED:
        case COAP_MSG_CODE_RESPONSE_VALID:
        case COAP_MSG_CODE_RESPONSE_CHANGED:
        case COAP_MSG_CODE_RESPONSE_CONTENT:
            error = M2MInterface::ErrorNone;
            break;
        default:
            error = M2MInterface::UnknownError;
            break;
    }
    if(coap_header.coap_status == COAP_STATUS_BUILDER_MESSAGE_SENDING_FAILED) {
        error = M2MInterface::NetworkError;
    }
    return error;
}

const char *M2MNsdlInterface::coap_error(const sn_coap_hdr_s &coap_header)
{

    if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_BAD_REQUEST) {
        return COAP_ERROR_REASON_1;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_BAD_OPTION) {
        return COAP_ERROR_REASON_2;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_INCOMPLETE) {
        return COAP_ERROR_REASON_3;
    }else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_PRECONDITION_FAILED) {
        return COAP_ERROR_REASON_4;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE) {
        return COAP_ERROR_REASON_5;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT) {
        return COAP_ERROR_REASON_6;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_UNAUTHORIZED) {
        return COAP_ERROR_REASON_7;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_FORBIDDEN) {
        return COAP_ERROR_REASON_8;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_NOT_ACCEPTABLE) {
        return COAP_ERROR_REASON_9;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_NOT_FOUND) {
        return COAP_ERROR_REASON_10;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED) {
        return COAP_ERROR_REASON_11;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_SERVICE_UNAVAILABLE) {
        return COAP_ERROR_REASON_13;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR) {
        return COAP_ERROR_REASON_14;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_BAD_GATEWAY) {
        return COAP_ERROR_REASON_15;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_GATEWAY_TIMEOUT) {
        return COAP_ERROR_REASON_16;
    } else if(coap_header.msg_code == COAP_MSG_CODE_RESPONSE_PROXYING_NOT_SUPPORTED) {
        return COAP_ERROR_REASON_17;
    } else if(coap_header.coap_status == COAP_STATUS_BUILDER_MESSAGE_SENDING_FAILED) {
        return COAP_ERROR_REASON_12;
    }
    return COAP_NO_ERROR;
}

void M2MNsdlInterface::send_object_observation(M2MObject *object,
                                               uint16_t obs_number,
                                               const m2m::Vector<uint16_t> &changed_instance_ids,
                                               bool send_object)
{
    tr_debug("M2MNsdlInterface::send_object_observation");
    if(object) {
        uint8_t *value = 0;
        uint32_t length = 0;
        uint8_t *token = NULL;
        uint8_t token_length = 0;

        // Send whole object structure
        if (send_object) {
            value = M2MTLVSerializer::serialize(object->instances(), length);
            if(!value) {
                tr_debug("send_object_observation - Not enough memory to send notification");
                return;
            }
        }
        // Send only changed object instances
        else {
            M2MObjectInstance * list = NULL;
            Vector<uint16_t>::const_iterator it;
            it = changed_instance_ids.begin();
            for (; it != changed_instance_ids.end(); it++) {
                M2MObjectInstance* obj_instance = object->object_instance(*it);
                if (obj_instance) {
                    if(!list) {
                        list = obj_instance;
                    } else {
                        list->set_next(obj_instance);
                    }
                }
            }
            if (list) {
                value = M2MTLVSerializer::serialize(list, length);
                list = NULL;
                if(!value) {
                    tr_debug("send_object_observation - Not enough memory to send notification 2");
                    return;
                }
            }
        }

        object->get_observation_token(token,token_length);

        uint16_t msgid = sn_nsdl_send_observation_notification(_nsdl_handle, token, token_length, value, length,
                                                               sn_coap_observe_e(obs_number), COAP_MSG_TYPE_CONFIRMABLE,
                                                               sn_coap_content_format_e(object->coap_content_type()));
        object->set_notification_msgid(msgid);

        memory_free(value);        
    }
}

void M2MNsdlInterface::send_object_instance_observation(M2MObjectInstance *object_instance,
                                                        uint16_t obs_number)
{
    tr_debug("M2MNsdlInterface::send_object_instance_observation");
    if(object_instance) {
        uint8_t *value = 0;
        uint32_t length = 0;
        uint8_t *token = NULL;
        uint8_t token_length = 0;

        value = M2MTLVSerializer::serialize_list(object_instance->resources(), length);
        if(!value) {
            tr_debug("Not enough memory to send notification");
            return;
        }
        object_instance->get_observation_token(token,token_length);

        uint16_t msgid = sn_nsdl_send_observation_notification(_nsdl_handle, token, token_length, value, length,
                                                               sn_coap_observe_e(obs_number), COAP_MSG_TYPE_CONFIRMABLE,
                                                               sn_coap_content_format_e(object_instance->coap_content_type()));
        object_instance->set_notification_msgid(msgid);

        memory_free(value);        
    }
}

void M2MNsdlInterface::send_resource_observation(M2MResource *resource,
                                                 uint16_t obs_number)
{
    tr_debug("M2MNsdlInterface::send_resource_observation");
    if(resource) {
        uint8_t *value = 0;
        uint32_t length = 0;
        uint8_t* token = NULL;
        uint8_t token_length = 0;
        bool dynamic_mem = false;

        resource->get_observation_token(token,token_length);
        uint16_t content_type = 0;
        if(M2MResourceBase::OPAQUE == resource->resource_instance_type()) {
            content_type = COAP_CONTENT_OMA_OPAQUE_TYPE;
        }
        if (resource->resource_instance_count() > 0) {
            dynamic_mem = true;
            content_type = resource->coap_content_type();
            value = M2MTLVSerializer::serialize(resource, length);
            if(!value) {
                tr_debug("Not enough memory to send notification");
                return;
            }
        } else {
            value = resource->value();
            length = resource->value_length();
        }

        uint16_t msgid = sn_nsdl_send_observation_notification(_nsdl_handle, token, token_length, value, length,
                                                               sn_coap_observe_e(obs_number), COAP_MSG_TYPE_CONFIRMABLE,
                                                               sn_coap_content_format_e(content_type));
        resource->set_notification_msgid(msgid);

        if(dynamic_mem) {
            memory_free(value);
        }
    }
}
nsdl_s * M2MNsdlInterface::get_nsdl_handle() const
{
    return _nsdl_handle;
}

void M2MNsdlInterface::handle_bootstrap_put_message(sn_coap_hdr_s *coap_header,
                                                sn_nsdl_addr_s *address) {
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MNsdlInterface::handle_bootstrap_message");
    uint8_t response_code = COAP_MSG_CODE_RESPONSE_CHANGED;
    sn_coap_hdr_s *coap_response = NULL;
    bool success = false;
    uint16_t content_type = 0;
    M2MNsdlInterface::ObjectType object_type = M2MNsdlInterface::SECURITY;

    tr_debug("M2MNsdlInterface::handle_bootstrap_message - object path  %.*s", coap_header->uri_path_len, (char*)coap_header->uri_path_ptr);
    // Security object
    if(strncmp((char*)coap_header->uri_path_ptr,"0", 1) == 0) {
        object_type = M2MNsdlInterface::SECURITY;
        success = true;
        // Not mandatory resource, that's why it must be created first
        _security->create_resource(M2MSecurity::ShortServerID, 1);
        change_operation_mode(_security, M2MBase::PUT_ALLOWED);
    }
    // Server object
    else if (strncmp((char*)coap_header->uri_path_ptr,"1", 1) == 0) {
        object_type = M2MNsdlInterface::SERVER;
        success = true;
    }
    // Device object
    else if (strncmp((char*)coap_header->uri_path_ptr,"3", 1) == 0) {
        M2MDevice* dev = M2MInterfaceFactory::create_device();
        // Not mandatory resource, that's why it must be created first
        uint32_t time = 0;
        dev->create_resource(M2MDevice::CurrentTime, time);
        object_type = M2MNsdlInterface::DEVICE;
        success = true;
    }

    if (success) {
        if(coap_header->content_format != COAP_CT_NONE) {
            content_type = coap_header->content_format;
        }

        if (content_type != COAP_CONTENT_OMA_TLV_TYPE &&
            content_type != COAP_CONTENT_OMA_TLV_TYPE_OLD) {
            tr_error("M2MNsdlInterface::handle_bootstrap_message - content_type %d", content_type);
            success = false;
        }
        if (success) {
            success = parse_bootstrap_message(coap_header, object_type);
            // Set operation back to default ones
             change_operation_mode(_security, M2MBase::NOT_ALLOWED);

        }
    }

    if (!success) {
        response_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    }

    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                           coap_header,
                                           response_code);
    if (coap_response) {
        sn_nsdl_send_coap_message(_nsdl_handle, address, coap_response);
        sn_nsdl_release_allocated_coap_msg_mem(_nsdl_handle, coap_response);
    }

    if (!success) {
        char buffer[MAX_ALLOWED_STRING_LENGTH];
        strcpy(buffer, ERROR_REASON_20);
        handle_bootstrap_error(buffer, true);
    }
#else
    (void) coap_header;
    (void) address;
#endif
}

bool M2MNsdlInterface::parse_bootstrap_message(sn_coap_hdr_s *coap_header,
                                               M2MNsdlInterface::ObjectType lwm2m_object_type)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MNsdlInterface::parse_bootstrap_put_message");
    bool ret = false;
    bool is_obj_instance = false;
    uint16_t instance_id = 0;
        tr_debug("M2MNsdlInterface::parse_bootstrap_put_message -- security OK");
    ret = is_obj_instance = M2MTLVDeserializer::is_object_instance(coap_header->payload_ptr);
    if (!is_obj_instance) {
        ret = M2MTLVDeserializer::is_resource(coap_header->payload_ptr);
    }
        tr_debug("M2MNsdlInterface::parse_bootstrap_put_message -- ret %d", ret);
    if (ret) {
        M2MTLVDeserializer::Error error = M2MTLVDeserializer::None;
        if (is_obj_instance) {
            M2MObject* dev_object = static_cast<M2MObject*> (M2MInterfaceFactory::create_device());
            switch (lwm2m_object_type) {
                case M2MNsdlInterface::SECURITY:
                    error = M2MTLVDeserializer::deserialise_object_instances(coap_header->payload_ptr,
                                                               coap_header->payload_len,
                                                               *_security,
                                                               M2MTLVDeserializer::Put);
                    break;
                case M2MNsdlInterface::SERVER: {
                    error = M2MTLVDeserializer::deserialise_object_instances(coap_header->payload_ptr,
                                                               coap_header->payload_len,
                                                               *_server,
                                                               M2MTLVDeserializer::Put);
                    }
                    break;
                case M2MNsdlInterface::DEVICE:
                   error = M2MTLVDeserializer::deserialise_object_instances(coap_header->payload_ptr,
                                                               coap_header->payload_len,
                                                               *dev_object,
                                                               M2MTLVDeserializer::Put);
                    break;
                default:
                    break;
            }
        }
        else {
            instance_id = M2MTLVDeserializer::instance_id(coap_header->payload_ptr);
            switch (lwm2m_object_type) {
                case M2MNsdlInterface::SECURITY:
                    error = M2MTLVDeserializer::deserialize_resources(coap_header->payload_ptr,
                                                               coap_header->payload_len,
                                                               *(_security->object_instance(instance_id)),
                                                               M2MTLVDeserializer::Put);

                    break;
                case M2MNsdlInterface::SERVER: {
                    error = M2MTLVDeserializer::deserialize_resources(coap_header->payload_ptr,
                                                               coap_header->payload_len,
                                                               *(_server->object_instance(instance_id)),
                                                               M2MTLVDeserializer::Post);
                    }
                    break;
                case M2MNsdlInterface::DEVICE:
                default:
                    break;
            }
        }

        if (error != M2MTLVDeserializer::None) {
            tr_error("M2MNsdlInterface::parse_bootstrap_put_message - error %d", error);
            ret = false;
        }
    }
    return ret;
#else
    (void) coap_header;
    return false;
#endif
}

void M2MNsdlInterface::handle_bootstrap_finished(sn_coap_hdr_s *coap_header,sn_nsdl_addr_s *address)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    char buffer[MAX_ALLOWED_STRING_LENGTH];

    tr_debug("M2MNsdlInterface::handle_bootstrap_finished - path: %.*s", coap_header->uri_path_len, (char*)coap_header->uri_path_ptr);
    sn_coap_hdr_s *coap_response = NULL;
    uint8_t msg_code = COAP_MSG_CODE_RESPONSE_CHANGED;

    // Accept only '/bs' path and check that needed data is in security object

    if (coap_header->uri_path_len != 2 ||
        strncmp((char*)coap_header->uri_path_ptr,BOOTSTRAP_URI, 2) != 0) {
        strcpy(buffer, ERROR_REASON_22);
        strncat(buffer,(char*)coap_header->uri_path_ptr,coap_header->uri_path_len +1);
        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    } else if(!validate_security_object()) {
        strcpy(buffer, ERROR_REASON_22);
        strncat(buffer,ERROR_REASON_26, sizeof(ERROR_REASON_26) +1);
        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    }
    else {
        // Add short server id to server object
        _server->set_resource_value(M2MServer::ShortServerID,
                                    _security->resource_value_int(M2MSecurity::ShortServerID));
    }

    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                           coap_header,
                                           msg_code);
    if(coap_response) {
        sn_nsdl_send_coap_message(_nsdl_handle, address, coap_response);
        sn_nsdl_release_allocated_coap_msg_mem(_nsdl_handle, coap_response);
    }

    if (COAP_MSG_CODE_RESPONSE_CHANGED == msg_code) {
        // Switch back to original ep name
        memcpy(_endpoint_name_ptr, (uint8_t*)_endpoint_name, strlen(_endpoint_name));
        _endpoint.endpoint_name_ptr = _endpoint_name_ptr;
        _endpoint.endpoint_name_len = strlen(_endpoint_name);
        // Inform observer that bootstrap is finished but it should wait until nsdl has sent data.
        // The final bootstrap_done callback is called in the observers data_sent callback.
        _observer.bootstrap_wait(_security);
    } else {
        handle_bootstrap_error(buffer, true);
    }
#else
    (void) coap_header;
    (void) address;
#endif
}

void M2MNsdlInterface::handle_bootstrap_delete(sn_coap_hdr_s *coap_header,sn_nsdl_addr_s *address)
{

#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    char buffer[MAX_ALLOWED_STRING_LENGTH];
    sn_coap_hdr_s *coap_response = NULL;
    uint8_t msg_code = COAP_MSG_CODE_RESPONSE_DELETED;

    tr_debug("M2MNsdlInterface::handle_bootstrap_delete - obj %.*s", coap_header->uri_path_len, (char*)coap_header->uri_path_ptr);
    if(!_identity_accepted) {
        strcpy(buffer, ERROR_REASON_21);
        strncat(buffer,ERROR_REASON_26, sizeof(ERROR_REASON_26) +1);
        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    }
    // Only following paths are accepted, 0, 0/0
    else if (coap_header->uri_path_len == 2 || coap_header->uri_path_len > 3) {
        strcpy(buffer, ERROR_REASON_21);
        strncat(buffer,(char*)coap_header->uri_path_ptr, coap_header->uri_path_len +1);

        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    }
    else if ((coap_header->uri_path_len == 1 && strncmp((char*)coap_header->uri_path_ptr,"0", 1) != 0) ||
             (coap_header->uri_path_len == 3 && strncmp((char*)coap_header->uri_path_ptr,"0/0", 3) != 0)) {
        strcpy(buffer, ERROR_REASON_21);
        strncat(buffer,(char*)coap_header->uri_path_ptr, coap_header->uri_path_len +1);

        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST;
    }

    coap_response = sn_nsdl_build_response(_nsdl_handle,
                                           coap_header,
                                           msg_code);

    if(coap_response) {
        sn_nsdl_send_coap_message(_nsdl_handle, address, coap_response);
        sn_nsdl_release_allocated_coap_msg_mem(_nsdl_handle, coap_response);
        _security->clear_resources();

    }
    if (!coap_response || COAP_MSG_CODE_RESPONSE_DELETED != msg_code) {
        handle_bootstrap_error(buffer, true);
    }
#else
    (void) coap_header;
    (void) address;
#endif
}

bool M2MNsdlInterface::validate_security_object()
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MNsdlInterface::validate_security_object");

    M2MResource * uri_resource = _security->get_resource(M2MSecurity::M2MServerUri);
    if(!uri_resource) {
        return false;
    }
    uint32_t sec_mode = _security->resource_value_int(M2MSecurity::SecurityMode);
    bool is_bs_server = _security->resource_value_int(M2MSecurity::BootstrapServer);
    uint32_t public_key_size = _security->get_resource(M2MSecurity::PublicKey)->value_length();
    uint32_t server_key_size = _security->get_resource(M2MSecurity::ServerPublicKey)->value_length();
    uint32_t pkey_size = _security->get_resource(M2MSecurity::Secretkey)->value_length();
    M2MDevice* dev = M2MInterfaceFactory::create_device();
    if (dev) {
        // Cast to uint32_t since all platforms does not support PRId64 macro
        tr_debug("M2MNsdlInterface::validate_security_object - Current time: %" PRIu32,
                 (uint32_t)dev->resource_value_int(M2MDevice::CurrentTime));
    }
    tr_debug("M2MNsdlInterface::validate_security_object - Server URI /0/0: %s", uri_resource->value());
    tr_debug("M2MNsdlInterface::validate_security_object - is bs server /0/1: %d", is_bs_server);
    tr_debug("M2MNsdlInterface::validate_security_object - Security Mode /0/2: %" PRIu32, sec_mode);
    tr_debug("M2MNsdlInterface::validate_security_object - Public key size /0/3: %" PRIu32, public_key_size);
    tr_debug("M2MNsdlInterface::validate_security_object - Server Public key size /0/4: %" PRIu32, server_key_size);
    tr_debug("M2MNsdlInterface::validate_security_object - Secret key size /0/5: %" PRIu32, pkey_size);
    // Only NoSec and Certificate modes are supported
    if (uri_resource->value_length() != 0 && !is_bs_server) {
        if (M2MSecurity::Certificate == sec_mode) {
            if (!public_key_size || !server_key_size || !pkey_size) {
                return false;
            } else {
                return true;
            }
        } else if (M2MSecurity::NoSecurity == sec_mode){
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }    
#else
    return false;
#endif
}

void M2MNsdlInterface::handle_bootstrap_error(const char *reason, bool wait)
{
    tr_debug("M2MNsdlInterface::handle_bootstrap_error(%s)",reason);
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    _identity_accepted = false;
    _security->clear_resources();

    if (wait) {
        _observer.bootstrap_error_wait(reason);
    } else {
        _observer.bootstrap_error(reason);
    }
#endif
}

const char *M2MNsdlInterface::endpoint_name() const
{
    return _endpoint_name;
}

const char *M2MNsdlInterface::internal_endpoint_name() const
{
    const char *iep = NULL;
    if (_nsdl_handle->ep_information_ptr->location_ptr) {
        // Get last part of the location path.
        // In mbed Cloud environment full path is /rd/accountid/internal_endpoint
        iep = strrchr((const char*)_nsdl_handle->ep_information_ptr->location_ptr, '/');
        iep += 1;
    }
    return iep;
}

void M2MNsdlInterface::change_operation_mode(M2MObject *object, M2MBase::Operation operation)
{
    M2MResource *res = object->object_instance()->resources();

    while (res) {

        res->set_operation(operation);

        res = res->get_next();
    }
}

void M2MNsdlInterface::set_server_address(const char* server_address)
{
    strncpy(_server_address, server_address, MAX_VALUE_LENGTH);
}

M2MTimer &M2MNsdlInterface::get_nsdl_execution_timer()
{
    return _nsdl_exceution_timer;
}

bool M2MNsdlInterface::get_unregister_ongoing() const
{
    return _unregister_ongoing;
}

bool M2MNsdlInterface::parse_and_send_uri_query_parameters()
{
    bool msg_sent = false;
    // Use high value since the address will be in the
    // format "coaps://xxx-xxx-xxx.xxx.xxx:5684?aid=xxyyzz"
    // MAX_VALUE_LENGTH = 256 is safe enough value
    char address_copy[MAX_VALUE_LENGTH];
    strcpy(address_copy, _server_address);
    char* query = query_string(address_copy);
    if (query != NULL) {
        int param_count = query_param_count(query);
        if (param_count) {
            char* uri_query_params[MAX_QUERY_COUNT];
            if (uri_query_parameters(query, uri_query_params)) {
                sn_nsdl_clear_coap_resending_queue(_nsdl_handle);
                msg_sent = sn_nsdl_register_endpoint(_nsdl_handle,&_endpoint,uri_query_params, param_count) != 0;
            }
        }
    }
    return msg_sent;
}

void M2MNsdlInterface::claim_mutex()
{
    _connection_handler.claim_mutex();
}

void M2MNsdlInterface::release_mutex()
{
    _connection_handler.release_mutex();
}

void M2MNsdlInterface::start_nsdl_execution_timer()
{
    tr_debug("M2MNsdlInterface::start_nsdl_execution_timer");
    _nsdl_exceution_timer_running = true;
    _nsdl_exceution_timer.stop_timer();
    _nsdl_exceution_timer.start_timer(ONE_SECOND_TIMER * 1000,
                                       M2MTimerObserver::NsdlExecution,
                                      false);
}

sn_nsdl_dynamic_resource_parameters_s* M2MNsdlInterface::get_first(struct iterator_state_s **it_state,
                                                                   char **path)
{
    sn_nsdl_dynamic_resource_parameters_s* param = NULL;
    iterator_state_s *state = NULL;
    state = (iterator_state_s*)M2MCoreMemory::memory_alloc(sizeof(iterator_state_s));
    memset(state, 0, sizeof(iterator_state_s));
    if(_object_list) {
        param = &(_object_list->get_nsdl_resource());
        resource_path(_object_list, path);
        state->object = _object_list;
        state->objectinstance = state->object->instances();
        state->resource = state->objectinstance->resources();
        state->resourceinstance = state->resource->resource_instances();
    }
    *it_state = state;
    return param;
}

sn_nsdl_dynamic_resource_parameters_s* M2MNsdlInterface::get_next(struct iterator_state_s **it_state,
                                                                  char **path)
{
    iterator_state_s *state = (iterator_state_s*)*it_state;
    sn_nsdl_dynamic_resource_parameters_s* param = NULL;
    while(state->object) {
        if(state->resourceinstance) {
            param = &(state->resourceinstance->get_nsdl_resource());
            resource_path(state->resourceinstance, path);
            state->resourceinstance = state->resourceinstance->get_next();
            break;
        }
        if(state->resource) {
            param = &(state->resource->get_nsdl_resource());
            resource_path(state->resource, path);
            state->resource = state->resource->get_next();
            break;
        }
        if(state->objectinstance) {
            param = &(state->objectinstance->get_nsdl_resource());
            resource_path(state->objectinstance, path);
            state->objectinstance = state->objectinstance->get_next();
            break;
        }
        state->object = state->object->get_next();
        if(state->object) {
            state->objectinstance = state->object->instances();
            param = &(state->object->get_nsdl_resource());
            resource_path(state->object, path);
        }
        if(state->objectinstance) {
            state->resource = state->objectinstance->resources();
        }
        if(state->resource) {
            state->resourceinstance = state->resource->resource_instances();
        }
        break;
    }
    if(!state->object) {
        M2MCoreMemory::memory_free(state);
        state = NULL;
    }
    *it_state = state;
    return param;
}

sn_nsdl_dynamic_resource_parameters_s* M2MNsdlInterface::find_resource(const char *path)
{
    sn_nsdl_dynamic_resource_parameters_s* param = NULL;
    M2MBase *base = NULL;
    base = find_resource(path,0);
    if(base) {
        param = &(base->get_nsdl_resource());
    }
    return param;
}

void M2MNsdlInterface::resource_path(M2MBase *base, char **path)
{
    path_buffer buffer;
    if(base) {
        if(base->base_type() == M2MBase::Object) {
            M2MObject *obj = (M2MObject*)base;
            obj->uri_path(buffer);
            *path = (char*)buffer.c_str();
        } else if(base->base_type() == M2MBase::ObjectInstance) {
            M2MObjectInstance *obj_inst = (M2MObjectInstance*)base;
            obj_inst->uri_path(buffer);
            *path = (char*)buffer.c_str();
        } else if(base->base_type() == M2MBase::Resource) {
            M2MResource *res = (M2MResource*)base;
            res->uri_path(buffer);
            *path = (char*)buffer.c_str();
        } else if(base->base_type() == M2MBase::ResourceInstance) {
            M2MResourceInstance *res_inst = (M2MResourceInstance*)base;
            res_inst->uri_path(buffer);
            *path = (char*)buffer.c_str();
        }
    }
}
