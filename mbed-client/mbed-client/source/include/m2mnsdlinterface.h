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
#ifndef M2MNSDLINTERFACE_H
#define M2MNSDLINTERFACE_H

#include "mbed-client/m2mvector.h"
#include "mbed-client/m2mconfig.h"
#include "mbed-client/m2minterface.h"
#include "mbed-client/m2mtimerobserver.h"
#include "mbed-client/m2mobservationhandler.h"
#include "mbed-client/m2mtimer.h"
#include "mbed-client/m2mbase.h"
#include "mbed-client/m2mserver.h"
#include "mbed-client/m2msecurity.h"
#include "mbed-client/m2mconstants.h"
#include "include/nsdllinker.h"
#include "mbed-client/m2mcorememory.h"

//FORWARD DECLARARTION
class M2MSecurity;
class M2MObject;
class M2MObjectInstance;
class M2MResource;
class M2MResourceInstance;
class M2MNsdlObserver;
class M2MServer;
class M2MConnectionHandler;

typedef Vector<M2MObject *> M2MObjectList;

struct iterator_state_s {
    M2MObject           *object;
    M2MObjectInstance   *objectinstance;
    M2MResource         *resource;
    M2MResourceInstance *resourceinstance;
};

/**
 * @brief M2MNsdlInterface
 * Class which interacts between mbed Client C++ Library and mbed-client-c library.
 */
class M2MNsdlInterface : public M2MTimerObserver,
                         public M2MObservationHandler,
                         public M2MCoreMemory
{
private:
    // Prevents the use of assignment operator by accident.
    M2MNsdlInterface& operator=( const M2MNsdlInterface& /*other*/ );

    // Prevents the use of copy constructor by accident
    M2MNsdlInterface( const M2MNsdlInterface& /*other*/ );

public:
    /**
    * @brief Constructor
    * @param observer, Observer to pass the event callbacks from nsdl library.
    */
    M2MNsdlInterface(M2MNsdlObserver &observer, M2MConnectionHandler &connection_handler);

    /**
     * @brief Destructor
     */
    virtual ~M2MNsdlInterface();

    /**
     * @brief Creates endpoint object for the nsdl stack.
     * @param endpoint_name, Endpoint name of the client.
     * @param endpoint_type, Endpoint type of the client.
     * @param life_time, Life time of the client in seconds
     * @param domain, Domain of the client.
     * @param mode, Binding mode of the client, default is UDP
     * @param context_address, Context address default is empty.
     * @return true if endpoint is created else false.
    */
    bool create_endpoint(const char *endpoint_name,
                         const char *endpoint_type,
                         const int32_t life_time,
                         const char *domain,
                         const uint8_t mode,
                         const char *context_address);


    /**
     * @brief Updates endpoint name.
    */
    void update_endpoint(const char *name);

    /**
     * @brief Updates domain.
    */
    void update_domain(const char *domain);

    /**
     * @brief Creates the NSDL structure for the registered objectlist.
     * @param object_list, List of objects to be registered.
     * @return true if structure created successfully else false.
    */
    bool create_nsdl_list_structure(const M2MObject *object_list);

    /**
     * @brief Removed the NSDL resource for the given resource.
     * @param base, Resource to be removed.
     * @return true if removed successfully else false.
    */
    bool remove_nsdl_resource(M2MBase *base);

    /**
     * @brief Creates the bootstrap object.
     * @param address Bootstrap address.
     * @return true if created and sent successfully else false.
    */
    bool create_bootstrap_resource(sn_nsdl_addr_s *address);

    /**
     * @brief Sets the register message to the server.
     * @param address M2MServer address.
     * @param address_length M2MServer address length.
     * @param port M2MServer port.
     * @param address_type IP Address type.
    */
    void set_server_address(uint8_t* address,
                            uint8_t address_length,
                            const uint16_t port,
                            sn_nsdl_addr_type_e address_type);
    /**
     * @brief Sends the register message to the server.
     * @return  true if register sent successfully else false.
    */
    bool send_register_message();

    /**
     * @brief Sends the update registration message to the server.
     * @param lifetime, Updated lifetime value in seconds.
     * @param clear_queue, Empties resending queue, by default it doesn't empties queue.
     * @return  true if sent successfully else false.
     *
    */
    bool send_update_registration(const uint32_t lifetime = 0, bool clear_queue = false);

    /**
     * @brief Sends unregister message to the server.
     * @return  true if unregister sent successfully else false.
    */
    bool send_unregister_message();

    /**
    * @brief Callback from nsdl library to inform the data is ready
    * to be sent to server.
    * @param nsdl_handle, Handler for the nsdl structure for this endpoint
    * @param protocol, Protocol format of the data
    * @param data, Data to be sent.
    * @param data_len, Size of the data to be sent
    * @param address, server address where data has to be sent.
    * @return 1 if successful else 0.
    */
    uint8_t send_to_server_callback(struct nsdl_s * nsdl_handle,
                                    sn_nsdl_capab_e protocol,
                                    uint8_t *data,
                                    uint16_t data_len,
                                    sn_nsdl_addr_s *address);

    /**
    * @brief Callback from nsdl library to inform the data which is
    * received from server for the client has been converted to coap message.
    * @param nsdl_handle, Handler for the nsdl structure for this endpoint
    * @param coap_header, Coap message formed from data.
    * @param address, Server address from where the data is received.
    * @return 1 if successful else 0.
    */
    uint8_t received_from_server_callback(struct nsdl_s * nsdl_handle,
                                          sn_coap_hdr_s *coap_header,
                                          sn_nsdl_addr_s *address);

    /**
    * @brief Callback from nsdl library to inform the data which is
    * received from server for the resources has been converted to coap message.
    * @param nsdl_handle, Handler for the nsdl resource structure for this endpoint..
    * @param coap_header, Coap message formed from data.
    * @param address, Server address from where the data is received.
    * @param nsdl_capab, Protocol for the message, currently only coap is supported.
    * @return 1 if successful else 0.
    */
    uint8_t resource_callback(struct nsdl_s *nsdl_handle, sn_coap_hdr_s *coap,
                               sn_nsdl_addr_s *address,
                               sn_nsdl_capab_e nsdl_capab);

    /**
     * @brief Callback when there is data received from server and needs to be processed.
     * @param data, data received from server.
     * @param data_size, data size received from server.
     * @param addres, address structure of the server.
     * @return true if successfully processed else false.
     */
    bool process_received_data(uint8_t *data,
                               uint16_t data_size,
                               sn_nsdl_addr_s *address);

    /**
     * @brief Stops all the timers in case there is any errors.
     */
    void stop_timers();

    /**
     * @brief Returns nsdl handle.
     * @return ndsl handle
     */
    nsdl_s* get_nsdl_handle() const;

    /**
     * @brief Get endpoint name
     * @return endpoint name
     */
    const char *endpoint_name() const;

    /**
     * @brief Get internal endpoint name
     * @return internal endpoint name
     */
    const char *internal_endpoint_name() const;

    /**
     * @brief Set server address
     * @param server_address, Bootstrap or M2M server address.
     */
    void set_server_address(const char *server_address);

    /**
     * @brief Get NSDL timer.
     * @return NSDL execution timer.
     */
    M2MTimer &get_nsdl_execution_timer();

    bool get_unregister_ongoing() const;

    /**
     * @brief Starts the NSDL execution timer.
     */
    void start_nsdl_execution_timer();

    sn_nsdl_dynamic_resource_parameters_s* get_first(struct iterator_state_s **state, char **path);

    sn_nsdl_dynamic_resource_parameters_s* get_next(struct iterator_state_s **state, char **path);

    sn_nsdl_dynamic_resource_parameters_s* find_resource(const char *path);

    void resource_path(M2MBase *base, char **path);

protected: // from M2MTimerObserver

    virtual void timer_expired(M2MTimerObserver::Type type);

protected: // from M2MObservationHandler

    virtual void observation_to_be_sent(M2MBase *object,
                                        uint16_t obs_number,
                                        const m2m::Vector<uint16_t> &changed_instance_ids,
                                        bool send_object = false);

    virtual void resource_to_be_deleted(M2MBase* base);

    virtual void value_updated(M2MBase *base);

    virtual void remove_object(M2MBase *object);

    virtual void send_post_response(M2MBase *base, const uint8_t *token, uint8_t token_length);

private:

    /**
     * Enum defining an LWM2M object type.
    */
    typedef enum {
        SECURITY = 0x00,
        SERVER   = 0x01,
        DEVICE   = 0x02
    }ObjectType;


    bool add_object_to_list(M2MObject *object);

    bool create_nsdl_object_structure(const M2MObject *object);

    bool create_nsdl_object_instance_structure(const M2MObjectInstance *object_instance);

    bool create_nsdl_resource_structure(const M2MResource *resource,
                                        bool multiple_instances = false);

    bool create_nsdl_resource(M2MBase *base);

    void execute_nsdl_process_loop();

    uint64_t registration_time() const;

    M2MBase* find_resource(const char *object,
                           const uint16_t msg_id) const;

    M2MBase* find_resource(const M2MObject *object,
                           const char *object_instance,
                           const uint16_t msg_id) const;

    M2MBase* find_resource(const M2MObjectInstance *object_instance,
                           const char *resource_instance,
                           const uint16_t msg_id) const;

    M2MBase* find_resource(const M2MResource *resource,
                           const char *object_name,
                           const char *resource_instance) const;

    static M2MInterface::Error interface_error(const sn_coap_hdr_s &coap_header);

    void send_object_observation(M2MObject *object,
                                 uint16_t obs_number,
                                 const m2m::Vector<uint16_t> &changed_instance_ids,
                                 bool send_object);

    void send_object_instance_observation(M2MObjectInstance *object_instance,
                                          uint16_t obs_number);

    void send_resource_observation(M2MResource *resource, uint16_t obs_number);

    /**
     * @brief Utility method to convert given lifetime int to ascii
     * and allocate a buffer for it and set it to _endpoint->lifetime_ptr.
     * @param lifetime A new value for lifetime.     
    */
    void set_endpoint_lifetime_buffer(int lifetime);

    /**
     * @brief Handle incoming bootstrap PUT message.
     * @param coap_header, Received CoAP message
     * @param address, Server address
    */
    void handle_bootstrap_put_message(sn_coap_hdr_s *coap_header, sn_nsdl_addr_s *address);

    /**
     * @brief Handle bootstrap finished message.
     * @param coap_header, Received CoAP message
     * @param address, Server address
    */
    void handle_bootstrap_finished(sn_coap_hdr_s *coap_header,sn_nsdl_addr_s *address);

    /**
     * @brief Handle bootstrap delete message.
     * @param coap_header, Received CoAP message
     * @param address, Server address
    */
    void handle_bootstrap_delete(sn_coap_hdr_s *coap_header,sn_nsdl_addr_s *address);

    /**
     * @brief Parse bootstrap TLV message.
     * @param coap_header, Received CoAP message
     * @return True if parsing was succesful else false
    */
    bool parse_bootstrap_message(sn_coap_hdr_s *coap_header, M2MNsdlInterface::ObjectType lwm2m_object_type);

    /**
     * @brief Parse bootstrap TLV message.
     * @param coap_header, Received CoAP message
     * @return True if parsing was succesful else false
    */
    bool validate_security_object();

    /**
     * @brief Handle bootstrap errors.
     * @param reason, Reason for Bootstrap failure.
     * @param wait, True if need to wait that ACK has been sent.
     *              False if reconnection can start immediately.
    */
    void handle_bootstrap_error(const char *reason, bool wait);

    /**
     * @brief Handle different coap errors.
     * @param coap_header, CoAP structure.
     * @return Error reason.
    */
    static const char *coap_error(const sn_coap_hdr_s &coap_header);

    /**
     * @brief Claim
     */
    void claim_mutex();

    /**
     * @brief Release
     */
    void release_mutex();

    /**
     * @brief Change operation mode of every resource.
     * @param object, Object to be updated.
     * @return operation, New operation mode.
    */
    void change_operation_mode(M2MObject *object, M2MBase::Operation operation);

    /**
     * @brief Parse URI query parameters and pass those to nsdl-c.
     * @return True if parsing success otherwise False
    */
    bool parse_and_send_uri_query_parameters();

private:

    M2MNsdlObserver                         &_observer;
    M2MObject                               *_object_list;
    sn_nsdl_ep_parameters_s                 _endpoint;
    nsdl_s                                  *_nsdl_handle;
#ifndef MBED_CLIENT_DISABLE_BOOTSTRAP_FEATURE
    M2MSecurity                             *_security; // owned
#endif
    M2MServer                               *_server;
    M2MTimer                                _nsdl_exceution_timer;
    M2MTimer                                _registration_timer;
    M2MConnectionHandler                    &_connection_handler;
    char                                    _endpoint_name[MAX_ALLOWED_STRING_LENGTH];

    // Todo: replace this String with const char* also, as it is just another copy,
    // but that change requires the caller to be changed to ensure the given data is
    // always available.
    uint32_t                                _counter_for_nsdl;
    uint16_t                                _bootstrap_id;
    char                                    _server_address[MAX_VALUE_LENGTH]; // BS or M2M address
    uint8_t                                 _endpoint_name_ptr[MAX_ALLOWED_STRING_LENGTH]; // endpoint name
    uint8_t                                 _domain_ptr[MAX_ALLOWED_STRING_LENGTH]; // domain/account ID
    uint8_t                                 _endpoint_type_ptr[MAX_ALLOWED_STRING_LENGTH]; // Endpoint type
    uint8_t                                 _lifetime[LIFETIME_LENGTH];
    bool                                    _unregister_ongoing;
    bool                                    _identity_accepted;
    bool                                    _nsdl_exceution_timer_running;
    uint8_t                                 _binding_mode;

friend class Test_M2MNsdlInterface;

};

#endif // M2MNSDLINTERFACE_H

