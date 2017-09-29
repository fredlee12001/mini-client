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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include <string>
#include "include/ServiceClient.h"
#include "include/CloudClientStorage.h"
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "include/UpdateClientResources.h"
#endif
#include "factory_configurator_client.h"
#include "mbed-trace/mbed_trace.h"

#include <assert.h>

#define TRACE_GROUP "svcc"

#define CONNECT 0
#define ERROR_UPDATE "Update has failed, check MbedCloudClient::Error"

/* lookup table for printing hexadecimal values */
const uint8_t ServiceClient::hex_table[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

ServiceClient::ServiceClient(ServiceClientCallback& callback)
: _service_callback(callback),
  _service_uri(NULL),
  _stack(NULL),
  _client_objs(NULL),
  _current_state(State_Init),
  _event_generated(false),
  _state_engine_running(false),
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
  _setup_update_client(false),
#endif
  _connector_client(this)
{
}

ServiceClient::~ServiceClient()
{
}

void ServiceClient::initialize_and_register(M2MObjectList& client_objs)
{
    tr_info("ServiceClient::initialize_and_register");
    if(_current_state == State_Init ||
       _current_state == State_Unregister ||
       _current_state == State_Failure) {
        _client_objs = &client_objs;

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        tr_info("ServiceClient::initialize_and_register: update client supported");

        if(!_setup_update_client) {
            _setup_update_client = true;

#ifdef MBED_CLOUD_DEV_UPDATE_ID
            /* Overwrite values stored in KCM. This is for development only
               since these IDs should be provisioned in the factory.
            */
            tr_info("ServiceClient::initialize_and_register: update IDs defined");

            /* replace manufacturer name */
            delete_config_parameter(g_fcc_manufacturer_parameter_name);
            set_config_parameter(g_fcc_manufacturer_parameter_name,
                                 arm_uc_vendor_id,
                                 arm_uc_vendor_id_size);

            /* replace model number */
            delete_config_parameter(g_fcc_model_number_parameter_name);
            set_config_parameter(g_fcc_model_number_parameter_name,
                                 arm_uc_class_id,
                                 arm_uc_class_id_size);
#endif /* MBED_CLOUD_DEV_UPDATE_ID */

#ifdef ARM_UPDATE_CLIENT_VERSION
            /* Inject Update Client version number if no other software
               version is present in the KCM.
            */
            tr_info("ServiceClient::initialize_and_register: update version defined");

            ccs_status_e status = CCS_STATUS_ERROR;
            const size_t buffer_size = 16;
            uint8_t buffer[buffer_size];
            size_t size = 0;

            /* check if software version is already set */
            status = get_config_parameter(KEY_DEVICE_SOFTWAREVERSION,
                                          buffer, buffer_size, &size);

            if (status == CCS_STATUS_KEY_DOESNT_EXIST) {
                tr_info("ServiceClient::initialize_and_register: insert update version");

                /* insert value from Update Client Common */
                set_config_parameter(KEY_DEVICE_SOFTWAREVERSION,
                                     (const uint8_t*) ARM_UPDATE_CLIENT_VERSION,
                                     sizeof(ARM_UPDATE_CLIENT_VERSION));
            }
#endif /* ARM_UPDATE_CLIENT_VERSION */

            /* Update Client adds the OMA LWM2M Firmware Update object */
            UpdateClient::populate_object_list(*_client_objs);

            /* Initialize Update Client */
            FP1<void, int32_t> callback(this, &ServiceClient::update_error_callback);
            UpdateClient::UpdateClient(callback);
        }
#endif /* MBED_CLOUD_CLIENT_SUPPORT_UPDATE */

        /* Device Object is mandatory.
           Get instance and add it to object list
        */
        M2MDevice *device_object = device_object_from_storage();

        if (device_object) {
            /* Publish device object resource to mds */
            M2MResource *res = device_object->object_instance()->resources();

            while (res) {
                res->set_register_uri(true);

                res = res->get_next();
            }

            /* Add Device Object to object list. */
            _client_objs->push_back(device_object);
        }

        internal_event(State_Bootstrap);
    } else if (_current_state == State_Success) { 
        state_success();
    }
}

ConnectorClient &ServiceClient::connector_client()
{
    return _connector_client;
}

const ConnectorClient &ServiceClient::connector_client() const
{
    return _connector_client;
}

// generates an internal event. called from within a state
// function to transition to a new state
void ServiceClient::internal_event(StartupMainState new_state)
{
    tr_debug("ServiceClient::internal_event: state: %d -> %d", _current_state, new_state);

    _event_generated = true;
    _current_state = new_state;

    if (!_state_engine_running) {
        state_engine();
    }
}

// the state engine executes the state machine states
void ServiceClient::state_engine(void)
{
    tr_debug("ServiceClient::state_engine");

    // this simple flagging gets rid of recursive calls to this method
    _state_engine_running = true;

    // while events are being generated keep executing states
    while (_event_generated) {
        _event_generated = false;  // event used up, reset flag

        state_function(_current_state);
    }

    _state_engine_running = false;
}

void ServiceClient::state_function(StartupMainState current_state)
{
    switch (current_state) {
        case State_Init:        // -> Goes to bootstrap state
        case State_Bootstrap:    // -> State_Register OR State_Failure
            state_bootstrap();
            break;
        case State_Register:     // -> State_Succes OR State_Failure
            state_register();
            break;
        case State_Success:      // return success to user
            state_success();
            break;
        case State_Failure:      // return error to user
            state_failure();
            break;
        case State_Unregister:   // return error to user
            state_unregister();
            break;
    }
}

void ServiceClient::state_bootstrap()
{
    tr_info("ServiceClient::state_bootstrap()");
    bool credentials_ready = _connector_client.connector_credentials_available();
    bool bootstrap = _connector_client.use_bootstrap();
    tr_info("ServiceClient::state_bootstrap() - credentials available: %d", credentials_ready);
    tr_info("ServiceClient::state_bootstrap() - use bs: %d", bootstrap);
    if (credentials_ready || !bootstrap) {
        internal_event(State_Register);
    } else {
        _connector_client.start_bootstrap();
    }
}

void ServiceClient::state_register()
{
    tr_info("ServiceClient::state_register()");
    _connector_client.start_registration(_client_objs);
}

void ServiceClient::registration_process_result(ConnectorClient::StartupSubStateRegistration status)
{
    tr_debug("ServiceClient::registration_process_result(): status: %d", status);
    if (status == ConnectorClient::State_Registration_Success) {
        internal_event(State_Success);
    } else if(status == ConnectorClient::State_Registration_Failure ||
              status == ConnectorClient::State_Bootstrap_Failure){
        internal_event(State_Failure); // XXX: the status should be saved to eg. event object
    }
    if(status == ConnectorClient::State_Bootstrap_Success) {
        internal_event(State_Register);
    }
    if(status == ConnectorClient::State_Unregistered) {
        internal_event(State_Unregister);
    }
}

void ServiceClient::connector_error(M2MInterface::Error error, const char *reason)
{
    tr_error("ServiceClient::connector_error() error %d", (int)error);
    if (_current_state == State_Register) {
        registration_process_result(ConnectorClient::State_Registration_Failure);
    }
    else if (_current_state == State_Bootstrap) {
        registration_process_result(ConnectorClient::State_Bootstrap_Failure);
    }
    _service_callback.error(int(error),reason);
    internal_event(State_Failure);
}

void ServiceClient::value_updated(M2MBase *base, M2MBase::BaseType type)
{
    tr_debug("ServiceClient::value_updated()");
    _service_callback.value_updated(base, type);
}

void ServiceClient::state_success()
{
    tr_info("ServiceClient::state_success()");
    // this is verified already at client API level, but this might still catch some logic failures
    _service_callback.complete(0);
}

void ServiceClient::state_failure()
{
    tr_error("ServiceClient::state_failure()");
    _service_callback.complete(-1); // XXX
}

void ServiceClient::state_unregister()
{
    tr_debug("ServiceClient::state_unregister()");
    _service_callback.complete(1); // XXX
}

M2MDevice* ServiceClient::device_object_from_storage()
{
    M2MDevice *device_object = M2MInterfaceFactory::create_device();
    if (device_object == NULL) {
        return NULL;
    }

    ccs_status_e status = CCS_STATUS_ERROR;
    const size_t buffer_size = MAX_ALLOWED_STRING_LENGTH;
    uint8_t buffer[MAX_ALLOWED_STRING_LENGTH];
    memset(buffer, 0, buffer_size);
    size_t size = 0;

    status = get_config_parameter(g_fcc_manufacturer_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::Manufacturer, (const char*)buffer);
    }
    status = get_config_parameter(g_fcc_model_number_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::ModelNumber, (const char*)buffer);
    }
    status = get_config_parameter(g_fcc_device_serial_number_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::SerialNumber, (const char*)buffer);
    }
    status = get_config_parameter(g_fcc_device_type_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::DeviceType, (const char*)buffer);
    }
    status = get_config_parameter(g_fcc_hardware_version_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::HardwareVersion, (const char*)buffer);
    }
    status = get_config_parameter(KEY_DEVICE_SOFTWAREVERSION, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::SoftwareVersion, (const char*)buffer);
    }
    uint8_t data[4];
    uint32_t value;
    status = get_config_parameter(g_fcc_memory_size_parameter_name, data, 4, &size);
    if (status == CCS_STATUS_SUCCESS) {
        memcpy(&value, data, 4);
        device_object->create_resource(M2MDevice::MemoryTotal, value);
        tr_debug("setting memory total value %" PRIu32 " (%s)", value, tr_array(data, 4));
    }
    status = get_config_parameter(g_fcc_current_time_parameter_name, data, 4, &size);
    if (status == CCS_STATUS_SUCCESS) {
        memcpy(&value, data, 4);
        device_object->create_resource(M2MDevice::CurrentTime, value);
        tr_debug("setting current time value %" PRIu32 " (%s)", value, tr_array(data, 4));
    }
    status = get_config_parameter(g_fcc_device_time_zone_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::Timezone, (const char*)buffer);
    }
    status = get_config_parameter(g_fcc_offset_from_utc_parameter_name, buffer, buffer_size, &size);
    if (status == CCS_STATUS_SUCCESS) {
        device_object->create_resource(M2MDevice::UTCOffset, (const char*)buffer);
    }
    return device_object;
}

/**
 * \brief Set resource value in the Device Object
 *
 * \param resource Device enum to have value set.
 * \param value char* object.
 * \return True if successful, false otherwise.
 */
bool ServiceClient::set_device_resource_value(M2MDevice::DeviceResource resource,
                                              const char *value)
{
    return set_device_resource_value(resource,
                                     value,
                                     strlen(value));
}

/**
 * \brief Set resource value in the Device Object
 *
 * \param resource Device enum to have value set.
 * \param value Byte buffer.
 * \param length Buffer length.
 * \return True if successful, false otherwise.
 */
bool ServiceClient::set_device_resource_value(M2MDevice::DeviceResource resource,
                                              const char* value,
                                              uint32_t length)
{
    bool retval = false;

    /* sanity check */
    if (value && (length < 256))
    {
#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
        /* Pass resource value to Update Client.
           Used for validating the manifest.
        */
        switch (resource) {
            case M2MDevice::Manufacturer:
                ARM_UC_SetVendorId((const uint8_t*) value, length);
                break;
            case M2MDevice::ModelNumber:
                ARM_UC_SetClassId((const uint8_t*) value, length);
                break;
            default:
                break;
        }
#endif

        /* Convert resource to printable string if necessary */

        /* Getting object instance from factory */
        M2MDevice *device_object = M2MInterfaceFactory::create_device();

        /* Check device object and resource both are present */
        if (device_object && device_object->is_resource_present(resource)) {
            /* set counter to not-zero */
            uint8_t printable_length = 0xFF;

            /* set printable_length to 0 if the buffer is not printable */
            for (uint8_t index = 0; index < length; index++) {
                /* break if character is not printable */
                if ((value[index] < ' ') || (value[index] > '~')) {
                    printable_length = 0;
                    break;
                }
            }

            /* resource is a string */
            if (printable_length != 0) {
                /* reset counter */
                printable_length = 0;

                /* find actual printable length */
                for ( ; printable_length < length; printable_length++) {
                    /* break prematurely if end-of-string character is found */
                    if (value[printable_length] == '\0') {
                        break;
                    }
                }

                /* convert to string and set value in object */
                M2MResourceBase *res = device_object->get_resource_instance(resource, 0);
                if(res) {
                    res->set_value((const uint8_t*)value, printable_length);
                }
            }
            else
            {
                /* resource is a byte array */
                char value_buffer[0xFF] = { 0 };

                /* count length */
                uint8_t index = 0;

                /* convert byte array to string */
                for ( ;
                     (index < length) && (index < 0xFF / 2);
                     index++) {

                    uint8_t byte = value[index];

                    value_buffer[2 * index]     = hex_table[byte >> 4];
                    value_buffer[2 * index + 1] = hex_table[byte & 0x0F];
                }


                M2MResourceBase *res = device_object->get_resource_instance(resource, 0);
                if(res) {
                    res->set_value((const uint8_t*)value_buffer, 2 * index);
                }

            }
        }
    }

    return retval;
}

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
void ServiceClient::set_update_authorize_handler(void (*handler)(int32_t request))
{
    UpdateClient::set_update_authorize_handler(handler);
}

void ServiceClient::update_authorize(int32_t request)
{
    UpdateClient::update_authorize(request);
}

void ServiceClient::set_update_progress_handler(void (*handler)(uint32_t progress, uint32_t total))
{
    UpdateClient::set_update_progress_handler(handler);
}

void ServiceClient::update_error_callback(int32_t error)
{
    _service_callback.error(error, ERROR_UPDATE);
}
#endif
