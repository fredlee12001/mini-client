/* Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define NBIOT_BAND (BC95::BAND_850MHZ_B5)
#define BC95_MAX_PACKAGE_SIZE 512
#define BC95_MAC "12:fe:34:dc:83:b7" // just some private address

// out-comment all to test as UDP server (needs public mobile IP address)
#define TEST_CLIENT
//#define TEST_MQTTSN // use MQTTSN main instead of UDP test server/client

// random local port from dynamic UDP port range
// this port is requested from BC95 by default without socket::bind
#define UDP_SOCKET_PORT 58032

// you must set server IP address where application starts sending UDP packets
#define SERVER_IP_ADDR "10.45.0.75"

#define UDP_TEST_PACKET {'H', 'e', 'l', 'l', 'o', '!'}

#define ASYNC_RECV // handle +NSOMI

// Debug flags
//#define DEBUG_UART // debug UART by forwarding chars from PC to BC95 and v.v.
//#define DEBUG_USE_ETHERNET // debug application over Ethernet socket
//#define DEBUG_TEST_AT // mockup test for AT command interface

#ifdef DEBUG_TEST_AT

#ifdef TEST_CLIENT
#warning Disabled TEST_CLIENT due to AT testing
#undef TEST_CLIENT
#endif // TEST_CLIENT

#ifdef TEST_MQTTSN
#warning Disabled TEST_MQTTSN due to AT testing
#undef TEST_MQTTSN
#endif // TEST_MQTTSN

#endif // DEBUG_TEST_AT


#ifdef DEBUG_TEST_AT
#define TEST_APN_PLMN 0
#if TEST_APN_PLMN
#define DEBUG_TEST_APN_PLMN (TEST_APN_PLMN-1)
#endif
#endif

#endif /* CONFIG_H_ */
