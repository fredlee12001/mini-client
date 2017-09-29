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
#include "mbed-client/m2msecurity.h"
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mobjectinstance.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mstring.h"
#include "mbed-trace/mbed_trace.h"

#include <stdlib.h>

#define TRACE_GROUP "mClt"

#define BUFFER_SIZE 21

M2MSecurity::M2MSecurity(ServerType ser_type)
: M2MObject(M2M_SECURITY_ID, NULL),
 _server_instance(NULL),
 _server_type(ser_type),
 _is_bootstrap_flow(false),
 _device_certificate(NULL),
 _server_certificate(NULL),
 _device_key(NULL)
{
}

bool M2MSecurity::allocate_resources()
{
    if(NULL == (_server_instance = M2MObject::create_object_instance())) {
        return false;
    }

    M2MResource* res = _server_instance->create_dynamic_resource(SECURITY_M2M_SERVER_URI,
                                                                OMA_RESOURCE_TYPE,
                                                                M2MResourceInstance::STRING,
                                                                false);

    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }
    res->set_operation(M2MBase::GET_PUT_ALLOWED);


    res = _server_instance->create_dynamic_resource(SECURITY_BOOTSTRAP_SERVER,
                                                   OMA_RESOURCE_TYPE,
                                                   M2MResourceInstance::BOOLEAN,
                                                   false);

    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }
    res->set_operation(M2MBase::NOT_ALLOWED);

    res = _server_instance->create_dynamic_resource(SECURITY_SECURITY_MODE,
                                                   OMA_RESOURCE_TYPE,
                                                   M2MResourceInstance::INTEGER,
                                                   false);

    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }
    res->set_operation(M2MBase::NOT_ALLOWED);

    res = _server_instance->create_dynamic_resource(SECURITY_PUBLIC_KEY,
                                                   OMA_RESOURCE_TYPE,
                                                   M2MResourceInstance::OPAQUE,
                                                   false);

    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }

    res->set_operation(M2MBase::NOT_ALLOWED);

    res = _server_instance->create_dynamic_resource(SECURITY_SERVER_PUBLIC_KEY,
                                                   OMA_RESOURCE_TYPE,
                                                   M2MResourceInstance::OPAQUE,
                                                   false);

    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }
    res->set_operation(M2MBase::NOT_ALLOWED);


    res = _server_instance->create_dynamic_resource(SECURITY_SECRET_KEY,
                                                   OMA_RESOURCE_TYPE,
                                                   M2MResourceInstance::OPAQUE,
                                                   false);
    if(!res) {
       M2MObject::remove_object_instance();
       return false;
    }
    res->set_operation(M2MBase::NOT_ALLOWED);

    if(M2MSecurity::M2MServer == _server_type) {
       res = _server_instance->create_dynamic_resource(SECURITY_SHORT_SERVER_ID,
                                                       OMA_RESOURCE_TYPE,
                                                       M2MResourceInstance::INTEGER,
                                                       false);

       if(!res) {
           M2MObject::remove_object_instance();
           return false;
       }
       res->set_operation(M2MBase::NOT_ALLOWED);
    }
    return true;
}

M2MSecurity::~M2MSecurity()
{
}

M2MResource* M2MSecurity::create_resource(SecurityResource resource, uint32_t value)
{
    M2MResource* res = NULL;
    int security_id = -1;
    if(!is_resource_present(resource)) {
        switch(resource) {
            case SMSSecurityMode:
               security_id = SECURITY_SMS_SECURITY_MODE;
               break;
            case M2MServerSMSNumber:
                security_id = SECURITY_M2M_SERVER_SMS_NUMBER;
                break;
            case ShortServerID:
                security_id = SECURITY_SHORT_SERVER_ID;
                break;
            case ClientHoldOffTime:
                security_id = SECURITY_CLIENT_HOLD_OFF_TIME;
                break;
            default:
                break;
        }
    }

    if (security_id >= 0) {
        if(_server_instance) {
            res = _server_instance->create_dynamic_resource(security_id,OMA_RESOURCE_TYPE,
                                                            M2MResourceInstance::INTEGER,
                                                            false);

            if(res) {
                res->set_operation(M2MBase::NOT_ALLOWED);
                res->set_value(value);
            }
        }
    }
    return res;
}

bool M2MSecurity::delete_resource(SecurityResource resource)
{
    bool success = false;
    int security_id;
    switch (resource) {
        case SMSSecurityMode:
           security_id = SECURITY_SMS_SECURITY_MODE;
           break;
        case M2MServerSMSNumber:
            security_id = SECURITY_M2M_SERVER_SMS_NUMBER;
            break;
        case ShortServerID:
            if(M2MSecurity::Bootstrap == _server_type) {
                security_id = SECURITY_SHORT_SERVER_ID;
            } else {
                security_id = -1;
            }
            break;
        case ClientHoldOffTime:
            security_id = SECURITY_CLIENT_HOLD_OFF_TIME;
            break;
        default:
            // Others are mandatory resources hence cannot be deleted.
            security_id = -1;
            break;
    }

    if (security_id >= 0) {
        if (_server_instance) {
            success = _server_instance->remove_resource(security_id);
        }
    }
    return success;
}

bool M2MSecurity::set_resource_value(SecurityResource resource,
                                     const String &value)
{
    bool success = false;
    if(M2MSecurity::M2MServerUri == resource) {
        M2MResource* res = get_resource(resource);
        if(res) {
            success = res->set_value((const uint8_t*)value.c_str(),(uint32_t)value.length());
        }
    }
    return success;
}

bool M2MSecurity::set_resource_value(SecurityResource resource,
                                     uint32_t value)
{
    bool success = false;
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::SecurityMode == resource        ||
           M2MSecurity::SMSSecurityMode == resource     ||
           M2MSecurity::M2MServerSMSNumber == resource  ||
           M2MSecurity::ShortServerID == resource       ||
           M2MSecurity::ClientHoldOffTime == resource) {
            success = res->set_value(value);

        }
    }
    return success;
}

bool M2MSecurity::set_resource_value(SecurityResource resource,
                                     const uint8_t *value,
                                     const uint16_t length)
{
    bool success = false;
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::PublicKey == resource           ||
           M2MSecurity::ServerPublicKey == resource     ||
           M2MSecurity::Secretkey == resource           ||
           M2MSecurity::M2MServerUri == resource) {
            success = res->set_value(value,length);
        }
    }
    return success;
}

String M2MSecurity::resource_value_string(SecurityResource resource) const
{
    String value = ""; // XXX
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::M2MServerUri == resource) {
            value = res->get_value_string();
        }
    }
    return value;
}

uint32_t M2MSecurity::resource_value_buffer(SecurityResource resource,
                               uint8_t *&data) const
{
    uint32_t size = 0;
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::PublicKey == resource        ||
           M2MSecurity::ServerPublicKey == resource  ||
           M2MSecurity::Secretkey == resource) {
            res->get_value(data,size);
        }
    }
    return size;
}

uint32_t M2MSecurity::resource_value_buffer(SecurityResource resource,
                               const uint8_t *&data) const
{
    uint32_t size = 0;
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::PublicKey == resource        ||
           M2MSecurity::ServerPublicKey == resource  ||
           M2MSecurity::Secretkey == resource) {
            data = res->value();
            size = res->value_length();
        }
    }
    return size;
}


uint32_t M2MSecurity::resource_value_int(SecurityResource resource) const
{
    uint32_t value = 0;
    M2MResource* res = get_resource(resource);
    if(res) {
        if(M2MSecurity::SecurityMode == resource        ||
           M2MSecurity::SMSSecurityMode == resource     ||
           M2MSecurity::M2MServerSMSNumber == resource  ||
           M2MSecurity::ShortServerID == resource       ||
           M2MSecurity::ClientHoldOffTime == resource) {
            // note: the value may be 32bit int on 32b archs.
            value = res->get_value_int();
        }
    }
    return value;
}

bool M2MSecurity::is_resource_present(SecurityResource resource) const
{
    bool success = false;
    M2MResource *res = get_resource(resource);
    if(res) {
        success = true;
    }
    return success;
}

uint16_t M2MSecurity::total_resource_count() const
{
    uint16_t count = 0;
    if(_server_instance) {
        count = _server_instance->total_resource_count();
    }
    return count;
}

M2MSecurity::ServerType M2MSecurity::server_type() const
{
    return _server_type;
}

M2MResource* M2MSecurity::get_resource(SecurityResource res) const
{
    M2MResource* res_object = NULL;
    if(_server_instance) {
        int res_id = -1;
        switch(res) {
            case M2MServerUri:
                res_id = SECURITY_M2M_SERVER_URI;
                break;
            case BootstrapServer:
                res_id = SECURITY_BOOTSTRAP_SERVER;
                break;
            case SecurityMode:
                res_id = SECURITY_SECURITY_MODE;
                break;
            case PublicKey:
                res_id = SECURITY_PUBLIC_KEY;
                break;
            case ServerPublicKey:
                res_id = SECURITY_SERVER_PUBLIC_KEY;
                break;
            case Secretkey:
                res_id = SECURITY_SECRET_KEY;
                break;
            case SMSSecurityMode:
                res_id = SECURITY_SMS_SECURITY_MODE;
                break;
            case SMSBindingKey:
                res_id = SECURITY_SMS_BINDING_KEY;
                break;
            case SMSBindingSecretKey:
                res_id = SECURITY_SMS_BINDING_SECRET_KEY;
                break;
            case M2MServerSMSNumber:
                res_id = SECURITY_M2M_SERVER_SMS_NUMBER;
                break;
            case ShortServerID:
                res_id = SECURITY_SHORT_SERVER_ID;
                break;
            case ClientHoldOffTime:
                res_id = SECURITY_CLIENT_HOLD_OFF_TIME;
                break;
        }

        if (res_id >= 0) {
            res_object = _server_instance->resource(res_id);
        }
    }
    return res_object;
}

void M2MSecurity::clear_resources()
{
    for(int i = 0; i <= M2MSecurity::ClientHoldOffTime; i++) {
        M2MResource *res = get_resource((SecurityResource) i);
        if (res) {
            res->clear_value();
        }
    }
}

void M2MSecurity::set_bootstrap_flow(bool is_bootstrap_flow)
{
    _is_bootstrap_flow = is_bootstrap_flow;
}

bool M2MSecurity::is_bootstrap_flow() const
{
    return _is_bootstrap_flow;
}

void M2MSecurity::set_device_certificate(buffer_callback callback)
{
    _device_certificate = callback;
}

void M2MSecurity::set_server_certificate(buffer_callback callback)
{
    _server_certificate = callback;
}

void M2MSecurity::set_device_key(buffer_callback callback)
{
    _device_key = callback;
}

buffer_callback M2MSecurity::get_device_certificate() const
{
    return _device_certificate;
}

buffer_callback M2MSecurity::get_server_certificate() const
{
    return _server_certificate;
}

buffer_callback M2MSecurity::get_device_key() const
{
    return _device_key;
}
