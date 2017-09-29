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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "include/m2minterfaceimpl.h"
#include "include/eventdata.h"
#include "mbed-client/m2minterfaceobserver.h"
#include "mbed-client/m2mconnectionhandler.h"
#include "mbed-client/m2mconnectionsecurity.h"
#include "include/m2mnsdlinterface.h"
#include "mbed-client/m2msecurity.h"
#include "mbed-client/m2mtimer.h"
#include "mbed-trace/mbed_trace.h"
#include "randLIB.h"

#include <stdlib.h>

#define TRACE_GROUP "mClt"

#define RESOLVE_SEC_MODE(mode)  ((mode == M2MInterface::TCP || mode == M2MInterface::TCP_QUEUE) ? M2MConnectionSecurity::TLS : M2MConnectionSecurity::DTLS)

M2MInterfaceImpl::M2MInterfaceImpl(M2MInterfaceObserver& observer,
                                   const uint16_t listen_port,
                                   M2MInterface::BindingMode mode,
                                   M2MInterface::NetworkStack stack)
: _event_data(NULL),
  _registration_flow_timer(NULL),
  _server_port(0),
  _listen_port(listen_port),
  _register_server(NULL),
  _queue_sleep_timer(NULL),
  _retry_timer(*this),
  _callback_handler(NULL),
  _max_states( STATE_MAX_STATES ),
  _event_ignored(false),
  _event_generated(false),
  _reconnecting(false),
  _retry_timer_expired(false),
  _bootstrapped(true), // True as default to get it working with connector only configuration
  _current_state(0),
  _binding_mode(mode),
  _reconnection_state(M2MInterfaceImpl::None),
  _observer(observer),
  _security_connection(RESOLVE_SEC_MODE(mode)),
  _connection_handler(*this, &_security_connection, mode, stack),
  _nsdl_interface(*this, _connection_handler),
  _security(NULL)
{
    tr_debug("M2MInterfaceImpl::M2MInterfaceImpl() -IN");
    //TODO:Increase the range from 1 to 100 seconds
    randLIB_seed_random();
    // Range is from 2 to 10
    _initial_reconnection_time = randLIB_get_random_in_range(2, 10);

    tr_debug("M2MInterfaceImpl::M2MInterfaceImpl() initial random time %d\n", _initial_reconnection_time);
    _reconnection_time = _initial_reconnection_time;

#ifndef DISABLE_ERROR_DESCRIPTION
    memset(_error_description, 0, sizeof(_error_description));
#endif


    //Here we must use TCP still
    _connection_handler.bind_connection(_listen_port);

    tr_debug("M2MInterfaceImpl::M2MInterfaceImpl() -OUT");
}

bool M2MInterfaceImpl::setup(const char *endpoint_name,
                             const char *endpoint_type,
                             const int32_t life_time,
                             const char *domain,
                             const char *context_address)
{
    tr_debug("M2MInterfaceImpl::setup()");

    // We need this timer only in case of TCP
    if (_binding_mode == M2MInterface::TCP ||
        _binding_mode == M2MInterface::TCP_QUEUE) {
        _registration_flow_timer = new M2MTimer(*this);
        if(!_registration_flow_timer) {
            return false;
        }
    }
    if(is_queue_mode()) {
        _queue_sleep_timer = new M2MTimer(*this);
        if(!_queue_sleep_timer) {
            delete _registration_flow_timer;
            return false;
        }

    }
    if(!_nsdl_interface.create_endpoint(endpoint_name,
                                           endpoint_type,
                                           life_time,
                                           domain,
                                           (uint8_t)_binding_mode & 0x07, // nsdl binding mode is only 3 least significant bits
                                           context_address)) {
        delete _queue_sleep_timer;
        delete _registration_flow_timer;
        return false;
    }

    return true;
}



M2MInterfaceImpl::~M2MInterfaceImpl()
{
    tr_debug("M2MInterfaceImpl::~M2MInterfaceImpl() - IN");
    _connection_handler.stop_listening();
    delete _registration_flow_timer;
    delete _queue_sleep_timer;
    tr_debug("M2MInterfaceImpl::~M2MInterfaceImpl() - OUT");
}

void M2MInterfaceImpl::bootstrap(M2MSecurity *security)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::bootstrap(M2MSecurity *security) - IN");
    _retry_timer.stop_timer();
    _security = NULL;
    if(!security) {
        set_error_description(ERROR_REASON_1);
        _observer.error(M2MInterface::InvalidParameters);
        return;
    }
    // Transition to a new state based upon
    // the current state of the state machine
    M2MSecurityData data;
    data._object = security;
    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY (STATE_BOOTSTRAP)              // state_idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state__bootstrap_address_resolved
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_resource_created
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_error_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrapped
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register_address_resolved
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_registered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_update_registration
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregister
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregistered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_sending_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_sent
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_received
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_processing_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_processed
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_waiting
    END_TRANSITION_MAP(&data)
    if(_event_ignored) {
        _event_ignored = false;
        set_error_description(ERROR_REASON_2);
        _observer.error(M2MInterface::NotAllowed);
    }
    tr_debug("M2MInterfaceImpl::bootstrap(M2MSecurity *security) - OUT");
#else
    set_error_description(ERROR_REASON_3);
    _observer.error(M2MInterface::NotAllowed);
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::cancel_bootstrap()
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
//TODO: Do we need this ?
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::add_object(M2MRegisterData *data, M2MObject *object)
{
    tr_debug("M2MInterfaceImpl::add_object()");

    if (object) {
        // verify, that the item is not already part of some list or this will create eternal loop
        assert(object->get_next() == NULL);

        // We keep the natural order of instances by adding to the end of list, which makes
        // this a O(N) operation instead of O(1) as if we added to the head of list. This
        // is not strictly speaking necessary but it is the old behaviour.
        M2MObject* last = data->_object_list;

        if (last == NULL) {
            data->_object_list = object;
        } else {
            // loop until we find the last one
            while ((last != NULL) && (last->get_next() != NULL)) {
                last = last->get_next();
                assert(last != object); // verify that we are not adding the same object there twice
            }
            last->set_next(object);
            object->set_next(NULL);
        }
    }
}

void M2MInterfaceImpl::register_object(M2MSecurity *security, const M2MObjectList &object_list)
{
    tr_debug("M2MInterfaceImpl::register_object - IN");
    if(!security) {
        set_error_description(ERROR_REASON_4);
        _observer.error(M2MInterface::InvalidParameters);
        return;
    }
    // Transition to a new state based upon
    // the current state of the state machine
    //TODO: manage register object in a list.
    _register_server = security;
    M2MRegisterData data;
    data._object = security;
    data._object_list = NULL;
    if(!object_list.empty()) {
       M2MObjectList::const_iterator it;
       it = object_list.begin();
       for ( ; it != object_list.end(); it++ ) {
          add_object(&data, (*it));
       }
    }

    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY (STATE_REGISTER)               // state_idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state__bootstrap_address_resolved
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_resource_created
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_error_wait
        TRANSITION_MAP_ENTRY (STATE_REGISTER)               // state_bootstrapped
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register_address_resolved
        TRANSITION_MAP_ENTRY (STATE_REGISTER)               // state_registered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_update_registration
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregister
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregistered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_sending_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_sent
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_received
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_processing_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_processed
        TRANSITION_MAP_ENTRY (STATE_REGISTER)               // state_waiting
    END_TRANSITION_MAP(&data)
    if(_event_ignored) {
        _event_ignored = false;
        set_error_description(ERROR_REASON_5);
        _observer.error(M2MInterface::NotAllowed);
    }
    tr_debug("M2MInterfaceImpl::register_object - OUT");
}

void M2MInterfaceImpl::update_registration(M2MSecurity *security_object, const uint32_t lifetime)
{
    tr_debug("M2MInterfaceImpl::update_registration(M2MSecurity *security_object, const uint32_t lifetime)");
    M2MUpdateRegisterData data;
    data._object = security_object;
    data._lifetime = lifetime;
    data._object_list = NULL;
    start_register_update(&data);
}

void M2MInterfaceImpl::update_registration(M2MSecurity *security_object,
                                           const M2MObjectList &object_list,
                                           const uint32_t lifetime)
{
    tr_debug("M2MInterfaceImpl::update_registration(M2MSecurity *security_object, const M2MObjectList &object_list, const uint32_t lifetime)");
    M2MUpdateRegisterData data;
    data._object = security_object;
    data._lifetime = lifetime;
    data._object_list = NULL;
    if(!object_list.empty()) {
       M2MObjectList::const_iterator it;
       it = object_list.begin();
       data._object_list = (*it);
       M2MObject * obj = data._object_list;
       for ( ; it != object_list.end(); it++ ) {
          obj->set_next(*it);
       }
    }
    start_register_update(&data);
}

void M2MInterfaceImpl::unregister_object(M2MSecurity* /*security*/)
{
    tr_debug("M2MInterfaceImpl::unregister_object - IN");
    tr_debug("M2MInterfaceImpl::unregister_object - current state %d", _current_state);
    // Transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                                // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state__bootstrap_address_resolved
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_resource_created
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_error_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrapped
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register_address_resolved
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_registered
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_update_registration
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregister
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregistered
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_sending_coap_data
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_coap_data_sent
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_coap_data_received
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_processing_coap_data
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_coap_data_processed
        TRANSITION_MAP_ENTRY (STATE_UNREGISTER)             // state_waiting
    END_TRANSITION_MAP(NULL)
    if(_event_ignored) {
        _event_ignored = false;
        set_error_description(ERROR_REASON_6);
        _observer.error(M2MInterface::NotAllowed);
    }
    tr_debug("M2MInterfaceImpl::unregister_object - OUT");
}

void M2MInterfaceImpl::set_queue_sleep_handler(callback_handler handler)
{
    tr_debug("M2MInterfaceImpl::set_queue_sleep_handler()");
    _callback_handler = handler;
}

void M2MInterfaceImpl::set_random_number_callback(random_number_cb callback)
{
    _security_connection.set_random_number_callback(callback);
}

void M2MInterfaceImpl::set_entropy_callback(entropy_cb callback)
{
    _security_connection.set_entropy_callback(callback);
}

void M2MInterfaceImpl::set_platform_network_handler(void *handler)
{
    tr_debug("M2MInterfaceImpl::set_platform_network_handler()");
    _connection_handler.set_platform_network_handler(handler);
}

void M2MInterfaceImpl::coap_message_ready(uint8_t *data_ptr,
                                          uint16_t data_len,
                                          sn_nsdl_addr_s *address_ptr)
{
    tr_debug("M2MInterfaceImpl::coap_message_ready");
    if (_current_state != STATE_IDLE) {
        internal_event(STATE_SENDING_COAP_DATA);
        if(!_connection_handler.send_data(data_ptr,data_len,address_ptr)) {
            internal_event( STATE_IDLE);
            tr_error("M2MInterfaceImpl::coap_message_ready() - M2MInterface::NetworkError");
            if (!_reconnecting) {
                _queue_mode_timer_ongoing = false;
                socket_error(M2MConnectionHandler::SOCKET_SEND_ERROR, true);
            } else {
                socket_error(M2MConnectionHandler::SOCKET_ABORT);
            }
        }
    }
}

void M2MInterfaceImpl::client_registered(M2MServer *server_object)
{
    tr_debug("M2MInterfaceImpl::client_registered");
    internal_event(STATE_REGISTERED);
    //Inform client is registered.
    //TODO: manage register object in a list.
    _observer.object_registered(_register_server,*server_object);
}

void M2MInterfaceImpl::registration_updated(const M2MServer &server_object)
{
    tr_debug("M2MInterfaceImpl::registration_updated");
    internal_event(STATE_REGISTERED);
    _observer.registration_updated(_register_server,server_object);
}

void M2MInterfaceImpl::registration_error(uint8_t error_code, bool retry)
{
    tr_debug("M2MInterfaceImpl::registration_error code [%d]", error_code);
    // Try to register again
    if (retry) {
        _queue_mode_timer_ongoing = false;
        if (error_code == M2MInterface::UnregistrationFailed) {
            _reconnection_state = M2MInterfaceImpl::Unregistration;
        }
        socket_error(M2MConnectionHandler::SOCKET_SEND_ERROR);
    } else {
        _security = NULL;
        internal_event(STATE_IDLE);
        if (error_code == M2MInterface::UnregistrationFailed) {
            set_error_description(ERROR_REASON_24);
        } else {
            set_error_description(ERROR_REASON_8);
        }
        _observer.error((M2MInterface::Error)error_code);
    }
}

void M2MInterfaceImpl::client_unregistered()
{
    tr_debug("M2MInterfaceImpl::client_unregistered()");
    internal_event(STATE_UNREGISTERED);
    //TODO: manage register object in a list.
    _observer.object_unregistered(_register_server);
}

void M2MInterfaceImpl::bootstrap_done(M2MSecurity *security_object)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::bootstrap_done");
    _reconnection_time = _initial_reconnection_time;
    _reconnecting = false;
    _reconnection_state = M2MInterfaceImpl::None;
    _bootstrapped = true;
    if (_registration_flow_timer) {
        _registration_flow_timer->stop_timer();
    }
    internal_event(STATE_BOOTSTRAPPED);
    _observer.bootstrap_done(security_object);
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::bootstrap_wait(M2MSecurity *security_object)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::bootstrap_wait");
    _security = security_object;
    internal_event(STATE_BOOTSTRAP_WAIT);
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::bootstrap_error_wait(const char *reason)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::bootstrap_error_wait");
    set_error_description(reason);
    internal_event(STATE_BOOTSTRAP_ERROR_WAIT);
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::bootstrap_error(const char *reason)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::bootstrap_error(%s)", reason);
    _bootstrapped = false;
    if (_registration_flow_timer) {
        _registration_flow_timer->stop_timer();
    }

    _reconnection_state = M2MInterfaceImpl::None;

    set_error_description(reason);

    _observer.error(M2MInterface::BootstrapFailed);

    internal_event(STATE_IDLE);
    _reconnecting = true;
    _connection_handler.stop_listening();

    _retry_timer_expired = false;
    _retry_timer.start_timer(_reconnection_time * 1000,
                              M2MTimerObserver::RetryTimer);
    tr_debug("M2MInterfaceImpl::bootstrap_error - reconnecting in %" PRIu32 "(s)", _reconnection_time);
    _reconnection_time = _reconnection_time * RECONNECT_INCREMENT_FACTOR;
    if(_reconnection_time >= MAX_RECONNECT_TIMEOUT) {
        _reconnection_time = MAX_RECONNECT_TIMEOUT;
    }
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::coap_data_processed()
{
    tr_debug("M2MInterfaceImpl::coap_data_processed()");
    internal_event(STATE_COAP_DATA_PROCESSED);
}

void M2MInterfaceImpl::value_updated(M2MBase *base)
{
    tr_debug("M2MInterfaceImpl::value_updated");
    if(base) {
        M2MBase::BaseType type = base->base_type();
        _observer.value_updated(base, type);
    }
}

void M2MInterfaceImpl::data_available(uint8_t* data,
                                      uint16_t data_size,
                                      const M2MConnectionObserver::SocketAddress &address)
{
    tr_debug("M2MInterfaceImpl::data_available");
    ReceivedData event;
    event._data = data;
    event._size = data_size;
    event._address = &address;
    internal_event(STATE_COAP_DATA_RECEIVED, &event);
}

void M2MInterfaceImpl::socket_error(uint8_t error_code, bool retry)
{
    tr_debug("M2MInterfaceImpl::socket_error: (%d), retry (%d), reconnecting (%d), reconnection_state (%d)",
             error_code, retry, _reconnecting, (int)_reconnection_state);

    // Ignore errors while client is sleeping
    if(_binding_mode == M2MInterface::UDP_QUEUE ||
       _binding_mode == M2MInterface::TCP_QUEUE  ||
       _binding_mode == M2MInterface::SMS_QUEUE  ||
       _binding_mode == M2MInterface::UDP_SMS_QUEUE) {
        if(_callback_handler && _queue_mode_timer_ongoing) {
            tr_debug("M2MInterfaceImpl::socket_error - Queue Mode - don't try to reconnect while in QueueMode");
            return;
        }
    }

    if (_registration_flow_timer) {
        _registration_flow_timer->stop_timer();
    }

    if (_queue_sleep_timer) {
        _queue_sleep_timer->stop_timer();
    }

    if (_reconnection_state != M2MInterfaceImpl::Unregistration) {
        _reconnection_state = M2MInterfaceImpl::WithUpdate;
    }

    const char *error_code_des;
    M2MInterface::Error error = M2MInterface::ErrorNone;
    switch (error_code) {
    case M2MConnectionHandler::SSL_CONNECTION_ERROR:
        error = M2MInterface::SecureConnectionFailed;
        error_code_des = ERROR_SECURE_CONNECTION;
        break;
    case M2MConnectionHandler::DNS_RESOLVING_ERROR:
        error = M2MInterface::DnsResolvingFailed;
        error_code_des = ERROR_DNS;
        break;
    case M2MConnectionHandler::SOCKET_READ_ERROR:
        error = M2MInterface::NetworkError;
        error_code_des = ERROR_NETWORK;
        break;
    case M2MConnectionHandler::SOCKET_SEND_ERROR:
        error = M2MInterface::NetworkError;
        error_code_des = ERROR_NETWORK;
        break;
    case M2MConnectionHandler::SSL_HANDSHAKE_ERROR:
        error = M2MInterface::SecureConnectionFailed;
        error_code_des = ERROR_SECURE_CONNECTION;
        break;
    case M2MConnectionHandler::SOCKET_ABORT:
        error = M2MInterface::NetworkError;
        error_code_des = ERROR_NETWORK;
        break;
    default:
        error_code_des = ERROR_NO;
        break;
    }

    internal_event(STATE_IDLE);
    // Try to do reconnecting
    if (retry) {
        _reconnecting = true;
        _connection_handler.stop_listening();
        _retry_timer_expired = false;
        _retry_timer.start_timer(_reconnection_time * 1000,
                                  M2MTimerObserver::RetryTimer);
        tr_debug("M2MInterfaceImpl::socket_error - reconnecting in %" PRIu32 "(s)", _reconnection_time);
        _reconnection_time = _reconnection_time * RECONNECT_INCREMENT_FACTOR;
        if(_reconnection_time >= MAX_RECONNECT_TIMEOUT) {
            _reconnection_time = MAX_RECONNECT_TIMEOUT;
        }
#ifndef DISABLE_ERROR_DESCRIPTION
        strcpy(_error_description, ERROR_REASON_9);
        strncat(_error_description, error_code_des,strlen(error_code_des) +1);
#endif
    }
    // Inform application
    if (!retry && M2MInterface::ErrorNone != error) {
        tr_debug("M2MInterfaceImpl::socket_error - send error to application");
        _connection_handler.stop_listening();
        _retry_timer.stop_timer();
        _security = NULL;
        _reconnecting = false;
        _reconnection_time = _initial_reconnection_time;
        _reconnection_state = M2MInterfaceImpl::None;
#ifndef DISABLE_ERROR_DESCRIPTION
        strcpy(_error_description, ERROR_REASON_10);
        strncat(_error_description, error_code_des, strlen(error_code_des) +1);
#endif
    }
    if(M2MInterface::ErrorNone != error) {
        _observer.error(error);
    }
}

void M2MInterfaceImpl::address_ready(const M2MConnectionObserver::SocketAddress &address,
                                     M2MConnectionObserver::ServerType server_type,
                                     const uint16_t server_port)
{
    tr_debug("M2MInterfaceImpl::address_ready");
    ResolvedAddressData data;
    data._address = &address;
    data._port = server_port;
    if( M2MConnectionObserver::Bootstrap == server_type) {
        tr_debug("M2MInterfaceImpl::address_ready() Server Type Bootstrap");
        internal_event(STATE_BOOTSTRAP_ADDRESS_RESOLVED, &data);
    } else {
        tr_debug("M2MInterfaceImpl::address_ready() Server Type LWM2M");
        internal_event(STATE_REGISTER_ADDRESS_RESOLVED, &data);
    }
}

void M2MInterfaceImpl::data_sent()
{
    tr_debug("M2MInterfaceImpl::data_sent()");
    if(is_queue_mode() && _callback_handler) {
        
        if(_callback_handler && (_nsdl_interface.get_unregister_ongoing()==false)) {
            _queue_sleep_timer->stop_timer();
            // Multiply by two to get enough time for coap retransmissions.
            _queue_sleep_timer->start_timer(MBED_CLIENT_RECONNECTION_COUNT*MBED_CLIENT_RECONNECTION_INTERVAL*1000*2,
                                    M2MTimerObserver::QueueSleep);
        }
    }

    if (_current_state == STATE_BOOTSTRAP_WAIT) {
        // For bootstrap we need to call bootstrap_done callback ONLY after we have
        // sent the last ACK and ended in STATE_BOOTSTRAP_WAIT
        M2MSecurity *sec = _security;
        _security = NULL;
        bootstrap_done(sec);
    }
    else if (_current_state == STATE_BOOTSTRAP_ERROR_WAIT) {
        // bootstrap_error to be called only after we have sent the last ACK.
        // Otherwise client will goto reconnection mode before ACK has sent.
        bootstrap_error(_error_description);
    }
    else {
        internal_event(STATE_COAP_DATA_SENT);
    }
}

void M2MInterfaceImpl::timer_expired(M2MTimerObserver::Type type)
{
    tr_debug("M2MInterfaceImpl::timer_expired()");
    if(M2MTimerObserver::QueueSleep == type) {
        M2MTimer &timer = _nsdl_interface.get_nsdl_execution_timer();
        timer.stop_timer();
        _queue_mode_timer_ongoing = true;
        if(_callback_handler) {
            _callback_handler();
        }
    }
    else if (M2MTimerObserver::RetryTimer == type) {
        _retry_timer_expired = true;
        if (_bootstrapped) {
            internal_event(STATE_REGISTER);
        } else {
            internal_event(STATE_BOOTSTRAP);
        }
    }
    else if (M2MTimerObserver::BootstrapFlowTimer == type) {
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
        _bootstrapped = false;
        if (_registration_flow_timer) {
            _registration_flow_timer->stop_timer();
        }
        bootstrap_error(ERROR_REASON_23);
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    }
    else if (M2MTimerObserver::RegistrationFlowTimer == type) {
        registration_error(M2MInterface::Timeout, true);
    }
}

// state machine sits here.
void M2MInterfaceImpl::state_idle(EventData* /*data*/)
{
    tr_debug("M2MInterfaceImpl::state_idle");
    _nsdl_interface.stop_timers();
}

void M2MInterfaceImpl::state_bootstrap(EventData *data)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::state_bootstrap");
    // Start with bootstrapping preparation
    _bootstrapped = false;
    M2MSecurityData *event = static_cast<M2MSecurityData *> (data);
    if(!_security) {
        M2MInterface::Error error = M2MInterface::InvalidParameters;
        if (event) {
            _security = event->_object;
            if(_security) {
                if(M2MSecurity::Bootstrap == _security->server_type()) {
                    tr_debug("M2MInterfaceImpl::state_bootstrap - server_type : M2MSecurity::Bootstrap");
                    M2MResource * uri_resource = _security->get_resource(M2MSecurity::M2MServerUri);
                    if(uri_resource) {
                        char server_uri[MAX_VALUE_LENGTH];
                        memset(server_uri, 0, MAX_VALUE_LENGTH);
                        memcpy(server_uri, (char*)uri_resource->value(), uri_resource->value_length());
                        _nsdl_interface.set_server_address(server_uri);
                        tr_debug("M2MInterfaceImpl::state_bootstrap - server_address %s", server_uri);
                        const char *server_address = NULL;
                        if(strstr(server_uri, COAP)) {
                            server_address = server_uri + strlen(COAP);
                        }
                        else if(strstr(server_uri, COAPS)) {
                            bool valid_coap = false;
                            _security->resource_value_int(M2MSecurity::SecurityMode) != M2MSecurity::NoSecurity ? valid_coap = true: valid_coap = false;
                            if(valid_coap) {
                                server_address  = server_uri + strlen(COAPS);
                            }
                        }
                        if(server_address) {

                            process_address(server_address, _server_port);

                            tr_debug("M2MInterfaceImpl::state_bootstrap - IP address %s , Port %d", _server_ip_address, _server_port);
                            // If bind and resolving server address succeed then proceed else
                            // return error to the application and go to Idle state.
                            if(strlen(_server_ip_address) != 0) {
                                error = M2MInterface::ErrorNone;
                                if (_registration_flow_timer) {
                                    _registration_flow_timer->stop_timer();
                                    _registration_flow_timer->start_timer(MBED_CLIENT_RECONNECTION_COUNT * MBED_CLIENT_RECONNECTION_INTERVAL * 8 * 1000,
                                                                  M2MTimerObserver::BootstrapFlowTimer);
                                }
                                _connection_handler.resolve_server_address(_server_ip_address,
                                                                            _server_port,
                                                                            M2MConnectionObserver::Bootstrap,
                                                                            _security);
                            }
                        }
                    }
                }
            }
        }
        if (error != M2MInterface::ErrorNone) {
            tr_error("M2MInterfaceImpl::state_bootstrap - set error as M2MInterface::InvalidParameters");
            internal_event(STATE_IDLE);
            set_error_description(ERROR_REASON_11);
            _observer.error(error);
        }
    } else {
        _listen_port = 0;
        _connection_handler.stop_listening();
        _connection_handler.bind_connection(_listen_port);
        if (_registration_flow_timer) {
            _registration_flow_timer->stop_timer();
            _registration_flow_timer->start_timer(MBED_CLIENT_RECONNECTION_COUNT * MBED_CLIENT_RECONNECTION_INTERVAL * 8 * 1000,
                                          M2MTimerObserver::BootstrapFlowTimer);
        }
        _connection_handler.resolve_server_address(_server_ip_address,
                                                    _server_port,
                                                    M2MConnectionObserver::Bootstrap,
                                                    _security);
    }
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::state_bootstrap_address_resolved( EventData *data)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::state_bootstrap_address_resolved");
    if (data) {
        ResolvedAddressData *event = static_cast<ResolvedAddressData *> (data);
        sn_nsdl_addr_s address;

        M2MInterface::NetworkStack stack = event->_address->_stack;

        if(M2MInterface::LwIP_IPv4 == stack) {
            tr_debug("M2MInterfaceImpl::state_bootstrap_address_resolved : IPv4 address");
            address.type = SN_NSDL_ADDRESS_TYPE_IPV4;
        } else if((M2MInterface::LwIP_IPv6 == stack) ||
                  (M2MInterface::Nanostack_IPv6 == stack)) {
            tr_debug("M2MInterfaceImpl::state_bootstrap_address_resolved : IPv6 address");
            address.type = SN_NSDL_ADDRESS_TYPE_IPV6;
        }
        address.port = event->_port;
        address.addr_ptr = (uint8_t*)event->_address->_address;
        address.addr_len = event->_address->_length;
        _connection_handler.start_listening_for_data();

        if(_nsdl_interface.create_bootstrap_resource(&address)) {
           tr_debug("M2MInterfaceImpl::state_bootstrap_address_resolved : create_bootstrap_resource - success");
           internal_event(STATE_BOOTSTRAP_RESOURCE_CREATED);
        } else{
            // If resource creation fails then inform error to application
            tr_error("M2MInterfaceImpl::state_bootstrap_address_resolved : M2MInterface::InvalidParameters");
            internal_event(STATE_IDLE);
            set_error_description(ERROR_REASON_12);
            _observer.error(M2MInterface::InvalidParameters);
        }
    }
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::state_bootstrap_resource_created( EventData */*data*/)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::state_bootstrap_resource_created");
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::state_bootstrapped( EventData */*data*/)
{
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    tr_debug("M2MInterfaceImpl::state_bootstrapped");
#endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
}

void M2MInterfaceImpl::state_register(EventData *data)
{
    tr_debug("M2MInterfaceImpl::state_register");
    M2MRegisterData *event = static_cast<M2MRegisterData *> (data);
    if (!_security) {
        M2MInterface::Error error = M2MInterface::InvalidParameters;
        // Start with registration preparation
        if(event) {
            _security = event->_object;
            if(_security) {
                if(M2MSecurity::M2MServer == _security->server_type()) {
                    tr_debug("M2MInterfaceImpl::state_register - server_type : M2MSecurity::M2MServer");
                    if(_nsdl_interface.create_nsdl_list_structure(event->_object_list)) {
                        tr_debug("M2MInterfaceImpl::state_register - create_nsdl_list_structure - success");
                        // If the nsdl resource structure is created successfully
                        M2MResource * uri_resource = _security->get_resource(M2MSecurity::M2MServerUri);
                        if(uri_resource) {
                            char server_uri[MAX_VALUE_LENGTH];
                            memset(server_uri, 0, MAX_VALUE_LENGTH);
                            memcpy(server_uri, (char*)uri_resource->value(), uri_resource->value_length());
                            _nsdl_interface.set_server_address(server_uri);
                            tr_debug("M2MInterfaceImpl::state_bootstrap - server_address %s", server_uri);
                            const char *server_address = NULL;
                            if(strstr(server_uri, COAP)) {
                                server_address = server_uri + strlen(COAP);
                            }
                            else if(strstr(server_uri, COAPS)) {
                                bool valid_coap = false;
                                _security->resource_value_int(M2MSecurity::SecurityMode) != M2MSecurity::NoSecurity ? valid_coap = true: valid_coap = false;
                                if(valid_coap) {
                                    server_address  = server_uri + strlen(COAPS);
                                }
                            }
                            if(server_address) {

                                process_address(server_address, _server_port);

                                tr_debug("M2MInterfaceImpl::state_register - IP address %s , Port %d", _server_ip_address, _server_port);
                                if(strlen(_server_ip_address) != 0) {
                                    // Connection related errors are coming through callback
                                    if (_registration_flow_timer) {
                                        _registration_flow_timer->stop_timer();
                                        _registration_flow_timer->start_timer(MBED_CLIENT_RECONNECTION_COUNT * MBED_CLIENT_RECONNECTION_INTERVAL * 8 * 1000,
                                                                      M2MTimerObserver::RegistrationFlowTimer);
                                    }

                                    error = M2MInterface::ErrorNone;
                                    _connection_handler.resolve_server_address(_server_ip_address,_server_port,
                                                                                M2MConnectionObserver::LWM2MServer,
                                                                                _security);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (error != M2MInterface::ErrorNone) {
            tr_error("M2MInterfaceImpl::state_register - set error as M2MInterface::InvalidParameters");
            internal_event(STATE_IDLE);
            set_error_description(ERROR_REASON_13);
            _observer.error(error);
        }
    } else {
        _listen_port = 0;
        _connection_handler.stop_listening();
        if (event) {
            _nsdl_interface.create_nsdl_list_structure(event->_object_list);
        }
        _connection_handler.bind_connection(_listen_port);
        if (_registration_flow_timer) {
            _registration_flow_timer->stop_timer();
            _registration_flow_timer->start_timer(MBED_CLIENT_RECONNECTION_COUNT * MBED_CLIENT_RECONNECTION_INTERVAL * 8 * 1000,
                                          M2MTimerObserver::RegistrationFlowTimer);
        }
        _connection_handler.resolve_server_address(_server_ip_address,_server_port,
                                                    M2MConnectionObserver::LWM2MServer,
                                                    _security);
    }
}

void M2MInterfaceImpl::process_address(const char *server_address, uint16_t& port)
{
    memset(_server_ip_address, 0 ,MAX_ALLOWED_IP_STRING_LENGTH);
    const char* colon = strrchr(server_address, ':');
    int colonFound = colon - server_address + 1;
    if(colonFound > 0) {
        memcpy(_server_ip_address, server_address, strlen(server_address) - strlen(colon));
        colon += 1;
        port = atoi(colon);
        const char* end = strrchr(_server_ip_address, ']');
        colonFound = end - _server_ip_address + 1;
        if(strncmp(_server_ip_address, "[", 1) == 0) {
            if(colonFound < 0) {
                memset(_server_ip_address, 0, MAX_ALLOWED_IP_STRING_LENGTH);
            } else {
                strncpy(_server_ip_address, _server_ip_address+1, colonFound-1);
                _server_ip_address[colonFound] = '\0';
            }
        } else if(colonFound > 0) {
            memset(_server_ip_address, 0, MAX_ALLOWED_IP_STRING_LENGTH);
        }
    }
}

bool M2MInterfaceImpl::is_queue_mode() const
{
    if(_binding_mode == M2MInterface::UDP_QUEUE ||
       _binding_mode == M2MInterface::TCP_QUEUE  ||
       _binding_mode == M2MInterface::SMS_QUEUE  ||
       _binding_mode == M2MInterface::UDP_SMS_QUEUE) {
        return true;
    }
    return false;
}

void M2MInterfaceImpl::state_register_address_resolved( EventData *data)
{
    tr_debug("M2MInterfaceImpl::state_register_address_resolved");
    if(data) {
        ResolvedAddressData *event = static_cast<ResolvedAddressData *> (data);

        sn_nsdl_addr_type_e address_type = SN_NSDL_ADDRESS_TYPE_IPV6;

        M2MInterface::NetworkStack stack = event->_address->_stack;

        if(M2MInterface::LwIP_IPv4 == stack) {
            tr_debug("M2MInterfaceImpl::state_register_address_resolved : IPv4 address");
            address_type = SN_NSDL_ADDRESS_TYPE_IPV4;
        } else if((M2MInterface::LwIP_IPv6 == stack) ||
                  (M2MInterface::Nanostack_IPv6 == stack)) {
            tr_debug("M2MInterfaceImpl::state_register_address_resolved : IPv6 address");
            address_type = SN_NSDL_ADDRESS_TYPE_IPV6;
        }
        _connection_handler.start_listening_for_data();
        _nsdl_interface.set_server_address((uint8_t*)event->_address->_address,event->_address->_length,
                                           event->_port, address_type);
        switch (_reconnection_state) {
            case M2MInterfaceImpl::None:
            case M2MInterfaceImpl::FullRegistration:
                if(!_nsdl_interface.send_register_message()) {
                    // If resource creation fails then inform error to application
                    tr_error("M2MInterfaceImpl::state_register_address_resolved : M2MInterface::InvalidParameters");
                    internal_event(STATE_IDLE);
                    set_error_description(ERROR_REASON_14);
                    _observer.error(M2MInterface::InvalidParameters);
                }
                break;
            case M2MInterfaceImpl::WithUpdate:
                // Start registration update in case it is reconnection logic because of network issue.
                internal_event(STATE_UPDATE_REGISTRATION);
                break;
            case M2MInterfaceImpl::Unregistration:
                internal_event(STATE_UNREGISTER);
                break;

        }
    }
}

void M2MInterfaceImpl::state_registered( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_registered");
    if (_registration_flow_timer) {
        _registration_flow_timer->stop_timer();
    }
    _reconnection_time = _initial_reconnection_time;
    _reconnecting = false;
    _reconnection_state = M2MInterfaceImpl::None;
}

void M2MInterfaceImpl::state_update_registration(EventData *data)
{
    tr_debug("M2MInterfaceImpl::state_update_registration");
    uint32_t lifetime = 0;
    bool clear_queue = false;
    // Set to false to allow reconnection to work.
    _queue_mode_timer_ongoing = false;

    if(data) {
        M2MUpdateRegisterData *event = static_cast<M2MUpdateRegisterData *> (data);
        // Create new resources if any
        if (event->_object_list) {
            _nsdl_interface.create_nsdl_list_structure(event->_object_list);
        }
        lifetime = event->_lifetime;
    } else {
        clear_queue = true;
    }

    bool success = _nsdl_interface.send_update_registration(lifetime, clear_queue);
    if(!success) {
        if(_reconnection_state == M2MInterfaceImpl::WithUpdate) {
            _reconnection_state = M2MInterfaceImpl::FullRegistration;
            if(!_nsdl_interface.send_register_message()) {
                tr_error("M2MInterfaceImpl::state_update_registration : M2MInterface::InvalidParameters");
                internal_event(STATE_IDLE);
                set_error_description(ERROR_REASON_1);
                _observer.error(M2MInterface::InvalidParameters);
            }
        }
    }
}

void M2MInterfaceImpl::state_unregister(EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_unregister");
    internal_event(STATE_SENDING_COAP_DATA);
    if(!_nsdl_interface.send_unregister_message()) {
        tr_error("M2MInterfaceImpl::state_unregister : M2MInterface::NotRegistered");
        internal_event(STATE_IDLE);
        set_error_description(ERROR_REASON_16);
        _observer.error(M2MInterface::NotRegistered);
    }
}

void M2MInterfaceImpl::state_unregistered( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_unregistered");
    _reconnection_time = _initial_reconnection_time;
    _connection_handler.stop_listening();
    internal_event(STATE_IDLE);
}

void M2MInterfaceImpl::state_sending_coap_data( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_sending_coap_data");
    _nsdl_interface.start_nsdl_execution_timer();
    internal_event(STATE_WAITING);
}

void M2MInterfaceImpl::state_coap_data_sent( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_coap_data_sent");
    internal_event(STATE_WAITING);
}

void M2MInterfaceImpl::state_coap_data_received( EventData *data)
{
    tr_debug("M2MInterfaceImpl::state_coap_data_received");
    if(data) {
        ReceivedData *event = static_cast<ReceivedData*> (data);
        sn_nsdl_addr_s address;

        M2MInterface::NetworkStack stack = event->_address->_stack;

        if(M2MInterface::LwIP_IPv4 == stack) {
            tr_debug("M2MInterfaceImpl::state_coap_data_received : IPv4 address");
            address.type = SN_NSDL_ADDRESS_TYPE_IPV4;
            address.addr_len = 4;
        } else if((M2MInterface::LwIP_IPv6 == stack) ||
                  (M2MInterface::Nanostack_IPv6 == stack)) {
            tr_debug("M2MInterfaceImpl::state_coap_data_received : IPv6 address");
            address.type = SN_NSDL_ADDRESS_TYPE_IPV6;
            address.addr_len = 16;
        }
        address.port = event->_address->_port;
        address.addr_ptr = (uint8_t*)event->_address->_address;
        address.addr_len = event->_address->_length;

        // Process received data
        internal_event(STATE_PROCESSING_COAP_DATA);
        if(!_nsdl_interface.process_received_data(event->_data,
                                                  event->_size,
                                                  &address)) {
           tr_error("M2MInterfaceImpl::state_coap_data_received : M2MInterface::ResponseParseFailed");
           set_error_description(ERROR_REASON_17);
            _observer.error(M2MInterface::ResponseParseFailed);
        }
    }
}

void M2MInterfaceImpl::state_processing_coap_data( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_processing_coap_data");
    internal_event(STATE_WAITING);
}

void M2MInterfaceImpl::state_coap_data_processed( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_coap_data_processed");
    internal_event(STATE_WAITING);
}

void M2MInterfaceImpl::state_waiting( EventData */*data*/)
{
    tr_debug("M2MInterfaceImpl::state_waiting");
}

// generates an external event. called once per external event
// to start the state machine executing
void M2MInterfaceImpl::external_event(uint8_t new_state,
                                     EventData* p_data)
{
    tr_debug("M2MInterfaceImpl::external_event : new state %d", new_state);
    // if we are supposed to ignore this event
    if (new_state == EVENT_IGNORED) {
        tr_debug("M2MInterfaceImpl::external_event : new state is EVENT_IGNORED");
        _event_ignored = true;
    }
    else {
        tr_debug("M2MInterfaceImpl::external_event : handle new state");
        // generate the event and execute the state engine
        internal_event(new_state, p_data);
    }
}

// generates an internal event. called from within a state
// function to transition to a new state
void M2MInterfaceImpl::internal_event(uint8_t new_state,
                                      EventData* p_data)
{
    tr_debug("M2MInterfaceImpl::internal_event : new state %d", new_state);
    _event_data = p_data;
    _event_generated = true;
    _current_state = new_state;
    state_engine();
}

// the state engine executes the state machine states
void M2MInterfaceImpl::state_engine (void)
{
    tr_debug("M2MInterfaceImpl::state_engine");
    EventData* p_data_temp = NULL;

    // while events are being generated keep executing states
    while (_event_generated) {
        p_data_temp = _event_data;  // copy of event data pointer
        _event_data = NULL;       // event data used up, reset ptr
        _event_generated = false;  // event used up, reset flag

        assert(_current_state < _max_states);

        state_function( _current_state, p_data_temp );
    }
}

void M2MInterfaceImpl::state_function( uint8_t current_state, EventData* data )
{
    switch( current_state ) {
        case STATE_IDLE:
            M2MInterfaceImpl::state_idle(data);
            break;
        case STATE_BOOTSTRAP:
        #ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            M2MInterfaceImpl::state_bootstrap(data);
        #endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            break;
        case STATE_BOOTSTRAP_ADDRESS_RESOLVED:
        #ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            M2MInterfaceImpl::state_bootstrap_address_resolved(data);
        #endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            break;
        case STATE_BOOTSTRAP_RESOURCE_CREATED:
        #ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            M2MInterfaceImpl::state_bootstrap_resource_created(data);
        #endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            break;
        case STATE_BOOTSTRAP_WAIT:
        case STATE_BOOTSTRAP_ERROR_WAIT:
        #ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            // Do nothing, we're just waiting for data_sent callback
        #endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            break;
        case STATE_BOOTSTRAPPED:
        #ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            M2MInterfaceImpl::state_bootstrapped(data);
        #endif //MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
            break;
        case STATE_REGISTER:
            M2MInterfaceImpl::state_register(data);
            break;
        case STATE_REGISTER_ADDRESS_RESOLVED:
            M2MInterfaceImpl::state_register_address_resolved(data);
            break;
        case STATE_REGISTERED:
            M2MInterfaceImpl::state_registered(data);
            break;
        case STATE_UPDATE_REGISTRATION:
            M2MInterfaceImpl::state_update_registration(data);
            break;
        case STATE_UNREGISTER:
            M2MInterfaceImpl::state_unregister(data);
            break;
        case STATE_UNREGISTERED:
            M2MInterfaceImpl::state_unregistered(data);
            break;
        case STATE_SENDING_COAP_DATA:
            M2MInterfaceImpl::state_sending_coap_data(data);
            break;
        case STATE_COAP_DATA_SENT:
            M2MInterfaceImpl::state_coap_data_sent(data);
            break;
        case STATE_COAP_DATA_RECEIVED:
            M2MInterfaceImpl::state_coap_data_received(data);
            break;
        case STATE_PROCESSING_COAP_DATA:
            M2MInterfaceImpl::state_processing_coap_data(data);
            break;
        case STATE_COAP_DATA_PROCESSED:
            M2MInterfaceImpl::state_coap_data_processed(data);
            break;
        case STATE_WAITING:
            M2MInterfaceImpl::state_waiting(data);
            break;
    }
}

void M2MInterfaceImpl::start_register_update(M2MUpdateRegisterData *data) {
    tr_debug("M2MInterfaceImpl::start_register_update - IN");
    if(!data || (data->_lifetime != 0 && (data->_lifetime < MINIMUM_REGISTRATION_TIME))) {
        set_error_description(ERROR_REASON_18);
        _observer.error(M2MInterface::InvalidParameters);
    }
    if (_reconnecting) {
        //If client is in reconnection mode, ignore this call, state machine will reconnect on its own.
        return;
    }
    _reconnection_state = M2MInterfaceImpl::WithUpdate;
    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY (STATE_REGISTER)                // state_idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state__bootstrap_address_resolved
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_resource_created
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrap_error_wait
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_bootstrapped
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_register_address_resolved
        TRANSITION_MAP_ENTRY (STATE_UPDATE_REGISTRATION)    // state_registered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_update_registration
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregister
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_unregistered
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_sending_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_sent
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_received
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_processing_coap_data
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)                // state_coap_data_processed
        TRANSITION_MAP_ENTRY (STATE_UPDATE_REGISTRATION)    // state_waiting
    END_TRANSITION_MAP(data)
    if(_event_ignored) {
        _event_ignored = false;
        if (!_reconnecting) {
            set_error_description(ERROR_REASON_19);
            _observer.error(M2MInterface::NotAllowed);
        }
    }
}

const String M2MInterfaceImpl::internal_endpoint_name() const
{
    return String(internal_endpoint_name_str());
}

void M2MInterfaceImpl::update_endpoint(const char *name)
{
    _nsdl_interface.update_endpoint(name);
}

void M2MInterfaceImpl::update_domain(const char *domain)
{
    _nsdl_interface.update_domain(domain);
}

const char* M2MInterfaceImpl::internal_endpoint_name_str() const
{
    return _nsdl_interface.internal_endpoint_name();
}


const char *M2MInterfaceImpl::error_description() const
{
#ifndef DISABLE_ERROR_DESCRIPTION
    return _error_description;
#else
    return "";
#endif
}

void M2MInterfaceImpl::set_error_description(const char *description)
{
#ifndef DISABLE_ERROR_DESCRIPTION
    if (strncmp(_error_description, description, sizeof(_error_description)) != 0) {
        strncpy(_error_description, description, sizeof(_error_description));
    }
#endif
}

