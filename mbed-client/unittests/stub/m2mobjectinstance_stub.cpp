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
#include "m2mobjectinstance_stub.h"

uint8_t m2mobjectinstance_stub::int_value;
bool m2mobjectinstance_stub::bool_value;
M2MResource* m2mobjectinstance_stub::resource;
M2MResource* m2mobjectinstance_stub::create_resource;

M2MResourceBase* m2mobjectinstance_stub::create_resource_instance;
sn_coap_hdr_s* m2mobjectinstance_stub::header;
M2MObservationHandler *m2mobjectinstance_stub::observe;

void m2mobjectinstance_stub::clear()
{
    int_value = 0;
    bool_value = false;
    resource = NULL;
    create_resource = NULL;
    create_resource_instance = NULL;
    header = NULL;
    observe = NULL;
}

M2MObjectInstance::M2MObjectInstance(M2MObject& parent,
                                     uint16_t name_id,
                                     const char *resource_type,
                                     char *path,
                                     bool external_blockwise_store)
: M2MBase(M2MBase::ObjectInstance,
          name_id,
          M2MBase::Dynamic,
#ifndef DISABLE_RESOURCE_TYPE
          resource_type,
#endif
          path,
          external_blockwise_store,
          false),
  _parent(parent),
  _resource_list(NULL)
{
}


M2MObjectInstance::~M2MObjectInstance()
{
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
    return m2mobjectinstance_stub::create_resource;
}

M2MResource* M2MObjectInstance::create_dynamic_resource(uint16_t resource_id,
                                                const char *resource_type,
                                                M2MResourceInstance::ResourceType type,
                                                bool observable,
                                                bool multiple_instance,
                                                bool external_blockwise_store)
{
    return m2mobjectinstance_stub::create_resource;
}

M2MResourceInstance* M2MObjectInstance::create_static_resource_instance(uint16_t resource_id,
                                                                        const char *resource_type,
                                                                        M2MResourceInstance::ResourceType type,
                                                                        const uint8_t *value,
                                                                        const uint8_t value_length,
                                                                        uint16_t instance_id,
                                                                        bool external_blockwise_store)
{
    return (M2MResourceInstance*)m2mobjectinstance_stub::create_resource_instance;
}


M2MResourceInstance* M2MObjectInstance::create_dynamic_resource_instance(uint16_t resource_id,
                                                                         const char *resource_type,
                                                                         M2MResourceInstance::ResourceType type,
                                                                         bool observable,
                                                                         uint16_t instance_id,
                                                                         bool external_blockwise_store)
{
    return (M2MResourceInstance*)m2mobjectinstance_stub::create_resource_instance;
}

void M2MObjectInstance::add_resource(M2MResource *resource)
{

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
    return m2mobjectinstance_stub::bool_value;
}

bool M2MObjectInstance::remove_resource_instance(uint16_t , uint16_t)
{
    return m2mobjectinstance_stub::bool_value;
}

M2MResource* M2MObjectInstance::resource(uint16_t resource_id) const
{
    return m2mobjectinstance_stub::resource;
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
    // XXX: a clone of the code from real implementation, which is actually much easier
    // to setup from the caller than the static number.

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
    return m2mobjectinstance_stub::int_value;
}

uint16_t M2MObjectInstance::resource_count(uint16_t ) const
{
    return m2mobjectinstance_stub::int_value;
}

M2MObservationHandler* M2MObjectInstance::observation_handler() const
{
    return m2mobjectinstance_stub::observe;
}

void M2MObjectInstance::set_observation_handler(M2MObservationHandler *handler)
{
    m2mobjectinstance_stub::observe = handler;
}

void M2MObjectInstance::add_observation_level(M2MBase::Observation)
{
}

void M2MObjectInstance::remove_observation_level(M2MBase::Observation)
{
}

sn_coap_hdr_s* M2MObjectInstance::handle_get_request(nsdl_s *,
                                  sn_coap_hdr_s *,
                                  M2MObservationHandler *)
{
    return m2mobjectinstance_stub::header;
}

sn_coap_hdr_s* M2MObjectInstance::handle_put_request(nsdl_s *,
                                  sn_coap_hdr_s *,
                                  M2MObservationHandler *,
                                  bool &)
{
    return m2mobjectinstance_stub::header;
}

sn_coap_hdr_s* M2MObjectInstance::handle_post_request(nsdl_s *,
                                   sn_coap_hdr_s *,
                                   M2MObservationHandler *,
                                   bool &execute,
                                   sn_nsdl_addr_s *)
{
    execute = m2mobjectinstance_stub::bool_value;
    return m2mobjectinstance_stub::header;
}

void M2MObjectInstance::notification_update(M2MBase::Observation)
{
}
