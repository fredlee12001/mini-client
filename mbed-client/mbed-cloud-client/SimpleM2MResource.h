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

#ifndef SIMPLE_M2M_RESOURCE_H
#define SIMPLE_M2M_RESOURCE_H


#include "mbed-cloud-client/MbedCloudClient.h"

/*! \file SimpleM2MResource.h
 *  \brief SimpleM2MResourceBase.
 * This class provides an easy wrapper base class for creating a simple M2MResource based on
 * integer and string values. This class is NOT meant to be directed instantiated but is used
 * by the SimpleM2MResourceInt and SimpleM2MResourceString classes to create resources.
 */

class SimpleM2MResourceBase {

protected:

    // Prevents the use of default constructor.
    SimpleM2MResourceBase();

    // Prevents the use of assignment operator.
    SimpleM2MResourceBase& operator=( const SimpleM2MResourceBase& /*other*/ );

    // Prevents the use of copy constructor
    SimpleM2MResourceBase( const M2MBase& /*other*/ );

    SimpleM2MResourceBase(MbedCloudClient* client, const char *route);

    /**
     * \brief Destructor
     */
    virtual ~SimpleM2MResourceBase();


public:

    /**
     * \brief Defines M2MResource internally and creates a necessary LWM2M
     * structure like object and object instance based on the given string
     * URI path and sets the right M2M operation to the resource.
     * \param v The URI path for the resource "Test/0/res" in this format.
     * \param opr An operation to be set for the resource.
     * \param observable True if the resource is observable, else false.
     * \return True if resource is created, else false.
     */
    bool define_resource_internal(const char *v,
                                  M2MBase::Operation opr,
                                  bool observable);

    /**
     * \brief Gets the value set in a resource in text format.
     * \return The value set in the resource.
     */
    const char* get() const;

    /**
     * \brief Sets the value in a resource in text format.
     * \param v The value to be set.
     * \return True if set successfully, else false.
     */
    bool set(const char *v);

    /**
     * \brief Sets the value in a resource in integer format.
     * \param v The value to be set.
     * \return True if set successfully, else false.
     */
    bool set(const int& v);

    /**
     * \brief Sets the callback function to be called
     * when the resource received a POST command.
     * \param fn A function to be called.
     * This is used for a statically defined function.
     * \return True if set successfully, else false.
     */
    bool set_post_function(void(*fn)(void*));

    /**
     * \brief Sets the callback function to be called
     * when a resource received the POST command.
     * \param fn A function to be called.
     * This is an overloaded function for a class function.
     * \return True if set successfully, else false.
     */
    bool set_post_function(execute_callback fn);

    /**
    * \brief Returns the M2MResource object of the registered object through
    * the SimpleM2MResourceBase objects.
    * \return The object of the M2MResource.
    */
    M2MResource* get_resource();

    /**
    * \brief Calls when there is an indication that the value of the resource
    * object is updated by the LWM2M Cloud server.
    */
    virtual void update(){}

private:

    /**
    * \brief An internal helper function to break the URI path into a list of
    * strings such as "Test/0/res" into a list of three strings.
    * \param route The URI path in the format "Test/0/res".
    * \return A list of strings parsed from the URI path.
    */
    vector<const char*> parse_route(const char* route);

private:
    MbedCloudClient*                    _client;   // Not owned
    const char*                         _route;
};

/**
 *  \brief SimpleM2MResourceString.
 *  This class provides an easy wrapper base class for creating a simple M2MResource based on
 * string values. This class provides an easy access to the M2MResource objects without the application
 * requiring to create Objects and Object Instances.
 */

class SimpleM2MResourceString : public SimpleM2MResourceBase
{
public:

    /**
     *  \brief Constructor.
     *  \param client A handler for MbedCloudClient.
     *  \param route The route for the resource such as "Test/0/res".
     *  \param v The value of the resource.
     *  \param opr An operation that can be supported by the resource.
     *  \param observable True if the resource is observable.
     *  \param on_update If the resource supports the PUT operation, a function pointer
     *  for the callback function that is called when the client receives an
     *  updated value for this resource.
     */
    SimpleM2MResourceString(MbedCloudClient* client,
                   const char* route,
                   const char *v,
                   M2MBase::Operation opr = M2MBase::GET_PUT_ALLOWED,
                   bool observable = true,
                   FP1<void, const char*> on_update = NULL);

    /**
     *  \brief Constructor. This is overloaded function.
     *  \param client A handler for MbedCloudClient.
     *  \param route The route for the resource such as "Test/0/res".
     *  \param v The value of the resource.
     *  \param opr An operation that can be supported by the resource.
     *  \param observable True if resource is observable.
     *  \param on_update If the resource supports the PUT operation, a function pointer
     *  for the callback function that is called when the client receives an
     *  updated value for this resource.
     */
    SimpleM2MResourceString(MbedCloudClient* client,
                   const char* route,
                   const char *v,
                   M2MBase::Operation opr,
                   bool observable,
                   void(*on_update)(const char*));


    /**
     * \brief Destructor
     */
    virtual ~SimpleM2MResourceString();

    /**
     * \brief Overloaded operator for = operation.
     */
    const char *operator=(const char *new_value);

    /**
     * \brief Overloaded operator for string() operation.
     */
    operator const char*() const;

    /**
    * \brief Calls when there is an indication that the value of the resource
    * object is updated by the LWM2M Cloud server.
    */
    virtual void update();

private:
    FP1<void, const char*>                   _on_update;
};

/**
 *  \brief SimpleM2MResourceInt.
 *  This class provides an easy wrapper base class for creating a simple M2MResource based on
 * integer values. This class provides easy access to M2MResource objects without the application
 * requiring to create Objects and Object Instances.
 */

class SimpleM2MResourceInt : public SimpleM2MResourceBase
{
public:

    /**
     *  \brief Constructor.
     *  \param client A handler for MbedCloudClient.
     *  \param route The route for the resource such as "Test/0/res".
     *  \param v The value of the resource.
     *  \param opr An operation that can be supported by the resource.
     *  \param observable True if the resource is observable, else false.
     *  \param on_update If the resource supports the PUT operation, a function pointer
     *  for the callback function that is called when the client receives an
     *  updated value for this resource.
     */
    SimpleM2MResourceInt(MbedCloudClient* client,
                const char* route,
                int v,
                M2MBase::Operation opr = M2MBase::GET_PUT_ALLOWED,
                bool observable = true,
                FP1<void, int> on_update = NULL);

    /**
     *  \brief Constructor. This is an overloaded function
     *  \param client A handler for MbedCloudClient.
     *  \param route The route for the resource such as "Test/0/res"
     *  \param v The value of the resource.
     *  \param opr An operation that can be supported by the resource.
     *  \param observable True if the resource is observable, else false.
     *  \param on_update If the resource supports the PUT operation, a function pointer
     *  for the callback function that is called when the client receives an
     *  updated value for this resource.
     */
    SimpleM2MResourceInt(MbedCloudClient* client,
                const char* route,
                int v,
                M2MBase::Operation opr,
                bool observable,
                void(*on_update)(int));

    /**
     * \brief Destructor
     */
    virtual ~SimpleM2MResourceInt();

    /**
     * \brief Overloaded operator for = operation.
     */
    int operator=(int new_value);

    /**
     * \brief Overloaded operator for int() operation.
     */
    operator int() const;

    /**
    * \brief Calls when there is an indication that the value of the resource
    * object is updated by the LWM2M Cloud server.
    */
    virtual void update();

private:
    FP1<void, int>                  _on_update;
};

#endif // SIMPLE_M2M_RESOURCE_H
