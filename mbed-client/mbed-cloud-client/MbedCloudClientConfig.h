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

#ifndef MBED_CLOUD_CLIENT_CONFIG_H
#define MBED_CLOUD_CLIENT_CONFIG_H

#include <stdint.h>

/*! \file MbedCloudClientConfig.h
* \brief Configuration options (set of defines and values).
*
*  This lists a set of compile-time options that needs to be used to enable
*  or disable features selectively, and set the values for the mandatory
*  parameters.
*/

#ifdef __DOXYGEN__

/**
* \def MBED_CLOUD_CLIENT_ENDPOINT_TYPE
*
* \brief This is mandatory MACRO and MUST be enabled. You MUST define it like this #define MBED_CLOUD_CLIENT_ENDPOINT_TYPE "default".
*/
#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE          /* "default" */

/**
* \def MBED_CLOUD_CLIENT_LIFETIME
*
* \brief This is mandatory MACRO and MUST be enabled. You MUST define it like this: #define MBED_CLOUD_CLIENT_LIFETIME 60.
* This value denotes time in seconds.
*/
#define MBED_CLOUD_CLIENT_LIFETIME               /* 60 */

/**
* \def MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP
*
* \brief Enable this MACRO if you want to enable UDP mode for the client.
*/
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP

/**
* \def MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
*
* \brief Enable this MACRO if you want to enable TCP mode for the client.
*/
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP

/**
* \def MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
*
* \brief Enable this MACRO if you want to enable UDP_QUEUE mode for the client.
*/
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE

/**
* \def MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE
*
* \brief Enable this MACRO if you want to enable TCP_QUEUE mode for the client.
*/

#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE

#endif

/**
* \def MBED_CLOUD_CLIENT_LISTEN_PORT
*
* \brief This is mandatory MACRO and is set to 0 by default. This implies that the client picks a random port
 * for listening to the incoming connection.
*/
#define MBED_CLOUD_CLIENT_LISTEN_PORT           0

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#include "MbedCloudClientConfigCheck.h"

#endif // MBED_CLOUD_CLIENT_CONFIG_H
