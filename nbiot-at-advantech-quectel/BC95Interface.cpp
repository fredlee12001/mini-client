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

#include "BC95Interface.h"

using namespace mbed;

// Various timeouts for different BC95 operations
#define BC95_CONNECT_TIMEOUT 1500
#define BC95_SEND_TIMEOUT    1500
#define BC95_RECV_TIMEOUT    1500
#define BC95_MISC_TIMEOUT    500

// BC95Interface implementation
BC95Interface::BC95Interface(PinName tx, PinName rx, bool debug)
    : _nbm(tx, rx, debug), _apn(0), _uname(0), _pwd(0), _pin(0), ap_sec(NSAPI_SECURITY_NONE), ap_ch(0)
{}

void BC95Interface::set_credentials(const char *apn, const char *username, const char *password)
{
    //only setting the APN is supported
    _nbm.set_credentials(apn);
}

void BC95Interface::set_plmn(int plmn)
{
    _nbm.set_plmn(plmn);
}

void BC95Interface::set_sim_pin(const char *sim_pin)
{
    //TBD
    //_pin = sim_pin;
}


nsapi_error_t BC95Interface::connect(const char *sim_pin, const char *apn,const char *username, const char *password)
{
    set_sim_pin(sim_pin);
    set_credentials(apn, username, password);

    return connect();
}

nsapi_error_t BC95Interface::connect()
{
    int startup_trials = 0;
    while (!_nbm.startup(NBIOT_BAND)) {
        startup_trials++;
        int reset_trials = 0;
        while (!_nbm.reset()) {
            reset_trials++;
            wait(2+reset_trials);
            if (reset_trials > BC95_MAX_RESET_TRIALS) {
                BC95_ERROR("Reset failed after %d attempts",BC95_MAX_RESET_TRIALS);
                break;
            }
        }
        wait(2+startup_trials);
        if (startup_trials > BC95_MAX_STARTUP_TRIALS) {
            BC95_ERROR("Startup failed after %d attempts",BC95_MAX_STARTUP_TRIALS);
            break;
        }
    }

    if (!_nbm.connect(NULL)) {
        return NSAPI_ERROR_NO_CONNECTION;
    }

    return NSAPI_ERROR_OK;
}

nsapi_error_t BC95Interface::disconnect()
{
    if (!_nbm.disconnect()) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

const char *BC95Interface::get_ip_address()
{
    return _nbm.getIPAddress();
}

const char *BC95Interface::get_netmask()
{
    return _nbm.getNetmask();
}

const char *BC95Interface::get_gateway()
{
    return _nbm.getGateway();
}

const char *BC95Interface::get_imsi()
{
    return _nbm.getIMSI();
}

bool BC95Interface::is_connected()
{
    return _nbm.isConnected();
}

const char *BC95Interface::get_mac_address()
{
    //return _nbm.getMACAddress();
    return BC95_MAC;
}

const char* BOOTS[]= {
    "coap-systemtest.dev.mbed.com",
    "bootstrap.us-east-1.mbedcloud.com"
};

const char* MDS[]= {
    "mds-systemtest.dev.mbed.com",
    "lwm2m.us-east-1.mbedcloud.com"
};

const char* BOOTSIP[]= {
    "169.50.115.241",
    "169.47.62.97"
};
const char* MDSIP[]= {
    "169.50.115.240",
    "169.47.62.96"
};

nsapi_error_t BC95Interface::gethostbyname(const char *name, SocketAddress *address, nsapi_version_t version)
{
    if (address->set_ip_address(name)) {
        if (version != NSAPI_UNSPEC && address->get_ip_version() != version) {
            printf("address_ver:%d, uper_ver:%d\n",address->get_ip_version(), version);
            return NSAPI_ERROR_DNS_FAILURE;
        }

        return NSAPI_ERROR_OK;
    }

    char *ipbuff = new char[NSAPI_IP_SIZE];
    int ret = 0;

    printf("Should not reach here, we didn't support DNS");

    delete[] ipbuff;
    return ret;
}

int BC95Interface::set_channel(uint8_t channel)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int8_t BC95Interface::get_rssi()
{
    return _nbm.getRSSI();
}

nsapi_error_t BC95Interface::socket_open(void **handle, nsapi_protocol_t proto)
{
    if(proto != NSAPI_UDP) {
        printf("BC95 only support UDP now!");
        return NSAPI_ERROR_UNSUPPORTED;
    }

    return _nbm.open("UDP", handle);
}

nsapi_error_t BC95Interface::socket_close(void *handle)
{
    int err = NSAPI_ERROR_OK;

    if (!_nbm.close(handle)) {
        err = NSAPI_ERROR_DEVICE_ERROR;
    }

    return err;
}

nsapi_error_t BC95Interface::socket_bind(void *handle, const SocketAddress &address)
{
    return _nbm.bind(handle, address.get_port());
}

nsapi_error_t BC95Interface::socket_listen(void *handle, int backlog)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_error_t BC95Interface::socket_connect(void *handle, const SocketAddress &addr)
{
    // Set connect state and address info to BC95
    _nbm.socket_connect(handle, addr.get_ip_address(), addr.get_port());

    return NSAPI_ERROR_OK;
}

nsapi_error_t BC95Interface::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_size_or_error_t BC95Interface::socket_send(void *handle, const void *data, unsigned size)
{
    return _nbm.sendto(handle, NULL, 0, data, size);
}

nsapi_size_or_error_t BC95Interface::socket_recv(void *handle, void *data, unsigned size)
{
    // IP address and port are always needed in BC95 for AT receive from socket which is actually a recvfrom,
    // so we can allocate them already here even if we will not use the information further
    char ip_address[NSAPI_IPv4_SIZE];
    int port;
    return _nbm.recvfrom(handle, data, size, ip_address, &port);
}

nsapi_size_or_error_t BC95Interface::socket_sendto(void *handle, const SocketAddress &addr, const void *data, unsigned size)
{
    return _nbm.sendto(handle, addr.get_ip_address(), addr.get_port(), data, size);
}

nsapi_size_or_error_t BC95Interface::socket_recvfrom(void *handle, SocketAddress *addr, void *data, unsigned size)
{
    char ip_address[NSAPI_IPv4_SIZE];
    int port;
    nsapi_size_or_error_t ret_val = _nbm.recvfrom(handle, data, size, ip_address, &port);

    // If data received
    if (ret_val >= 0) {
        // Set ip address and port for the socket address
        addr->set_ip_address((const char*)ip_address);
        addr->set_port(port);
    }

    return ret_val;
}

void BC95Interface::socket_attach(void *handle, void (*callback)(void *), void *data)
{
    _nbm.socket_attach(handle, callback, data);
}
