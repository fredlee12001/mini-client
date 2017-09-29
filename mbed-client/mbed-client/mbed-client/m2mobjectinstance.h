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
#ifndef M2M_OBJECT_INSTANCE_H
#define M2M_OBJECT_INSTANCE_H

#include "mbed-client/m2mvector.h"
#include "mbed-client/m2mresource.h"

//FORWARD DECLARATION
typedef Vector<M2MResource *> M2MResourceList;
typedef Vector<M2MResourceInstance *> M2MResourceInstanceList;


class M2MObject;

/*! \file m2mobjectinstance.h
 *  \brief M2MObjectInstance.
 *  This class is the instance class for mbed Client Objects. All defined
 *  LWM2M object models can be created based on it. This class also holds all resource
 *  instances associated with the given object.
 */

class M2MObjectInstance : public M2MBase
{

friend class M2MObject;

private: // Constructor and destructor are private which means
         // that these objects can be created or
         // deleted only through function provided by M2MObject.

    /**
     * \brief Constructor
     * \param name Name of the object
     */
    M2MObjectInstance(M2MObject& parent,
                      uint16_t name_id,
                      char *path,
                      bool external_blockwise_store = false);

    // Prevents the use of default constructor.
    M2MObjectInstance();

    // Prevents the use of assignment operator.
    M2MObjectInstance& operator=( const M2MObjectInstance& /*other*/ );

    // Prevents the use of copy constructor.
    M2MObjectInstance( const M2MObjectInstance& /*other*/ );

    /**
     * Destructor
     */
    virtual ~M2MObjectInstance();

public:

    /**
     * \brief Creates a static resource for a given mbed Client Inteface object. With this, the
     * client can respond to server's GET methods with the provided value.
     * \param resource_name The name of the resource.
     * \param resource_type The type of the resource.
     * \param value A pointer to the value of the resource.
     * \param value_length The length of the value in the pointer.
     * \param multiple_instance A resource can have
     *        multiple instances, default is false.
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     * \return M2MResource The resource for managing other client operations.
     */
    M2MResource* create_static_resource(uint16_t resource_id,
                                        const char *resource_type,
                                        M2MResourceInstance::ResourceType type,
                                        const uint8_t *value,
                                        const uint8_t value_length,
                                        bool multiple_instance = false,
                                        bool external_blockwise_store = false);

    /**
     * \brief Creates a dynamic resource for a given mbed Client Inteface object. With this,
     * the client can respond to different queries from the server (GET,PUT etc).
     * This type of resource is also observable and carries callbacks.
     * \param resource_name The name of the resource.
     * \param resource_type The type of the resource.
     * \param observable Indicates whether the resource is observable or not.
     * \param multiple_instance The resource can have
     *        multiple instances, default is false.
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     * \return M2MResource The resource for managing other client operations.
     */
    M2MResource* create_dynamic_resource(uint16_t resource_id,
                                         const char *resource_type,
                                         M2MResourceInstance::ResourceType type,
                                         bool observable,
                                         bool multiple_instance = false,
                                         bool external_blockwise_store = false);

    /**
     * \brief Creates a static resource instance for a given mbed Client Inteface object. With this,
     * the client can respond to server's GET methods with the provided value.
     * \param resource_name The name of the resource.
     * \param resource_type The type of the resource.
     * \param value A pointer to the value of the resource.
     * \param value_length The length of the value in pointer.
     * \param instance_id The instance ID of the resource.
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     * \return M2MResourceInstance The resource instance for managing other client operations.
     */
    M2MResourceInstance* create_static_resource_instance(uint16_t resource_id,
                                                         const char *resource_type,
                                                         M2MResourceInstance::ResourceType type,
                                                         const uint8_t *value,
                                                         const uint8_t value_length,
                                                         uint16_t instance_id,
                                                         bool external_blockwise_store = false);

    /**
     * \brief Creates a dynamic resource instance for a given mbed Client Inteface object. With this,
     * the client can respond to different queries from the server (GET,PUT etc).
     * This type of resource is also observable and carries callbacks.
     * \param resource_name The name of the resource.
     * \param resource_type The type of the resource.
     * \param observable Indicates whether the resource is observable or not.
     * \param instance_id The instance ID of the resource.
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     * \return M2MResourceInstance The resource instance for managing other client operations.
     */
    M2MResourceInstance* create_dynamic_resource_instance(uint16_t resource_id,
                                                          const char *resource_type,
                                                          M2MResourceInstance::ResourceType type,
                                                          bool observable,
                                                          uint16_t instance_id,
                                                          bool external_blockwise_store = false);

    /**
     * \brief Removes the resource with the given name.
     * \param name The name of the resource to be removed.
     * \return True if removed, else false.
     */
    bool remove_resource(uint16_t name_id);

    /**
     * \brief Removes the resource instance with the given name.
     * \param resource_name The name of the resource instance to be removed.
     * \param instance_id The instance ID of the instance.
     * \return True if removed, else false.
     */
    bool remove_resource_instance(uint16_t resource_id,
                                  uint16_t instance_id);

    /**
     * \brief Returns the resource with the given id.
     * \param resource_id The id of the requested resource.
     * \return Resource reference if found, else NULL.
     */
    M2MResource* resource(uint16_t resource_id) const;

    /**
     * \brief Returns a list of M2MResource objects.
     * \return A linked list of M2MResource objects.
     */
    const M2MResource* resources() const;
    M2MResource* resources();

    /**
     * \brief Returns the total number of resources and instances with the object.
     * Note: to get only the resources, one needs to use total_resource_count() instead.
     * \return Total number of the resources.
     */
    uint16_t resource_count() const; // XXX: the uint16_t is not necessarily wide enough!

    /**
     * \brief Returns the total number of resources with the object,
     * EXCLUDING the resource instances
     * \return Total number of the resources.
     */
    uint16_t total_resource_count() const;

    /**
     * \brief Returns the total number of single resource instances.
     * \param resource The name of the resource.
     * \return Total number of the resources.
     */
    uint16_t resource_count(uint16_t resource_id) const;

    /**
     * \brief Adds the observation level for the object.
     * \param observation_level The level of observation.
     */
    virtual void add_observation_level(M2MBase::Observation observation_level);

    /**
     * \brief Removes the observation level from the object.
     * \param observation_level The level of observation.
     */
    virtual void remove_observation_level(M2MBase::Observation observation_level);

    /**
     * \brief Returns the Observation Handler object.
     * \return M2MObservationHandler object.
    */
    virtual M2MObservationHandler* observation_handler() const;

    /**
     * \brief Sets the observation handler
     * \param handler Observation handler
    */
    virtual void set_observation_handler(M2MObservationHandler *handler);

    /**
     * \brief Handles GET request for the registered objects.
     * \param nsdl The NSDL handler for the CoAP library.
     * \param received_coap_header The CoAP message received from the server.
     * \param observation_handler The handler object for sending
     * observation callbacks.
     * return sn_coap_hdr_s The message that needs to be sent to the server.
     */
    virtual sn_coap_hdr_s* handle_get_request(nsdl_s *nsdl,
                                              sn_coap_hdr_s *received_coap_header,
                                              M2MObservationHandler *observation_handler = NULL);
    /**
     * \brief Handles PUT request for the registered objects.
     * \param nsdl The NSDL handler for the CoAP library.
     * \param received_coap_header The CoAP message received from the server.
     * \param observation_handler The handler object for sending
     * observation callbacks.
     * \param execute_value_updated True will execute the "value_updated" callback.
     * \return sn_coap_hdr_s The message that needs to be sent to server.
     */
    virtual sn_coap_hdr_s* handle_put_request(nsdl_s *nsdl,
                                              sn_coap_hdr_s *received_coap_header,
                                              M2MObservationHandler *observation_handler,
                                              bool &execute_value_updated);

    /**
     * \brief Handles POST request for the registered objects.
     * \param nsdl The NSDL handler for the CoAP library.
     * \param received_coap_header The CoAP message received from the server.
     * \param observation_handler The handler object for sending
     * observation callbacks.
     * \param execute_value_updated True will execute the "value_updated" callback.
     * \return sn_coap_hdr_s The message that needs to be sent to server.
     */
    virtual sn_coap_hdr_s* handle_post_request(nsdl_s *nsdl,
                                               sn_coap_hdr_s *received_coap_header,
                                               M2MObservationHandler *observation_handler,
                                               bool &execute_value_updated,
                                               sn_nsdl_addr_s *address = NULL);

    inline M2MObject& get_parent_object() const;

    // callback used from M2MResource/M2MResourceInstance
    void notification_update(M2MBase::Observation observation_level);

    // These could be in M2MResourceBase as M2MResource* versions, but that would remove
    // some type safety and increase need for casts.
    inline const M2MObjectInstance* get_next() const;
    inline M2MObjectInstance* get_next();
    inline void set_next(M2MObjectInstance*next);

    /**
     * \brief Returns the path of the object.
     * \return The path of the object (eg. 3/0/1).
     */
    virtual void uri_path(path_buffer &buffer) const;

private:

    /**
     * \brief Helper for adding the resource to end of the resource list.
     * Note: the item must not be already added to this or any other object instance.
     * \param resource to add
     */
    void add_resource(M2MResource *resource);

    /**
     * \brief Utility function to map M2MResourceInstance ResourceType
     * to M2MBase::DataType.
     * \param resource_type M2MResourceInstance::ResourceType.
     * \return M2MBase::DataType.
     */
    M2MBase::DataType convert_resource_type(M2MResourceInstance::ResourceType);

private:

    M2MObject                   &_parent;

    // linked list of object instances belonging to the _parent
    M2MObjectInstance           *_next;

    // one way linked list of resources owned by this object instance
    M2MResource                 *_resource_list;

    friend class Test_M2MObjectInstance;
    friend class Test_M2MObject;
    friend class Test_M2MDevice;
    friend class Test_M2MSecurity;
    friend class Test_M2MServer;
    friend class Test_M2MNsdlInterface;
    friend class Test_M2MFirmware;
    friend class Test_M2MTLVSerializer;
    friend class Test_M2MTLVDeserializer;
    friend class Test_M2MBase;
    friend class Test_M2MResource;
    friend class Test_M2MResourceInstance;
};

inline M2MObject& M2MObjectInstance::get_parent_object() const
{
    return _parent;
}

inline const M2MObjectInstance* M2MObjectInstance::get_next() const
{
    return _next;
}

inline M2MObjectInstance* M2MObjectInstance::get_next()
{
    return _next;
}

inline void M2MObjectInstance::set_next(M2MObjectInstance* next)
{
    _next = next;
}

#endif // M2M_OBJECT_INSTANCE_H
