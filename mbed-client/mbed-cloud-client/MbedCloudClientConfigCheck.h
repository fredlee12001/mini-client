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

#ifndef MBED_CLOUD_CONFIG_CHECK_H
#define MBED_CLOUD_CONFIG_CHECK_H

/*! \file MbedCloudClientConfigCheck.h
* \brief Configuration options check.
*
*  This set checks and validates the compile-time options that can be made for possible client library.
*  NOTE: Not to be used by developers.
*/

#ifndef MBED_CLOUD_CLIENT_ENDPOINT_TYPE
#error "MBED_CLOUD_CLIENT_ENDPOINT_TYPE must be defined with valid endpoint type"
#endif

#ifndef MBED_CLOUD_CLIENT_LIFETIME
#error "MBED_CLOUD_CLIENT_LIFETIME must be defined with valid non-zero lifetime value in seconds, default is 60"
#endif

#ifndef MBED_CLOUD_CLIENT_LISTEN_PORT
#error "MBED_CLOUD_CLIENT_LISTEN_PORT must be defined with valid non-zero port number, default is 0"
#endif

#if !defined (SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE) || (SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE < 16)
#error "SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE must be defined with one of the values from this - 128, 256, 512 or 1024"
#endif

#if defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) && ( defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) || \
defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE) || defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE))
#error "Only one MBED_CLOUD_CLIENT_TRANSPORT_MODE can be defined at a time"
#endif

#if defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) && ( defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) || \
defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE) || defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE))
#error "Only one MBED_CLOUD_CLIENT_TRANSPORT_MODE can be defined at a time"
#endif

#if defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE) && ( defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) || \
defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) || defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE))
#error "Only one MBED_CLOUD_CLIENT_TRANSPORT_MODE can be defined at a time"
#endif

#if defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE) && ( defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) || \
defined (MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE) || defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP))
#error "Only one MBED_CLOUD_CLIENT_TRANSPORT_MODE can be defined at a time"
#endif

#if !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) \
&& !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP_QUEUE)
#error "One MBED_CLOUD_CLIENT_TRANSPORT_MODE must be defined at a time"
#endif

#endif // MBED_CLOUD_CONFIG_CHECK_H
