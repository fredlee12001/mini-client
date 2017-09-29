//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#include <string>
#include <assert.h>
#include <stdio.h>
#include "include/ConnectorClient.h"
#include "include/CloudClientStorage.h"
#include "include/CertificateParser.h"
#include "MbedCloudClient.h"
#include "mbed-client/m2mdevice.h"
#include "mbed-trace/mbed_trace.h"
#include "factory_configurator_client.h"

#define TRACE_GROUP "regc"

#define INTERNAL_ENDPOINT_PARAM     "&iep="
#define DEFAULT_ENDPOINT "endpoint"
#define INTERFACE_ERROR             "Client interface is not created. Restart"
#define CREDENTIAL_ERROR            "Failed to read credentials from storage"
#define DEVICE_NOT_PROVISIONED      "Device not provisioned"
#define ERROR_NO_MEMORY             "Not enough memory to stroe LWM2M credentials"

// XXX: nothing here yet
class EventData {

};

ConnectorClient::ConnectorClient(ConnectorClientCallback* callback)
: _callback(callback),
  _current_state(State_Bootstrap_Start),
  _event_generated(false), _state_engine_running(false),
  _interface(NULL), _register_security(NULL),_bootstrap_security(NULL),
  _endpoint_info(M2MSecurity::Certificate), _client_objs(NULL),
  _rebootstrap_timer(*this),
  _bootstrap_done(false)
{
    assert(_callback != NULL);

    _interface = M2MInterfaceFactory::create_interface(*this,
                                                      DEFAULT_ENDPOINT,                     // endpoint name string
                                                      MBED_CLOUD_CLIENT_ENDPOINT_TYPE,      // endpoint type string
                                                      MBED_CLOUD_CLIENT_LIFETIME,           // lifetime
                                                      MBED_CLOUD_CLIENT_LISTEN_PORT,        // listen port
                                                      _endpoint_info.account_id,            // domain string
                                                      transport_mode(),                     // binding mode
                                                      M2MInterface::LwIP_IPv4,              // network stack
                                                      NULL);

    initialize_storage();
}


ConnectorClient::~ConnectorClient()
{
    M2MDevice::delete_instance();
    if(!_bootstrap_done) {
        delete _register_security;
    }
    delete _bootstrap_security;
    delete _interface;
}

void ConnectorClient::start_bootstrap()
{
    tr_debug("ConnectorClient::start_bootstrap()");
    assert(_callback != NULL);
    StartupSubStateRegistration next_state = State_Bootstrap_Failure;
    // Stop rebootstrap timer if it was running
    _rebootstrap_timer.stop_timer();
    if (create_bootstrap_object()) {
        _interface->update_endpoint(_endpoint_info.endpoint_name);
        _interface->update_domain(_endpoint_info.account_id);
        next_state = State_Bootstrap_Start;
        internal_event(next_state);

    } else {
        tr_debug("ConnectorClient::start_bootstrap() - object fail");
    }
    state_engine();
}

void ConnectorClient::start_registration(M2MObjectList* client_objs)
{
    tr_debug("ConnectorClient::start_registration()");
    assert(_callback != NULL);
    _client_objs = client_objs;

    // XXX: actually this call should be external_event() to match the pattern used in other m2m classes
    create_register_object();
    if(_register_security) {
        if(use_bootstrap()) {
            // Bootstrap registration always uses iep
            _interface->update_endpoint(_endpoint_info.internal_endpoint_name);
        } else {
            // Registration without bootstrap always uses external id
            _interface->update_endpoint(_endpoint_info.endpoint_name);
        }
        _interface->update_domain(_endpoint_info.account_id);
        internal_event(State_Registration_Start);
    } else {
        tr_error("ConnectorClient::state_init(): failed to create objs");
        _callback->connector_error(M2MInterface::InvalidParameters, INTERFACE_ERROR);
    }
    state_engine();
}

M2MInterface * ConnectorClient::m2m_interface()
{
    return _interface;
}

void ConnectorClient::update_registration()
{
    if(_interface && _register_security) {
        _interface->update_registration(_register_security, MBED_CLOUD_CLIENT_LIFETIME);
    }
}

// generates an internal event. called from within a state
// function to transition to a new state
void ConnectorClient::internal_event(StartupSubStateRegistration new_state)
{
    tr_debug("ConnectorClient::internal_event: state: %d -> %d", _current_state, new_state);
    _event_generated = true;
    _current_state = new_state;

    // Avoid recursive chain which eats too much of stack
    if (!_state_engine_running) {
        state_engine();
    }
}

// the state engine executes the state machine states
void ConnectorClient::state_engine(void)
{
    tr_debug("ConnectorClient::state_engine");

    // this simple flagging gets rid of recursive calls to this method
    _state_engine_running = true;

    // while events are being generated keep executing states
    while (_event_generated) {
        _event_generated = false;  // event used up, reset flag

        state_function(_current_state);
    }

    _state_engine_running = false;
}

void ConnectorClient::state_function(StartupSubStateRegistration current_state)
{
    switch (current_state) {
        case State_Bootstrap_Start:
            state_bootstrap_start();
            break;
        case State_Bootstrap_Started:
            state_bootstrap_started();
            break;
        case State_Bootstrap_Success:
            state_bootstrap_success();
            break;
        case State_Bootstrap_Failure:
            state_bootstrap_failure();
            break;
        case State_Registration_Start:
            state_registration_start();
            break;
        case State_Registration_Started:
            state_registration_started();
            break;
        case State_Registration_Success:
            state_registration_success();
            break;
        case State_Registration_Failure:
            state_registration_failure();
            break;
        case State_Unregistered:
            state_unregistered();
            break;
    }
}

/*
*  Creates register server object with mbed device server address and other parameters
*  required for client to connect to mbed device server.
*/
void ConnectorClient::create_register_object()
{
    tr_debug("ConnectorClient::create_register_object()");
    if(!_register_security) {
        _register_security = M2MInterfaceFactory::create_security(M2MSecurity::M2MServer);
        // Add ResourceID's and values to the security ObjectID/ObjectInstance
        if (_register_security) {
            _register_security->set_bootstrap_flow(use_bootstrap());
            _register_security->set_resource_value(M2MSecurity::SecurityMode, _endpoint_info.mode);

            // Allocate scratch buffer, this will be used to copy parameters from storage to security object
            const int max_size = 1024;
            uint8_t buffer[max_size];
            size_t real_size = 0;
            bool success = true;

            // Connector CA
            if (success) {
                success = false;
                if (get_config_certificate(g_fcc_lwm2m_server_ca_certificate_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                    success = true;
                    _register_security->set_resource_value(M2MSecurity::ServerPublicKey,
                                                           buffer,
                                                           (uint32_t)real_size);
                }
                else
                    tr_debug("KEY_CONNECTOR_CA cert failed.\n");
            }

            // Connector device public key
            if (success) {
                success = false;
                if (get_config_certificate(g_fcc_lwm2m_device_certificate_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                    success = true;
                    _register_security->set_resource_value(M2MSecurity::PublicKey, buffer, (uint32_t)real_size);
                }
                else
                    tr_debug("KEY_CONNECTOR__DEVICE_CERT failed.\n");
            }

            // Connector device private key
            if (success) {
                success = false;
                if (get_config_private_key(g_fcc_lwm2m_device_private_key_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                    success = true;
                    _register_security->set_resource_value(M2MSecurity::Secretkey, buffer, (uint32_t)real_size);
                }
                else
                    tr_debug("KEY_CONNECTOR_DEVICE_PRIV failed.\n");
            }

            // Connector URL
            if (success) {
                success = false;
                if (get_config_parameter(g_fcc_lwm2m_server_uri_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                    success = true;
                    _register_security->set_resource_value(M2MSecurity::M2MServerUri, buffer, (uint32_t)real_size);
                }
                else
                    tr_debug("KEY_CONNECTOR_URL failed.\n");
            }

            // Endpoint
            if (success) {
                success = false;
                if (get_config_parameter(g_fcc_endpoint_parameter_name, (uint8_t*)_endpoint_info.endpoint_name, MAX_ALLOWED_STRING_LENGTH, &real_size) == CCS_STATUS_SUCCESS) {
                    success = true;                    
                }
                else
                    tr_debug("KEY_ENDPOINT_NAME failed.\n");
            }

            // Try to get internal endpoint name
            if (success) {
                if (get_config_parameter(KEY_INTERNAL_ENDPOINT, (uint8_t*)_endpoint_info.internal_endpoint_name, MAX_ALLOWED_STRING_LENGTH, &real_size) == CCS_STATUS_SUCCESS) {
                    tr_debug("Using internal endpoint name instead: %s", _endpoint_info.internal_endpoint_name);
                }
                else
                    tr_debug("KEY_INTERNAL_ENDPOINT failed.\n");
            }

            // Account ID, not mandatory
            if (success) {
                if (get_config_parameter(KEY_ACCOUNT_ID, (uint8_t*)_endpoint_info.account_id, MAX_ALLOWED_STRING_LENGTH, &real_size) == CCS_STATUS_SUCCESS) {
                }
                else
                    tr_debug("KEY_ACCOUNT_ID failed.\n");
            }

            if (!success) {
                tr_error("ConnectorClient::create_register_object - Failed to read credentials");
                _callback->connector_error((M2MInterface::Error)MbedCloudClient::ConnectorFailedToReadCredentials,CREDENTIAL_ERROR);
                delete _register_security;
                _register_security = NULL;
            }
        }
    } else {
        tr_debug("ConnectorClient::create_register_object() - Credentials already exists");
    }
}

/*
*  Creates bootstrap server object with mbed device server address and other parameters
*  required for client to connect to mbed device server.
*/
bool ConnectorClient::create_bootstrap_object()
{
    tr_debug("ConnectorClient::create_bootstrap_object");
    bool success = true;

    // Check if bootstrap credentials are already stored in KCM
    if (bootstrap_credentials_stored_in_kcm()) {
        if (!_bootstrap_security) {
            _bootstrap_security = M2MInterfaceFactory::create_security(M2MSecurity::Bootstrap);
            if (_bootstrap_security) {
                _bootstrap_security->set_bootstrap_flow(true);
                _bootstrap_security->set_resource_value(M2MSecurity::SecurityMode, M2MSecurity::Certificate);
                tr_debug("ConnectorClient::create_bootstrap_object - use credentials from KCM");

                // Allocate scratch buffer, this will be used to copy parameters from storage to security object
                size_t real_size = 0;
                const int max_size = 1024;
                uint8_t buffer[max_size];
                // XXX: get rid of these copies and memsets

                // Bootstrap URI
                if (success) {
                    success = false;
                    if (get_config_parameter(g_fcc_bootstrap_server_uri_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                        success = true;
                        // Include internal endpoint name into server uri if exists.
                        // This is needed aif device is already bootstrapped once.
                        uint8_t iep[MAX_ALLOWED_STRING_LENGTH];
                        size_t iep_size = 0;
                        if (get_config_parameter(KEY_INTERNAL_ENDPOINT, iep, MAX_ALLOWED_STRING_LENGTH, &iep_size) == CCS_STATUS_SUCCESS) {
                            tr_debug("ConnectorClient::create_bootstrap_object - iep: %.*s", (int)iep_size, iep);
                            strcat((char*)buffer, INTERNAL_ENDPOINT_PARAM);
                            strcat((char*)buffer, (const char*)iep);
                            real_size += iep_size + strlen(INTERNAL_ENDPOINT_PARAM) + 1;
                        }

                        tr_debug("ConnectorClient::create_bootstrap_object - M2MServerUri %.*s", (int)real_size, buffer);
                        _bootstrap_security->set_resource_value(M2MSecurity::M2MServerUri, buffer, real_size);
                    }
                }

                // Bootstrap server public key (certificate)
                if (success) {
                    success = false;
                    if (get_config_certificate(g_fcc_bootstrap_server_ca_certificate_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                        success = true;
                        tr_debug("ConnectorClient::create_bootstrap_object - ServerPublicKey %d", (int)real_size);
                        _bootstrap_security->set_resource_value(M2MSecurity::ServerPublicKey, buffer, real_size);
                    }
                }

                // Bootstrap client public key (certificate)
                if (success) {
                    success = false;
                    if (get_config_certificate(g_fcc_bootstrap_device_certificate_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                        success = true;
                        tr_debug("ConnectorClient::create_bootstrap_object - PublicKey %d", (int)real_size);
                        _bootstrap_security->set_resource_value(M2MSecurity::PublicKey, buffer, real_size);
                    }
                }

                // Bootstrap client private key
                if (success) {
                    success = false;
                    if (get_config_private_key(g_fcc_bootstrap_device_private_key_name, buffer, max_size, &real_size) == CCS_STATUS_SUCCESS) {
                        success = true;
                        tr_debug("ConnectorClient::create_bootstrap_object - Secretkey %d", (int)real_size);
                        _bootstrap_security->set_resource_value(M2MSecurity::Secretkey, buffer, real_size);
                    }
                }

                // Endpoint
                if (success) {
                    success = false;
                    if (get_config_parameter(g_fcc_endpoint_parameter_name,(uint8_t*) _endpoint_info.endpoint_name, MAX_ALLOWED_STRING_LENGTH, &real_size) == CCS_STATUS_SUCCESS) {
                        success = true;
                        tr_debug("ConnectorClient::create_bootstrap_object - Endpoint %s", _endpoint_info.endpoint_name);
                    }
                }

                // Account ID, not mandatory
                if (success) {
                    if (get_config_parameter(KEY_ACCOUNT_ID,(uint8_t*) _endpoint_info.account_id, MAX_ALLOWED_STRING_LENGTH, &real_size) == CCS_STATUS_SUCCESS) {
                        tr_debug("ConnectorClient::create_bootstrap_object - AccountId %s", _endpoint_info.account_id);
                    }
                }

                if (!success) {
                    tr_error("ConnectorClient::create_bootstrap_object - Failed to read credentials");
                    _callback->connector_error((M2MInterface::Error)MbedCloudClient::ConnectorFailedToReadCredentials,CREDENTIAL_ERROR);
                    delete _bootstrap_security;
                    _bootstrap_security = NULL;
                }
            }
        } else {
            success = true;
            tr_debug("ConnectorClient::create_bootstrap_object - bootstrap object already done");
        }
    // Device not provisioned
    } else {
        _callback->connector_error((M2MInterface::Error)MbedCloudClient::ConnectorInvalidCredentials, DEVICE_NOT_PROVISIONED);
        tr_error("ConnectorClient::create_bootstrap_object - device not provisioned!");
    }
    return success;
}

void ConnectorClient::state_bootstrap_start()
{
    tr_debug("ConnectorClient::state_bootstrap_start()");
    assert(_interface != NULL);
    assert(_bootstrap_security != NULL);

    _interface->bootstrap(_bootstrap_security);

    internal_event(State_Bootstrap_Started);
}

void ConnectorClient::state_bootstrap_started()
{
    // this state may be useful only for verifying the callbacks?
}

void ConnectorClient::state_bootstrap_success()
{
    assert(_callback != NULL);
    // Parse internal endpoint name from mDS cert
    delete _bootstrap_security;
    _bootstrap_security = NULL;

    _callback->registration_process_result(State_Bootstrap_Success);
}

void ConnectorClient::state_bootstrap_failure()
{
    assert(_callback != NULL);
    // maybe some additional canceling and/or leanup is needed here?
    _callback->registration_process_result(State_Bootstrap_Failure);
}

void ConnectorClient::state_registration_start()
{
    assert(_interface != NULL);
    assert(_register_security != NULL);
    _interface->register_object(_register_security, *_client_objs);
    internal_event(State_Registration_Started);
}

void ConnectorClient::state_registration_started()
{
    // this state may be useful only for verifying the callbacks?
}

void ConnectorClient::state_registration_success()
{
    assert(_callback != NULL);
    memcpy(_endpoint_info.internal_endpoint_name, _interface->internal_endpoint_name_str(), strlen(_interface->internal_endpoint_name_str()));
    //The endpoint is maximum 32 character long, we put bigger buffer for future extensions
    size_t real_size = 0;

    //If this returns success, don't do anything else delete old value and set new one
    if (size_config_parameter(KEY_INTERNAL_ENDPOINT, &real_size) != CCS_STATUS_SUCCESS) {
            delete_config_parameter(KEY_INTERNAL_ENDPOINT);
            set_config_parameter(KEY_INTERNAL_ENDPOINT,(const uint8_t*)_endpoint_info.internal_endpoint_name,
                                     (size_t)strlen(_endpoint_info.internal_endpoint_name));
    }
    _callback->registration_process_result(State_Registration_Success);
}

void ConnectorClient::state_registration_failure()
{
    assert(_callback != NULL);
    // maybe some additional canceling and/or leanup is needed here?
    _callback->registration_process_result(State_Registration_Failure);
}

void ConnectorClient::state_unregistered()
{
    assert(_callback != NULL);
    _callback->registration_process_result(State_Unregistered);
}

void ConnectorClient::bootstrap_done(M2MSecurity *server_object)
{
    _bootstrap_done = true;
    _register_security = server_object; // TODO: Memory leak if below line fails?
    ccs_status_e status = set_connector_credentials(_register_security);
    tr_error("set_connector_credentials status %d", status);
    if (status != CCS_STATUS_SUCCESS) {
        internal_event(State_Bootstrap_Failure);
        //Failed to store credentials, bootstrap failed
        _callback->connector_error(M2MInterface::MemoryFail,ERROR_NO_MEMORY); // Translated to error code ConnectMemoryConnectFail
        return;
    }
    internal_event(State_Bootstrap_Success);
}

void ConnectorClient::object_registered(M2MSecurity *security_object, const M2MServer &server_object)
{
    internal_event(State_Registration_Success);
}

void ConnectorClient::object_unregistered(M2MSecurity *server_object)
{
    internal_event(State_Unregistered);
}

void ConnectorClient::registration_updated(M2MSecurity *security_object, const M2MServer & server_object)
{
}

void ConnectorClient::error(M2MInterface::Error error)
{
    tr_error("ConnectorClient::error() - error: %d", error);
    assert(_callback != NULL);
    if (_current_state >= State_Registration_Start &&
            use_bootstrap() &&
            (error == M2MInterface::SecureConnectionFailed ||
            error == M2MInterface::InvalidParameters)) {
        tr_info("ConnectorClient::error() - Error during lwm2m registration");
        tr_info("ConnectorClient::error() - Clearing lwm2m credentials");
        // delete the old connector credentials when DTLS handshake fails or
        // server rejects the registration.
        delete_config_parameter(g_fcc_lwm2m_server_uri_name);
        delete_config_certificate(g_fcc_lwm2m_server_ca_certificate_name);
        delete_config_certificate(g_fcc_lwm2m_device_certificate_name);
        delete_config_private_key(g_fcc_lwm2m_device_private_key_name);
        // Delete bootstrap security object
        delete _bootstrap_security;
        _bootstrap_security = NULL;
        // Start re-bootstrap timer
        tr_info("ConnectorClient::error() - Re-bootstrapping in 100 milliseconds");
        _rebootstrap_timer.start_timer(100, M2MTimerObserver::BootstrapFlowTimer, true);
    }
    else {
        _callback->connector_error(error, _interface->error_description());
    }
}

void ConnectorClient::value_updated(M2MBase *base, M2MBase::BaseType type)
{
    assert(_callback != NULL);
    _callback->value_updated(base, type);
}

bool ConnectorClient::connector_credentials_available()
{
    tr_debug("ConnectorClient::connector_credentials_available");
    const int max_size = 512;
    uint8_t buffer[max_size];
    size_t real_size = 0;
    get_config_private_key(g_fcc_lwm2m_device_private_key_name, buffer, max_size, &real_size);
    if (real_size > 0) {
        return true;
    }
    return false;
}

bool ConnectorClient::use_bootstrap()
{
    tr_debug("ConnectorClient::use_bootstrap");
    const int max_size = 32;
    uint8_t buffer[max_size];
    bool ret = false;
    size_t real_size = 0;
    ccs_status_e status = get_config_parameter(g_fcc_use_bootstrap_parameter_name, buffer, max_size, &real_size);
    if (status == CCS_STATUS_SUCCESS && real_size > 0 && buffer[0] > 0) {
        ret = true;
    }
    return ret;
}

ccs_status_e ConnectorClient::set_connector_credentials(M2MSecurity *security)
{
    tr_debug("ConnectorClient::set_connector_credentials");
    ccs_status_e status = CCS_STATUS_ERROR;

    const uint8_t *srv_public_key = NULL;
    const uint8_t *public_key = NULL;
    const uint8_t *sec_key = NULL;

    uint32_t srv_public_key_size = security->resource_value_buffer(M2MSecurity::ServerPublicKey, srv_public_key);
    uint32_t public_key_size = security->resource_value_buffer(M2MSecurity::PublicKey, public_key);
    uint32_t sec_key_size = security->resource_value_buffer(M2MSecurity::Secretkey, sec_key);

    if(srv_public_key && public_key && sec_key) {
        // Parse common name
        if (extract_cn_from_certificate(public_key, public_key_size, _endpoint_info.internal_endpoint_name)){
            tr_debug("ConnectorClient::set_connector_credentials - CN: %s", _endpoint_info.internal_endpoint_name);
            delete_config_parameter(KEY_INTERNAL_ENDPOINT);
            status = set_config_parameter(KEY_INTERNAL_ENDPOINT,(uint8_t*)_endpoint_info.internal_endpoint_name, strlen(_endpoint_info.internal_endpoint_name));
        }

        if(status == CCS_STATUS_SUCCESS) {
            delete_config_certificate(g_fcc_lwm2m_server_ca_certificate_name);
            status = set_config_certificate(g_fcc_lwm2m_server_ca_certificate_name,
                                            srv_public_key,
                                            (size_t)srv_public_key_size);
        }
        if(status == CCS_STATUS_SUCCESS) {
            status = set_config_certificate(g_fcc_lwm2m_device_certificate_name,
                                            public_key,
                                            (size_t)public_key_size);
        }
        if(status == CCS_STATUS_SUCCESS) {
            status = set_config_private_key(g_fcc_lwm2m_device_private_key_name,
                                            sec_key,
                                            (size_t)sec_key_size);
        }

        if(status == CCS_STATUS_SUCCESS) {
            delete_config_parameter(KEY_ACCOUNT_ID);
            // AccountID optional so don't fail if unable to store
            set_config_parameter(KEY_ACCOUNT_ID,
                                 (const uint8_t*)_endpoint_info.account_id,
                                 (size_t)strlen(_endpoint_info.account_id));
        }
        if(status == CCS_STATUS_SUCCESS) {

        M2MResource* res = security->get_resource(M2MSecurity::M2MServerUri);
        status = set_config_parameter(g_fcc_lwm2m_server_uri_name,
                                      (const uint8_t*)res->value(),
                                      (size_t)res->value_length());

        }
        M2MDevice *device = M2MInterfaceFactory::create_device();
        if (device) {
            uint32_t currenttime = (uint32_t)device->resource_value_int(M2MDevice::CurrentTime, 0);

            uint8_t data[4];
            memcpy(data, &currenttime, 4);
            delete_config_parameter(g_fcc_current_time_parameter_name);
            set_config_parameter(g_fcc_current_time_parameter_name, data, 4);

            M2MResourceBase* res = device->get_resource_instance(M2MDevice::Timezone, 0);

            if(res) {
                delete_config_parameter(g_fcc_device_time_zone_parameter_name);
                set_config_parameter(g_fcc_device_time_zone_parameter_name, (const uint8_t*)res->value(), res->value_length());
            }

            res = device->get_resource_instance(M2MDevice::UTCOffset, 0);
            if(res) {
                delete_config_parameter(g_fcc_offset_from_utc_parameter_name);
                set_config_parameter(g_fcc_offset_from_utc_parameter_name, (const uint8_t*)res->value(), res->value_length());
            }

            status = CCS_STATUS_SUCCESS;
        }
        else {
            tr_debug("No device object to store!");
        }
    }

    return status;
}

const ConnectorClientEndpointInfo *ConnectorClient::endpoint_info() const
{
    return &_endpoint_info;
}

bool ConnectorClient::bootstrap_credentials_stored_in_kcm()
{
    size_t real_size = 0;
    ccs_status_e success = size_config_parameter(g_fcc_bootstrap_server_uri_name, &real_size);
    // Return true if bootstrap uri exists in KCM
    if ((success == CCS_STATUS_SUCCESS) && real_size > 0) {
        return true;
    } else {
        return false;
    }
}

void ConnectorClient::timer_expired(M2MTimerObserver::Type type)
{
    if (type == M2MTimerObserver::BootstrapFlowTimer) {
        start_bootstrap();
    }
}

M2MInterface::BindingMode ConnectorClient::transport_mode()
{
    M2MInterface::BindingMode mode = M2MInterface::UDP;
#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP
    mode = M2MInterface::UDP;
#elif defined MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
    mode = M2MInterface::TCP;
#elif defined MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    mode = M2MInterface::UDP_QUEUE;
#elif defined MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE
    mode = M2MInterface::TCP_QUEUE;
#endif
    return mode;
}
