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
#ifndef M2M_RESOURCE_INSTANCE_H
#define M2M_RESOURCE_INSTANCE_H

#include "mbed-client/m2mresourcebase.h"


/*! \file m2mresourceinstance.h
 *  \brief M2MResourceInstance.
 *  This class is the base class for mbed Client Resources. All defined
 *  LWM2M resource models can be created based on it.
 */
class M2MBlockMessage;


class M2MResource;

class M2MResourceInstance : public M2MResourceBase {

friend class M2MObjectInstance;
friend class M2MResource;

private: // Constructor and destructor are private
         // which means that these objects can be created or
         // deleted only through a function provided by the M2MObjectInstance.

    /**
     * \brief Memory allocations for the parameters.
     * \param value The value pointer of the object.
     * \param value_length The length of the value pointer.
     * \return false if any memory allocation fails else true.
     */
    bool allocate_resources(const uint8_t *value,
                            const uint8_t value_length);

    /**
     * \brief A constructor for creating a resource.
     * \param resource_name The name of the resource.
     * \param resource_type The type of the resource.
     * \param type The resource data type of the object.
     * \param object_name Object name where resource exists.
     * \param path Path of the object like 3/0/1
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     */
    M2MResourceInstance(M2MResource &parent,
                        uint16_t name_id,
                        M2MBase::Mode mode,
                        const char *resource_type,
                        M2MBase::DataType type,
                        char* path,
                        bool external_blockwise_store,
                        bool multiple_instance);

    // Prevents the use of default constructor.
    M2MResourceInstance();

    // Prevents the use of assignment operator.
    M2MResourceInstance& operator=( const M2MResourceInstance& /*other*/ );

    // Prevents the use of copy constructor
    M2MResourceInstance( const M2MResourceInstance& /*other*/ );

    /**
     * Destructor
     */
    virtual ~M2MResourceInstance();

public:

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
     * \brief Parses the received query for a notification
     * attribute.
     * \return True if required attributes are present, else false.
     */
    virtual bool handle_observation_attribute(const char *query);


    /**
     * \brief Returns the instance ID of the object where the resource exists.
     * \return Object instance ID.
    */
    virtual uint16_t object_instance_id() const;

    /**
     * \brief Returns the name of the object where the resource exists.
     * \return Object name.
    */
    virtual uint16_t object_id() const;

    /**
     * \brief Get reference to the resource owning this resource instance.
     * \return parent resource
     */
    virtual M2MResource& get_parent_resource() const;

    /**
     * \brief Returns the path of the object.
     * \return The path of the object (eg. 3/0/1).
     */
    virtual void uri_path(path_buffer &buffer) const;


    inline const M2MResourceInstance* get_next() const;
    inline M2MResourceInstance* get_next();
    inline void set_next(M2MResourceInstance *next);

private:

    // Parent resource which owns this resource instance
    M2MResource &_parent_resource;

    // linked list of resource instances belonging to the _parent_resource
    M2MResourceInstance *_next;

    friend class Test_M2MResourceInstance;
    friend class Test_M2MResource;
    friend class Test_M2MObjectInstance;
    friend class Test_M2MObject;
    friend class Test_M2MDevice;
    friend class Test_M2MSecurity;
    friend class Test_M2MServer;
    friend class Test_M2MNsdlInterface;
    friend class Test_M2MFirmware;
    friend class Test_M2MTLVSerializer;
    friend class Test_M2MTLVDeserializer;
};

inline const M2MResourceInstance* M2MResourceInstance::get_next() const
{
    return _next;
}

inline M2MResourceInstance* M2MResourceInstance::get_next()
{
    return _next;
}

inline void M2MResourceInstance::set_next(M2MResourceInstance* next)
{
    _next = next;
}


#endif // M2M_RESOURCE_INSTANCE_H
