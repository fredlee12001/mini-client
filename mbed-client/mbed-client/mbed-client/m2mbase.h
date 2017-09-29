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
#ifndef M2M_BASE_H
#define M2M_BASE_H

// Support for std args
#include <stdint.h>
#include "mbed-client/m2mconfig.h"
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mreportobserver.h"
#include "mbed-client/functionpointer.h"
#include "mbed-client/m2mstringbuffer.h"
#include "mbed-client/m2mstring.h"
#include "mbed-client/m2mcorememory.h"
#include "nsdl-c/sn_nsdl.h"
#include "sn_coap_header.h"
#include "nsdl-c/sn_nsdl_lib.h"

using namespace m2m;

//FORWARD DECLARATION
struct sn_coap_hdr_;
typedef sn_coap_hdr_ sn_coap_hdr_s;
struct nsdl_s;
struct sn_nsdl_addr_;
typedef sn_nsdl_addr_ sn_nsdl_addr_s;

typedef FP1<void, const char*> value_updated_callback;
typedef void(*value_updated_callback2) (const char* object_name);
typedef StringBuffer<25> path_buffer;
class M2MObservationHandler;
class M2MReportHandler;

class M2MObjectInstance;
class M2MObject;
class M2MResource;


/*! \file m2mbase.h
 *  \brief M2MBase.
 *  This class is the base class based on which all LWM2M object models
 *  can be created. This serves base class for Object, ObjectInstances and Resources.
 */
class M2MBase : public M2MReportObserver, public M2MCoreMemory {

public:

    /**
      * Enum to define the type of object.
      */
    typedef enum {
        Object = 0x0,
        Resource = 0x1,
        ObjectInstance = 0x2,
        ResourceInstance = 0x3
    } BaseType;

    /**
      * Enum to define observation level.
      */
    typedef enum {
        None                 = 0x0,
        R_Attribute          = 0x01,
        OI_Attribute         = 0x02,
        OIR_Attribute        = 0x03,
        O_Attribute          = 0x04,
        OR_Attribute         = 0x05,
        OOI_Attribute        = 0x06,
        OOIR_Attribute       = 0x07
    } Observation;


    /**
     * \brief Enum defining a resource type.
    */
    typedef enum {
        Static,
        Dynamic,
        Directory
    }Mode;

    /**
     * \brief Enum defining a resource data type.
    */
    typedef enum {
        STRING,
        INTEGER,
        FLOAT,
        BOOLEAN,
        OPAQUE,
        TIME,
        OBJLINK
    }DataType;

    /**
     * Enum defining an operation that can be
     * supported by a given resource.
    */
    typedef enum {
        NOT_ALLOWED                 = 0x00,
        GET_ALLOWED                 = 0x01,
        PUT_ALLOWED                 = 0x02,
        GET_PUT_ALLOWED             = 0x03,
        POST_ALLOWED                = 0x04,
        GET_POST_ALLOWED            = 0x05,
        PUT_POST_ALLOWED            = 0x06,
        GET_PUT_POST_ALLOWED        = 0x07,
        DELETE_ALLOWED              = 0x08,
        GET_DELETE_ALLOWED          = 0x09,
        PUT_DELETE_ALLOWED          = 0x0A,
        GET_PUT_DELETE_ALLOWED      = 0x0B,
        POST_DELETE_ALLOWED         = 0x0C,
        GET_POST_DELETE_ALLOWED     = 0x0D,
        PUT_POST_DELETE_ALLOWED     = 0x0E,
        GET_PUT_POST_DELETE_ALLOWED = 0x0F
    }Operation;

    enum MaxPathSize {
        MAX_NAME_SIZE = 64,

        // length of a max id (65535) when converted to ascii
        MAX_INSTANCE_SIZE = 5,

        MAX_PATH_SIZE = ((MAX_NAME_SIZE * 2) + (MAX_INSTANCE_SIZE * 2) + 3 + 1),
        MAX_PATH_SIZE_2 = ((MAX_NAME_SIZE * 2) + MAX_INSTANCE_SIZE + 2 + 1),
        MAX_PATH_SIZE_3 = (MAX_NAME_SIZE + (MAX_INSTANCE_SIZE * 2) + 2 + 1),
        MAX_PATH_SIZE_4 = (MAX_NAME_SIZE + MAX_INSTANCE_SIZE + 1 + 1)
    };

    typedef struct lwm2m_parameters {
        //add multiple_instances
        uint32_t            max_age; // todo: add flag
        sn_nsdl_dynamic_resource_parameters_s dynamic_resource_params;
        uint16_t            name_id;    // id of the object, obj-instance, resource or res-instance
        BaseType            base_type : 2;
        M2MBase::DataType   data_type : 3;
        bool                multiple_instance :1;
    } lwm2m_parameters_s;

protected:

    // Prevents the use of default constructor.
    M2MBase();

    // Prevents the use of assignment operator.
    M2MBase& operator=( const M2MBase& /*other*/ );

    // Prevents the use of copy constructor
    M2MBase( const M2MBase& /*other*/ );

    /**
     * \brief Constructor
     * \param base_type The base type of the object.
     * \param name Name of the object created.
     * \param mode Type of the resource.
     * \param resource_type Textual information of resource.
     * \param path Path of the object like 3/0/1
     * \param external_blockwise_store If true CoAP blocks are passed to application through callbacks
     *        otherwise handled in mbed-client-c.
     */
    M2MBase(M2MBase::BaseType base_type,
            uint16_t name_id,
            M2MBase::Mode mode,
            const char *resource_type,
            char *path,
            bool external_blockwise_store,
            bool multiple_instance,
            M2MBase::DataType type = M2MBase::OBJLINK);

public:

    /**
     * Destructor
     */
    virtual ~M2MBase();

    /**
     * \brief Sets the operation type for an object.
     * \param operation The operation to be set.
     */
    void set_operation(M2MBase::Operation operation);

    /**
     * \brief Sets the interface description of the object.
     * Note: the ownership of the string is not transferred, client is required to keep
     * the given char string to be valid during the object life cycle.
     * \param description The description to be set.
     */
    void set_interface_description(const char *description);

    /**
     * \brief Returns the interface description of the object.
     * \return The interface description of the object.
     */
    const char* interface_description() const;

    /**
     * \brief Sets the resource type of the object.
     * Note: the ownership of the string is not transferred, client is required to keep
     * the given char string to be valid during the object life cycle.
     * \param resource_type The resource type to be set.
     */
    void set_resource_type(const char *resource_type);

    /**
     * \brief Returns the resource type of the object.
     * \return The resource type of the object.
     */
    const char* resource_type() const;

    /**
     * \brief Sets the CoAP content type of the object.
     * \param content_type The content type to be set based on
     * CoAP specifications.
     */
    void set_coap_content_type(const uint16_t content_type);

    /**
     * \brief Sets the observable mode for the object.
     * \param observable A value for the observation.
     */
    void set_observable(bool observable);

    /**
     * \brief Adds the observation level for the object.
     * \param observation_level The level of observation.
     */
    virtual void add_observation_level(M2MBase::Observation observation_level);

    /**
     * \brief Removes the observation level for the object.
     * \param observation_level The level of observation.
     */
    virtual void remove_observation_level(M2MBase::Observation observation_level);

    /**
     * \brief Sets the object under observation.
     * \param observed The value for observation. When true, starts observing. When false, the ongoing observation is cancelled.
     * \param handler A handler object for sending
     * observation callbacks.
     */
    void set_under_observation(bool observed,
                                       M2MObservationHandler *handler);
    /**
     * \brief Returns the Observation Handler object.
     * \return M2MObservationHandler object.
    */
    virtual M2MObservationHandler* observation_handler() const = 0;

    /**
     * \brief Sets the observation handler
     * \param handler Observation handler
    */
    virtual void set_observation_handler(M2MObservationHandler *handler) = 0;

    /**
     * \brief Sets the observation token value.
     * \param token A pointer to the token of the resource.
     * \param length The length of the token pointer.
     */
    void set_observation_token(const uint8_t *token,
                                       const uint8_t length);

    /**
     * \brief Sets the max age for the resource value to be cached.
     * \param max_age The max age in seconds.
     */
    void set_max_age(const uint32_t max_age);

    /**
     * \brief Returns the object type.
     * \return The base type of the object.
     */
    M2MBase::BaseType base_type() const;

    /**
     * \brief Returns the operation type of the object.
     * \return The supported operation on the object.
     */
    M2MBase::Operation operation() const;

    /**
     * \brief Returns the object name in integer.
     * \return The name of the object in integer.
     */
    int32_t name_id() const;

    /**
     * \brief Returns the object's instance ID.
     * \returns The instance ID of the object.
     */
    uint16_t instance_id() const;

    /**
     * \brief Returns the path of the object.
     * \return The path of the object (eg. 3/0/1).
     */
    virtual void uri_path(path_buffer &buffer) const;

    /**
     * \brief Returns the CoAP content type of the object.
     * \return The CoAP content type of the object.
     */
    uint16_t coap_content_type() const;

    /**
     * \brief Returns the observation status of the object.
     * \return True if observable, else false.
     */
    bool is_observable() const;

    /**
     * \brief Returns the observation level of the object.
     * \return The observation level of the object.
     */
    M2MBase::Observation observation_level() const;

    /**
     * \brief Provides the observation token of the object.
     * \param value[OUT] A pointer to the value of the token.
     * \param value_length[OUT] The length of the token pointer.
     */
    void get_observation_token(uint8_t *&token, uint8_t &token_length) const;

    /**
     * \brief Returns the mode of the resource.
     * \return The mode of the resource.
     */
     Mode mode() const;

    /**
     * \brief Returns the observation number.
     * \return The observation number of the object.
     */
    uint16_t observation_number() const;

    /**
     * \brief Returns the max age for the resource value to be cached.
     * \return The maax age in seconds.
     */
    uint32_t max_age() const;

    /**
     * \brief Parses the received query for the notification
     * attribute.
     * \param query The query that needs to be parsed.
     * \return True if required attributes are present, else false.
     */
    virtual bool handle_observation_attribute(const char *query);

    /**
     * \brief Handles GET request for the registered objects.
     * \param nsdl An NSDL handler for the CoAP library.
     * \param received_coap_header The received CoAP message from the server.
     * \param observation_handler A handler object for sending
     * observation callbacks.
     * \return sn_coap_hdr_s The message that needs to be sent to server.
     */
    virtual sn_coap_hdr_s* handle_get_request(nsdl_s *nsdl,
                                              sn_coap_hdr_s *received_coap_header,
                                              M2MObservationHandler *observation_handler = NULL);
    /**
     * \brief Handles PUT request for the registered objects.
     * \param nsdl An NSDL handler for the CoAP library.
     * \param received_coap_header The received CoAP message from the server.
     * \param observation_handler A handler object for sending
     * observation callbacks.
     * \param execute_value_updated True executes the "value_updated" callback.
     * \return sn_coap_hdr_s The message that needs to be sent to server.
     */
    virtual sn_coap_hdr_s* handle_put_request(nsdl_s *nsdl,
                                              sn_coap_hdr_s *received_coap_header,
                                              M2MObservationHandler *observation_handler,
                                              bool &execute_value_updated);

    /**
     * \brief Handles GET request for the registered objects.
     * \param nsdl An NSDL handler for the CoAP library.
     * \param received_coap_header The received CoAP message from the server.
     * \param observation_handler A handler object for sending
     * observation callbacks.
     * \param execute_value_updated True executes the "value_updated" callback.
     * \return sn_coap_hdr_s  The message that needs to be sent to server.
     */
    virtual sn_coap_hdr_s* handle_post_request(nsdl_s *nsdl,
                                               sn_coap_hdr_s *received_coap_header,
                                               M2MObservationHandler *observation_handler,
                                               bool &execute_value_updated,
                                               sn_nsdl_addr_s *address = NULL);

    /**
     * \brief Sets whether this resource is published to server or not.
     * \param register_uri True sets the resource as part of registration message.
     */
    void set_register_uri(bool register_uri);

    /**
     * \brief Returns whether this resource is published to server or not.
     * \return True if the resource is a part of the registration message, else false.
     */
    bool register_uri() const;

    /**
     * @brief Returns whether this resource is under observation or not.
     * @return True if the resource is under observation, else false,
     */
    bool is_under_observation() const;

    /**
     * @brief Sets the function that is executed when this
     * object receives a PUT or POST command.
     * @param callback The function pointer that is called.
     * @return True, if callback could be set, false otherwise.
     */
    bool set_value_updated_function(value_updated_callback callback);

    /**
     * @brief Sets the function that is executed when this
     * object receives a PUT or POST command.
     * @param callback The function pointer that is called.
     * @return True, if callback could be set, false otherwise.
     */
    bool set_value_updated_function(value_updated_callback2 callback);

    /**
     * @brief Returns whether a callback function is set or not.
     * @return True if the callback function is set, else false.
     */
    bool is_value_updated_function_set() const;

    /**
     * @brief Calls the function that is set in the "set_value_updated_function".
     * @param name The name_id of the object.
     */
    void execute_value_updated(const char *name);

    /**
     * @brief Returns length of the object name.
     * @return Length of the object name.
     */
    size_t resource_name_length() const;

    /**
     * @brief Returns the resource information.
     * @return Resource information.
     */
    const sn_nsdl_dynamic_resource_parameters_s& get_nsdl_resource() const;

    sn_nsdl_dynamic_resource_parameters_s& get_nsdl_resource();

    /**
     * @brief Returns the resource structure.
     * @return Resource structure.
     */
    const M2MBase::lwm2m_parameters_s& get_lwm2m_parameters() const;

    /**
     * @brief Returns the notification message id.
     * @return Message id.
     */
    uint16_t get_notification_msgid() const;

    /**
     * @brief Sets the notification message id.
     * This is used to map RESET message.
     * @param msgid The message id.
     */
    void set_notification_msgid(uint16_t msgid);


protected: // from M2MReportObserver

    virtual void observation_to_be_sent(const m2m::Vector<uint16_t> &changed_instance_ids,
                                        uint16_t obs_number,
                                        bool send_object = false);

protected:

    /**
     * \brief Allocate and make a copy of given zero terminated string. This
     * is functionally equivalent with strdup().
     * \param source The source string to copy, may not be NULL.
    */
    static char* alloc_string_copy(const char* source);

    /**
     * \brief Allocate (size + 1) amount of memory, copy size bytes into
     * it and add zero termination.
     * \param source The source string to copy, may not be NULL.
     * \param size The size of memory to be reserved.
    */
    static uint8_t* alloc_string_copy(const uint8_t* source, uint32_t size);

    /**
     * \brief Allocate (size) amount of memory, copy size bytes into it.
     * \param source The source buffer to copy, may not be NULL.
     * \param size The size of memory to be reserved.
    */
    static uint8_t* alloc_copy(const uint8_t* source, uint32_t size);

    // validate string length to be [min_length..max_length]
    static bool validate_string_length(const char* str, size_t min_length, size_t max_length);

    /**
     * \brief Create Report Handler object.
     * \return M2MReportHandler object.
    */
    M2MReportHandler* create_report_handler();

    /**
     * \brief Returns the Report Handler object.
     * \return M2MReportHandler object.
    */
    M2MReportHandler* report_handler() const;

    static bool build_path(StringBuffer<MAX_PATH_SIZE_3> &buffer, uint16_t i1, uint16_t i2, uint16_t i3);

    static char* make_string(uint16_t src_int);

    static char* stringdup(const char* s);

    /**
     * \brief Delete the resource structures owned by this object. Note: this needs
     * to be called separately from each subclass' destructor as this method uses a
     * virtual method and the call needs to be done at same class which has the
     * implementation of the pure virtual method.
     */
    void free_resources();

private:

    static bool is_integer(const char *value);

private:
    lwm2m_parameters_s          _sn_resource;
    M2MReportHandler            *_report_handler; // TODO: can be broken down to smaller classes with inheritance.

friend class Test_M2MBase;
friend class Test_M2MObject;
friend class Test_M2MResource;
friend class Test_M2MResourceInstance;
};

#endif // M2M_BASE_H

