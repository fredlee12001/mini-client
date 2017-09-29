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

#include "mbed-cloud-client/SimpleM2MResource.h"
#include "mbed-trace/mbed_trace.h"
#if 0
#include <ctype.h>

#include<stdio.h>

#define TRACE_GROUP "SMCR"


SimpleM2MResourceBase::SimpleM2MResourceBase()
: _client(NULL), _route(NULL)
{
    tr_debug("SimpleM2MResourceBase::SimpleM2MResourceBase()");
}

SimpleM2MResourceBase::SimpleM2MResourceBase(MbedCloudClient* client, const char *route)
: _client(client),_route(route)
{
    tr_debug("SimpleM2MResourceBase::SimpleM2MResourceBase(), resource name %s\r\n", _route);
}

SimpleM2MResourceBase::~SimpleM2MResourceBase()
{
}

bool SimpleM2MResourceBase::define_resource_internal(const char *v, M2MBase::Operation opr, bool observable)
{
    tr_debug("SimpleM2MResourceBase::define_resource_internal(), resource name %s!\r\n", _route);

    vector<const char*> segments = parse_route(_route);
    if (segments.size() != 3) {
        tr_debug("[SimpleM2MResourceBase] [ERROR] define_resource_internal(), Route needs to have three segments, split by '/' (%s)\r\n", _route);
        return false;
    }

    // segments[1] should be one digit and numeric
    if (!isdigit(segments.at(1)[0])) {
        tr_debug("[SimpleM2MResourceBase] [ERROR] define_resource_internal(), second route segment should be numeric, but was not (%s)\r\n", _route);
        return false;
    }

    int inst_id = atoi(segments.at(1));

    // Check if object exists
    M2MObject* obj;
    m2m::Map<M2MObject*>::iterator obj_it = _client->_objects.find(segments[0]) ;
    if(obj_it != _client->_objects.end()) {
      tr_debug("Found object... %s\r\n", segments.at(0));
      obj = *obj_it;
    } else {
        tr_debug("Create new object... %s\r\n", segments.at(0));
        obj = M2MInterfaceFactory::create_object(segments.at(0));
        _client->_objects[segments.at(0)] = obj;
    }

    // Check if object instance exists
    M2MObjectInstance* inst = obj->object_instance(inst_id);
    if(!inst) {
        tr_debug("Create new object instance... %s\r\n", segments.at(1));
        inst = obj->create_object_instance(inst_id);
    }

    // @todo check if the resource exists yet
    M2MResource* res = inst->resource(segments.at(2));
    if(!res) {
        res = inst->create_dynamic_resource(segments.at(2), "",
            M2MResourceInstance::STRING, observable);
        if(!res) {
            return false;
        }
        res->set_operation(opr);
        res->set_value((uint8_t*)v, strlen(v));

        _client->_resources[_route] = res;
        _client->register_update_callback(_route, this);
    }

    return true;
}

vector<const char*> SimpleM2MResourceBase::parse_route(const char* route)
{
    const char *s(route);
    vector<const char*> v;
#if 0
    std::size_t found = s.find_first_of("/");

    while (found!=std::string::npos) {
        v.push_back(s.substr(0,found));
        s = s.substr(found+1);
        found=s.find_first_of("/");
        if(found == std::string::npos) {
            v.push_back(s);
        }
    }
#endif
    return v;
}

const char *SimpleM2MResourceBase::get() const
{
    tr_debug("SimpleM2MResourceBase::get() resource (%s)", _route);
    if (!_client->_resources.count(_route)) {
        tr_debug("[SimpleM2MResourceBase] [ERROR] No such route (%s)\r\n", _route);
        return NULL;
    }

    // otherwise ask mbed Client...
    uint8_t* buffIn = NULL;
    uint32_t sizeIn;
    _client->_resources[_route]->get_value(buffIn, sizeIn);

    tr_debug("SimpleM2MResourceBase::get() resource value (%s)", (const char*)buffIn);
    return (const char*)buffIn;
}

bool SimpleM2MResourceBase::set(const char *v)
{
    // Potentially set() happens in InterruptContext. That's not good.
    tr_debug("SimpleM2MResourceBase::set() resource (%s)", _route);
    if (!_client->_resources.count(_route)) {
        tr_debug("[SimpleM2MResourceBase] [ERROR] No such route (%s)\r\n", _route);
        return false;
    }

    if (!v) {
        _client->_resources[_route]->clear_value();
    }
    else {
        _client->_resources[_route]->set_value((uint8_t*)v, strlen(v));
    }

    return true;
}

bool SimpleM2MResourceBase::set(const int& v)
{
    char buffer[20];
    sprintf(buffer,"%d",v);

    return set(buffer);
}

bool SimpleM2MResourceBase::set_post_function(void(*fn)(void*))
{
    //TODO: Check the resource exists with right operation being set or append the operation into it.
    M2MResource *resource = get_resource();
    if(!resource) {
        return false;
    }
    M2MBase::Operation op = resource->operation();
    op = (M2MBase::Operation)(op | M2MBase::POST_ALLOWED);
    resource->set_operation(op);

    _client->_resources[_route]->set_execute_function(execute_callback_2(fn));
    return true;
}

bool SimpleM2MResourceBase::set_post_function(execute_callback fn)
{
    //TODO: Check the resource exists with right operation being set or append the operation into it.
    M2MResource *resource = get_resource();
    if(!resource) {
        return false;
    }
    M2MBase::Operation op = resource->operation();
    op = (M2MBase::Operation)(op | M2MBase::POST_ALLOWED);
    resource->set_operation(op);

    // No clue why this is not working?! It works with class member, but not with static function...
    _client->_resources[_route]->set_execute_function(fn);
    return true;
}

M2MResource* SimpleM2MResourceBase::get_resource()
{
    if (!_client->_resources.count(_route)) {
        tr_debug("[SimpleM2MResourceBase] [ERROR] No such route (%s)\r\n", _route);
        return NULL;
    }
    return _client->_resources[_route];
}

SimpleM2MResourceString::SimpleM2MResourceString(MbedCloudClient* client,
                               const char* route,
                               const char* v,
                               M2MBase::Operation opr,
                               bool observable,
                               FP1<void, const char*> on_update)
: SimpleM2MResourceBase(client,route),_on_update(on_update)
{
    tr_debug("SimpleM2MResourceString::SimpleM2MResourceString() creating (%s)\r\n", route);
    define_resource_internal(v, opr, observable);
}

SimpleM2MResourceString::SimpleM2MResourceString(MbedCloudClient* client,
                               const char* route,
                               const char* v,
                               M2MBase::Operation opr,
                               bool observable,
                               void(*on_update)(const char*))

: SimpleM2MResourceBase(client,route)
{
    tr_debug("SimpleM2MResourceString::SimpleM2MResourceString() overloaded creating (%s)\r\n", route);
    FP1<void, const char*> fp;
    fp.attach(on_update);
    _on_update = fp;
    define_resource_internal(v, opr, observable);
}

SimpleM2MResourceString::~SimpleM2MResourceString()
{
}

const char* SimpleM2MResourceString::operator=(const char* new_value)
{
    tr_debug("SimpleM2MResourceString::operator=()");
    set(new_value);
    return new_value;
}

SimpleM2MResourceString::operator const char*() const
{
    tr_debug("SimpleM2MResourceString::operator string()");
    return get();
}

void SimpleM2MResourceString::update()
{
    _on_update(get());
}

SimpleM2MResourceInt::SimpleM2MResourceInt(MbedCloudClient* client,
                         const char* route,
                         int v,
                         M2MBase::Operation opr,
                         bool observable,
                         FP1<void, int> on_update)
: SimpleM2MResourceBase(client,route),_on_update(on_update)
{
    tr_debug("SimpleM2MResourceInt::SimpleM2MResourceInt() creating (%s)\r\n", route);
    char buffer[20];
    sprintf(buffer,"%d",v);
    define_resource_internal(buffer, opr, observable);
}

SimpleM2MResourceInt::SimpleM2MResourceInt(MbedCloudClient* client,
                         const char* route,
                         int v,
                         M2MBase::Operation opr,
                         bool observable,
                         void(*on_update)(int))
: SimpleM2MResourceBase(client,route)
{
    tr_debug("SimpleM2MResourceInt::SimpleM2MResourceInt() overloaded creating (%s)\r\n", route);
    FP1<void, int> fp;
    fp.attach(on_update);
    _on_update = fp;
    char buffer[20];
    sprintf(buffer,"%d",v);
    define_resource_internal(buffer, opr, observable);
}

SimpleM2MResourceInt::~SimpleM2MResourceInt()
{
}

int SimpleM2MResourceInt::operator=(int new_value)
{
    set(new_value);
    return new_value;
}

SimpleM2MResourceInt::operator int() const
{
    const char *v = get();
    if (!v) return 0;

    return atoi(v);
}

void SimpleM2MResourceInt::update()
{
    const char *v = get();
    if (!v) {
        _on_update(0);
    } else {
        _on_update(atoi(v));
    }
}
#endif
