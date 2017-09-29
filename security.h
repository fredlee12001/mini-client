

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

#ifndef __SECURITY_H__

#define __SECURITY_H__

 

#include <inttypes.h>

 

#define MBED_DOMAIN "fe8a3555-e7ed-4503-8a8d-2dc129629300"

#define MBED_ENDPOINT_NAME "fd4704af-abf2-4bac-bb39-019f96421e85"

 

const uint8_t SERVER_CERT[] = "-----BEGIN CERTIFICATE-----\r\n"

"MIIBmDCCAT6gAwIBAgIEVUCA0jAKBggqhkjOPQQDAjBLMQswCQYDVQQGEwJGSTEN\r\n"

"MAsGA1UEBwwET3VsdTEMMAoGA1UECgwDQVJNMQwwCgYDVQQLDANJb1QxETAPBgNV\r\n"

"BAMMCEFSTSBtYmVkMB4XDTE1MDQyOTA2NTc0OFoXDTE4MDQyOTA2NTc0OFowSzEL\r\n"

"MAkGA1UEBhMCRkkxDTALBgNVBAcMBE91bHUxDDAKBgNVBAoMA0FSTTEMMAoGA1UE\r\n"

"CwwDSW9UMREwDwYDVQQDDAhBUk0gbWJlZDBZMBMGByqGSM49AgEGCCqGSM49AwEH\r\n"

"A0IABLuAyLSk0mA3awgFR5mw2RHth47tRUO44q/RdzFZnLsAsd18Esxd5LCpcT9w\r\n"

"0tvNfBv4xJxGw0wcYrPDDb8/rjujEDAOMAwGA1UdEwQFMAMBAf8wCgYIKoZIzj0E\r\n"

"AwIDSAAwRQIhAPAonEAkwixlJiyYRQQWpXtkMZax+VlEiS201BG0PpAzAiBh2RsD\r\n"

"NxLKWwf4O7D6JasGBYf9+ZLwl0iaRjTjytO+Kw==\r\n"

"-----END CERTIFICATE-----\r\n";

 

const uint8_t CERT[] = "-----BEGIN CERTIFICATE-----\r\n"

"MIIBzjCCAXOgAwIBAgIEGE+VzzAMBggqhkjOPQQDAgUAMDkxCzAJBgNVBAYTAkZ\r\n"

"JMQwwCgYDVQQKDANBUk0xHDAaBgNVBAMME21iZWQtY29ubmVjdG9yLTIwMTgwHh\r\n"

"cNMTcwOTIxMDI0NzUzWhcNMTgxMjMxMDYwMDAwWjCBoTFSMFAGA1UEAxNJZmU4Y\r\n"

"TM1NTUtZTdlZC00NTAzLThhOGQtMmRjMTI5NjI5MzAwL2ZkNDcwNGFmLWFiZjIt\r\n"

"NGJhYy1iYjM5LTAxOWY5NjQyMWU4NTEMMAoGA1UECxMDQVJNMRIwEAYDVQQKEwl\r\n"

"tYmVkIHVzZXIxDTALBgNVBAcTBE91bHUxDTALBgNVBAgTBE91bHUxCzAJBgNVBA\r\n"

"YTAkZJMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErgK1Gw+TlFOt4SA0ofbmE\r\n"

"ssK4oo22W+lu8nnIr6WzpG8TXPaZqej10n0JBQUGNXdIL39UqK7aIp2tXa8UdkN\r\n"

"hjAMBggqhkjOPQQDAgUAA0cAMEQCIGSke2512HggfY/JfwhdJEockxdUVNanp9h\r\n"

"39G7W/Z/QAiAS5N1P3JvIEQJubeYER+CGsPU26QrTC9GGEhdwXdhyUA==\r\n"

"-----END CERTIFICATE-----\r\n";

 

const uint8_t KEY[] = "-----BEGIN PRIVATE KEY-----\r\n"

"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgoep8I6dZOEpV4fbq\r\n"

"P/ww3s+QP5qzhhua1IRVFROXbSuhRANCAASuArUbD5OUU63hIDSh9uYSywriijbZ\r\n"

"b6W7yecivpbOkbxNc9pmp6PXSfQkFBQY1d0gvf1Sortoina1drxR2Q2G\r\n"

"-----END PRIVATE KEY-----\r\n";

 

#endif //__SECURITY_H__
