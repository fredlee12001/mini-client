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

#include "config.h"

#ifndef TEST_MQTTSN

#include "mbed.h"
#ifdef DEBUG_USE_ETHERNET
#include "EthernetInterface.h"
#else
#include "BC95Interface.h"
#endif // DEBUG_USE_ETHERNET

#ifdef TARGET_MCU_K64F
#define UART_TX_BC95 PTC17
#define UART_RX_BC95 PTC16
#elif TARGET_STM32L4
#define UART_TX_BC95 PC_1
#define UART_RX_BC95 PC_0
#else
#error "You need to define UART pins for your target"
#endif // TARGET_LPC11XX

using namespace mbed;

#ifdef DEBUG_USE_ETHERNET
static EthernetInterface net;
#else
static BC95Interface net(UART_TX_BC95, UART_RX_BC95, true);
#endif // DEBUG_USE_ETHERNET

#ifdef DEBUG_UART
int main()
{
    Serial pc(USBTX, USBRX, 9600);
    Serial bc95(UART_TX_BC95, UART_RX_BC95, 9600);
    while(1) {
        if(pc.readable()) {
            bc95.putc(pc.getc());
        }
        if(bc95.readable()) {
            pc.putc(bc95.getc());
        }
    }
}
#else

#ifdef ASYNC_RECV

static rtos::Semaphore sock_event;

void receive_indication_from_bc95()
{
    sock_event.release();
}

#endif // ASYNC_RECV



#ifdef DEBUG_TEST_APN_PLMN
void test_apn_plmn(int test_choice);
#endif



int main()
{
    while (1) {
#ifdef DEBUG_TEST_APN_PLMN
        test_apn_plmn(DEBUG_TEST_APN_PLMN);
#endif
        net.connect();
        const char *ip = net.get_ip_address();

        UDPSocket unconnected_socket;
        nsapi_error_t err = unconnected_socket.open(&net);
        if (err < 0) {
            printf("ERROR: UDP socket creation failed with error: %+5d\r\n", err);
        }

        srand(time(NULL));
        int port = UDP_SOCKET_PORT;
        // for testing we simply use fixed UDP_SOCKET_PORT
#ifndef DEBUG_TEST_AT
        // randomise port here to prevent using the same port always after restart
        // using the same port might confuse e.g. server or network NAT
        port += (rand()&0xff);
#endif //DEBUG_TEST_AT
        err = unconnected_socket.bind(port);
        if (err < 0) {
            printf("ERROR: UDP socket bind failed with error: %+5d\r\n", err);
        }

#ifdef ASYNC_RECV
        printf("Non-blocking sockets in use.\r\n");
        unconnected_socket.sigio(receive_indication_from_bc95);
        unconnected_socket.set_blocking(false);
#else
        printf("Blocking sockets in use.\r\n");
        unconnected_socket.set_blocking(true);
#endif // ASYNC_RECV

#ifndef TEST_CLIENT
        printf("UDP echo server started.\r\nTest on local PC: python test-client.py %s %d\r\n", ip ? ip : "No IP", port);
        // Receive data from UDP socket and echo everything back to sender.
        while (1) {
            SocketAddress address;
            char rbuffer[BC95_MAX_PACKAGE_SIZE];
            int rcount = unconnected_socket.recvfrom(&address, rbuffer, sizeof rbuffer);
            if (rcount >= 0) {
                if (rcount == 0) {
                    rbuffer[0] = '\0';
                }
                printf("From %s:%d\r\n", address.get_ip_address(), address.get_port());
                printf("Recv %d [%.*s]\r\n", rcount, rcount, rbuffer);
                int scount = unconnected_socket.sendto(address, rbuffer, rcount);
                printf("Sent %d [%.*s]\r\n", scount, scount, rbuffer);
                if (strncasecmp(rbuffer, "quit", 4) == 0)
                    break;
            } else {
#ifdef ASYNC_RECV
                int32_t event;
                event = sock_event.wait();
                if (event < 1) {
                    continue;
                }
#endif // ASYNC_RECV
            }
        }
#else // is not TEST_CLIENT
        printf("Sending UDP message from %s to %s once per second\r\nTest on local PC: python test-server.py %s %d\r\n", ip ? ip : "No IP", SERVER_IP_ADDR, SERVER_IP_ADDR, port);
#ifndef ASYNC_RECV
        unconnected_socket.set_timeout(1000);
#endif // ASYNC_RECV
        // Send UDP packet once per second and read any data back from server.
        while (1) {
            SocketAddress address;
            char sbuffer[] = UDP_TEST_PACKET;
            int scount = unconnected_socket.sendto(SERVER_IP_ADDR, port, sbuffer, sizeof sbuffer);
            printf("Sent %d [%.*s]\r\n", scount, scount, sbuffer);
            char rbuffer[BC95_MAX_PACKAGE_SIZE];
            int rcount = unconnected_socket.recvfrom(&address, rbuffer, sizeof rbuffer);
            if (rcount >= 0) {
                if (rcount == 0) {
                    rbuffer[0] = '\0';
                }
                printf("From %s:%d\r\n", address.get_ip_address(), address.get_port());
                printf("Recv %d [%.*s]\r\n", rcount, rcount, rbuffer);
                if (strncasecmp(rbuffer, "quit", 4) == 0)
                    break;
                wait(1);
            }  else {
#ifdef ASYNC_RECV
                int32_t event;
                event = sock_event.wait(1000);
                if (event < 1) {
                    continue;
                }
#endif // ASYNC_RECV
            }
        }
#endif // DEBUG_TEST_AT

        unconnected_socket.close();

        net.disconnect();
    }
    //return 0;
}



#ifdef DEBUG_TEST_APN_PLMN
void test_apn_plmn(int test_choice)
{
    printf("Setting APN PLMN\r\n");
    switch (test_choice) {
        case 0: 
            net.set_credentials("www.example.com"); 
            break;
        case 1: 
            net.set_plmn(50001);
            break;
        default:
            net.set_credentials("www.example.com"); 
            net.set_plmn(50001);
            break;
    }
}
#endif




#endif // DEBUG_UART

#endif // TEST_MQTTSN
