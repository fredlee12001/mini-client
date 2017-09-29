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

#include "mbed-cloud-client/MbedCloudClientConfig.h"
#include "mbed-cloud-client/MbedCloudClient.h"
#include "mbed-cloud-client/SimpleM2MResource.h"

#include "ns_hal_init.h"
#include "mbed-trace/mbed_trace.h"

#include <assert.h>

#define xstr(s) str(s)
#define str(s) #s

#define TRACE_GROUP "MCCT"

#ifdef MBED_CONF_MBED_CLIENT_EVENT_LOOP_SIZE
    #define MBED_CLIENT_EVENT_LOOP_SIZE MBED_CONF_MBED_CLIENT_EVENT_LOOP_SIZE
#else
    #define MBED_CLIENT_EVENT_LOOP_SIZE 1024
#endif

//static uint8_t EVENT_LOOP_POOL[MBED_CLIENT_EVENT_LOOP_SIZE];

MbedCloudClient::MbedCloudClient()
:_client(*this),
 _value_callback(NULL),
 _error_description(NULL)
{
    ns_hal_init(NULL, MBED_CLIENT_EVENT_LOOP_SIZE, NULL, NULL);
}

MbedCloudClient::~MbedCloudClient()
{    
    _object_list.clear();
}

void MbedCloudClient::add_objects(const M2MObjectList& object_list)
{
    if(!object_list.empty()) {
        M2MObjectList::const_iterator it;
        it = object_list.begin();
        for (; it!= object_list.end(); it++) {
            _object_list.push_back(*it);
        }
    }
}

void MbedCloudClient::set_update_callback(MbedCloudClientCallback *callback)
{
    _value_callback = callback;
}

bool MbedCloudClient::setup(void* iface)
{
    tr_debug("[MbedCloudClient] setup()");    
    // Add objects to list
    m2m::Map<M2MObject*>::iterator it;
    for (it = _objects.begin(); it != _objects.end(); it++)
    {
        _object_list.push_back(*it);
    }
    _client.connector_client().m2m_interface()->set_platform_network_handler(iface);

    _client.initialize_and_register(_object_list);
    return true;
}

void MbedCloudClient::on_registered(void(*fn)(void))
{
    FP0<void> fp(fn);
    _on_registered = fp;
}


void MbedCloudClient::on_error(void(*fn)(int))
{
    _on_error = fn;
}


void MbedCloudClient::on_unregistered(void(*fn)(void))
{
    FP0<void> fp(fn);
    _on_unregistered = fp;
}


void MbedCloudClient::keep_alive()
{    
    _client.connector_client().update_registration();
}

void MbedCloudClient::close()
{
    _client.connector_client().m2m_interface()->unregister_object(NULL);
}

const ConnectorClientEndpointInfo *MbedCloudClient::endpoint_info() const
{
    return _client.connector_client().endpoint_info();
}

void MbedCloudClient::set_queue_sleep_handler(callback_handler handler)
{
    _client.connector_client().m2m_interface()->set_queue_sleep_handler(handler);
}

void MbedCloudClient::set_random_number_callback(random_number_cb callback)
{
    _client.connector_client().m2m_interface()->set_random_number_callback(callback);
}

void MbedCloudClient::set_entropy_callback(entropy_cb callback)
{
    _client.connector_client().m2m_interface()->set_entropy_callback(callback);
}

bool MbedCloudClient::set_device_resource_value(M2MDevice::DeviceResource resource,
                                                const char *value)
{
    return _client.set_device_resource_value(resource, value);
}

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
void MbedCloudClient::set_update_authorize_handler(void (*handler)(int32_t request))
{
    _client.set_update_authorize_handler(handler);
}

void MbedCloudClient::set_update_progress_handler(void (*handler)(uint32_t progress, uint32_t total))
{    
    _client.set_update_progress_handler(handler);
}

void MbedCloudClient::update_authorize(int32_t request)
{
    _client.update_authorize(request);
}
#endif

const char *MbedCloudClient::error_description() const
{
    return _error_description;
}


void MbedCloudClient::register_update_callback(const char *route,
                                               SimpleM2MResourceBase* resource)
{
    _update_values[route] = resource;
}

void MbedCloudClient::complete(int status)
{
    tr_debug("[MbedCloudClient] [complete] status (%d)", status);
    if(status == 0) {
        _on_registered.call();
    } else if(status == 1) {
        _object_list.clear();
        _on_unregistered.call();
    }
}

void MbedCloudClient::error(int error, const char *reason)
{
    tr_error("[MbedCloudClient] [error] code (%d)", error);
    _error_description = reason;
    _on_error(error);
}

void MbedCloudClient::value_updated(M2MBase *base, M2MBase::BaseType type)
{
    if (base) {
        path_buffer buffer;
        base->uri_path(buffer);
        tr_debug("MbedCloudClient::value_updated path %s", buffer.c_str());
        if (buffer.c_str()) {
            if (_update_values.count(buffer.c_str()) != 0) {
                tr_debug("MbedCloudClient::value_updated calling update() for %s", buffer.c_str());
                _update_values[buffer.c_str()]->update();
            } else {
                // way to tell application that there is a value update
                assert(_value_callback != NULL);
                _value_callback->value_updated(base, type);
            }
        }
    }
}
