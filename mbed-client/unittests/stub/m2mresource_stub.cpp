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
#include "m2mresource_stub.h"

#include "m2mobject.h"

uint32_t m2mresource_stub::int_value;
uint8_t* m2mresource_stub::delayed_token;
uint8_t m2mresource_stub::delayed_token_len;
bool m2mresource_stub::bool_value;

// The statically initialized list must be big enough to cater
// for all the tests, or the utest framework will complain for memory leak.
M2MResourceInstance *m2mresource_stub::list;

M2MResourceInstance *m2mresource_stub::instance;
M2MObjectInstance *m2mresource_stub::object_instance;
sn_coap_hdr_s *m2mresource_stub::header;
M2MObservationHandler *m2mresource_stub::observe;

void m2mresource_stub::clear()
{
    int_value = 0;
    delayed_token = NULL;
    delayed_token_len = 0;
    bool_value = false;
    list = NULL;
    instance = NULL;
    object_instance = NULL;
    observe = NULL;
}

M2MResource::M2MResource(M2MObjectInstance &parent,
                         uint16_t name_id,
                         M2MBase::Mode resource_mode,
                         const char *resource_type,
                         M2MBase::DataType type,
                         const uint8_t *value,
                         const uint8_t value_length,
                         char *path,
                         bool multiple_instance,
                         bool external_blockwise_store)
: M2MResourceBase(M2MBase::Resource, name_id, resource_mode, resource_type, type, value, value_length,
                      path, external_blockwise_store, multiple_instance),
  _parent(parent),
  _next(NULL),
  _resource_instance_list(NULL)
#ifndef DISABLE_DELAYED_RESPONSE
  ,_delayed_token(NULL),
  _delayed_token_len(0),
  _delayed_response(false)
#endif
{
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
                      path,
                      external_blockwise_store,multiple_instance),
  _parent(parent),
  _next(NULL),
  _resource_instance_list(NULL)
#ifndef DISABLE_DELAYED_RESPONSE
  ,_delayed_token(NULL),
  _delayed_token_len(0),
  _delayed_response(false)
#endif
{
}

M2MResource::~M2MResource()
{
    free_resources();
}

bool M2MResource::supports_multiple_instances() const
{
    return m2mresource_stub::bool_value;
}

void M2MResource::get_delayed_token(unsigned char *&token, unsigned char &token_len)
{
    token_len = 0;
    if(token) {
        free(token);
        token = NULL;
    }
    token = (uint8_t *)malloc(m2mresource_stub::delayed_token_len);
    if(token) {
        token_len = m2mresource_stub::delayed_token_len;
        memcpy((uint8_t *)token, (uint8_t *)m2mresource_stub::delayed_token, token_len);
    }
}

bool M2MResource::remove_resource_instance(uint16_t inst_id)
{
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
    return m2mresource_stub::instance;
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
    return m2mresource_stub::int_value;
}

bool M2MResource::handle_observation_attribute(const char *query)
{
    return m2mresource_stub::bool_value;
}

M2MObservationHandler* M2MResource::observation_handler() const
{
    return m2mresource_stub::observe;
}

void M2MResource::set_observation_handler(M2MObservationHandler *handler)
{
    m2mresource_stub::observe = handler;
}

void M2MResource::add_resource_instance(M2MResourceInstance *res)
{
    // XXX: a copy paste of real implementation, but no point to implement it twice
    assert(res);
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

void M2MResource::add_observation_level(M2MBase::Observation)
{
}

void M2MResource::remove_observation_level(M2MBase::Observation)
{
}

sn_coap_hdr_s* M2MResource::handle_get_request(nsdl_s *,
                                               sn_coap_hdr_s *,
                                               M2MObservationHandler *)
{
    return m2mresource_stub::header;
}

sn_coap_hdr_s* M2MResource::handle_put_request(nsdl_s *,
                                               sn_coap_hdr_s *,
                                               M2MObservationHandler *,
                                               bool &)
{
    return m2mresource_stub::header;
}

sn_coap_hdr_s* M2MResource::handle_post_request(nsdl_s *,
                                               sn_coap_hdr_s *,
                                               M2MObservationHandler *,
                                               bool &, sn_nsdl_addr_s *)
{
    return m2mresource_stub::header;
}

M2MObjectInstance& M2MResource::get_parent_object_instance() const
{
    return _parent;
}

uint16_t M2MResource::object_instance_id() const
{
    return 0; // XXX

/* XXX real implementation:
    const M2MObjectInstance& parent_object_instance = get_parent_object_instance();
    return parent_object_instance.instance_id();
*/    
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
