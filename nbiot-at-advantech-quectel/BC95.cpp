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

#include "BC95.h"

// A string that would not normally be sent by the modem on the AT interface.
#define UNNATURAL_STRING "\x01"

BC95::BC95(PinName tx, PinName rx, bool debug)
    : _serial(tx, rx, _BC95_CONNECT_DEFAULT_BAUD),
      _parser(&_serial, _BC95_CONNECT_DELIM, _BC95_CONNECT_BUFFER_SIZE, _BC95_CONNECT_TIMEOUT, debug),
      _debugIsOn(debug),
      _packets(0),
      _packets_end(&_packets),
      _apnLength(0),
      _plmn(0)
{
    memset(_socket, 0, sizeof(_socket));

    _parser_timeout = 0;

    _parser.oob("+NSONMI:", mbed::Callback<void()>(this, &BC95::rx_msg_ind));

    _event_thread.start(callback(this, &BC95::handle_event));
}


int BC95::set_credentials(const char *apn) {
    int i = 0;

    while (apn[i] != '\0' && i < _BC95_APN_MAX_LENGTH) {
        _apn[i] = apn[i];
        i++; 
    }
    _apnLength = i;

    return i;
}


void BC95::set_plmn(int plmn) {
    _plmn = plmn;
}



bool BC95::reset(void)
{
    BC95_INFO(">>>>reset the bc95 module.");

    _serialMutex.lock();

    _parser.send("AT+NRB");
    if(!(_parser.recv("REBOOTING"))) {
        BC95_ERROR("BC95::reset: Could not get response of 'REBOOTING'.");
        _parser.flush();
        _serialMutex.unlock();
        return false;
    }

    _serialMutex.unlock();

    return true;
}


bool BC95::_atsync()
{
    _serialMutex.lock();

    setTimeout(_BC95_MISC_TIMEOUT);

    _parser.flush();

    /* QUECTEL: Start AT SYNC: Send AT every 1s, if receive OK, SYNC success, if no OK return after sending AT 10 times, SYNC fail */
    for(int i=0; i<BC95_ATSYNC_RETRY; i++) {
        BC95_INFO("\t>>>>BC95 AT SYNC >>> %d",i);
        if (_parser.send("AT") && _parser.recv("OK\n")) {
            _serialMutex.unlock();
            return true;
        }

        wait(1);
    }

    _serialMutex.unlock();

    BC95_ERROR("BC95 AT SYNC Fail.");
    
    return false;
}


bool BC95::startup(int band)
{
    //bool retval;
    BC95_INFO("======================================================");
    BC95_INFO("BC95 INIT, Start>>>>");

    _serialMutex.lock();

    _parser.debug_on(false);

    /* AT SYNC Before each set of AT CMD*/
    if (!_atsync()) {
        BC95_ERROR("The module failed to get AT sync");
        _serialMutex.unlock();
        return false;
    }

    setTimeout(_BC95_CONNECT_TIMEOUT);


    /* Query module support Bands */
    /*
    [2017-07-13 20:09:54:382_S:] AT+NBAND?
    [2017-07-13 20:09:54:396_R:]
    [2017-07-13 20:09:54:398_R:] +NBAND:5
    [2017-07-13 20:09:54:408_R:]
    [2017-07-13 20:09:54:411_R:] OK
    */
    _parser.send("AT+NBAND?");
    bool restartIsNotNeeded = _parser.recv("+NBAND:%d\n", &_cfg.band) && _parser.recv("OK\n");

    BC95_INFO(">>>>Module Band Support = %d, requested = %d",_cfg.band, band);
    if (_cfg.band != band) {
        restartIsNotNeeded = false;
        // Set band and reset to take into use
        if (!(_parser.send("AT+NBAND=%d",band) && _parser.recv("OK\n"))) {
            BC95_ERROR("The module failed to change band to %d", band);
        }
    }

    /* Query the user configuration */
    /*
    [2017-07-13 20:09:54:416_S:] AT+NCONFIG?
    [2017-07-13 20:09:54:432_R:]
    [2017-07-13 20:09:54:433_R:] +NCONFIG:AUTOCONNECT,TRUE
    [2017-07-13 20:09:54:461_R:] +NCONFIG:CR_0354_0338_SCRAMBLING,TRUE
    [2017-07-13 20:09:54:502_R:] +NCONFIG:CR_0859_SI_AVOID,TRUE
    [2017-07-13 20:09:54:539_R:]
    [2017-07-13 20:09:54:541_R:] OK
    */
    char tmpstr[10];
    _parser.send("AT+NCONFIG?");
    bool nconfigReplyReceived = _parser.recv("+NCONFIG:AUTOCONNECT,%9[^\n]\n",tmpstr);
    if (nconfigReplyReceived) {
        _cfg.autoconnect = (strstr(tmpstr,"TRUE")==NULL)?0:1;
        BC95_INFO(">>>>+NCONFIG:AUTOCONNECT = %d",_cfg.autoconnect);
    
        bool nconfigRemainingWorking = _parser.recv("+NCONFIG:CR_0354_0338_SCRAMBLING,%9[^\n]\n",tmpstr);
        if (nconfigRemainingWorking) {
            _cfg.scrambling = (strstr(tmpstr,"TRUE")==NULL)?0:1;
        }
        nconfigRemainingWorking = _parser.recv("+NCONFIG:CR_0859_SI_AVOID,%9[^\n]\n",tmpstr);
        if (nconfigRemainingWorking) {
            _cfg.si_avoid = (strstr(tmpstr,"TRUE")==NULL)?0:1;
        }
        nconfigRemainingWorking = _parser.recv("OK\n");
        if (nconfigRemainingWorking) {
            BC95_INFO(">>>>+NCONFIG:CR_0354_0338_SCRAMBLING = %d",_cfg.scrambling);
            BC95_INFO(">>>>+NCONFIG:CR_0859_SI_AVOID = %d",_cfg.si_avoid);
        } else {
            BC95_ERROR("Function values after autoconnect not received");
            _parser.flush();
        }
    } else {
        BC95_ERROR("Autoconnect not received");
        restartIsNotNeeded = false;
    }


    if (!_cfg.autoconnect && !_plmn && !_apnLength && nconfigReplyReceived) {
        restartIsNotNeeded = false;
        if (!(_parser.send("AT+NCONFIG=AUTOCONNECT,TRUE") && _parser.recv("OK\n"))) {
            BC95_ERROR("Setting autoconnect failed");
        }
    } else if ((_plmn || _apnLength) && _cfg.autoconnect && nconfigReplyReceived) {
        //if autoconnect was in use and _plmn or _apn was set, then disable autoconnect
        restartIsNotNeeded = false;
        if (!(_parser.send("AT+NCONFIG=AUTOCONNECT,FALSE") && _parser.recv("OK\n"))) {
            BC95_ERROR("Setting autoconnect failed");
        }
    }

    _serialMutex.unlock();

    if (restartIsNotNeeded) {
        return true;
    } else {
        return false;
    }
}



bool BC95::connect(const char *apn)
{
    BC95_INFO("======================================================");
    BC95_INFO("BC95 CONNECT, Start>>>>");

    _serialMutex.lock();

    setTimeout(_BC95_CONNECT_TIMEOUT);

    // Query functionality level
    _parser.send("AT+CFUN?");
    bool cfunSetWorking = _parser.recv("+CFUN:%d\n", &_state.cfun) && _parser.recv("OK\n");
    BC95_INFO(">>>> CFUN= %d",_state.cfun);

    if(_state.cfun!=1 && cfunSetWorking) {
        BC95_INFO(">>>> The device is working on the mode of minimum functionality,SET CFUN to 1");
        cfunSetWorking = _parser.send("AT+CFUN=1") && _parser.recv("OK\n");

        if (cfunSetWorking) {
            _parser.send("AT+CFUN?");
            cfunSetWorking = _parser.recv("+CFUN:%d", &_state.cfun) && _parser.recv("OK\n");
            BC95_INFO("\t>>>> CFUN :  %d", _state.cfun);
        }
    }

    if (!cfunSetWorking) {
        BC95_ERROR("Setting cfun failed");
        _parser.flush();
        //continue trying on debug mode
        if (!_debugIsOn) {
            _serialMutex.unlock();
            return false;
        }
    }


    if (_plmn) {
        BC95_INFO("Setting plmn %d", _plmn);
        if (!(_parser.send("AT+COPS=1,2,\"%d\"",_plmn) && _parser.recv("OK\n"))) {
            BC95_ERROR("BC95 PLMN set failing");
            _parser.flush();
        }
    }


   if (apn || _apnLength) {
        if (apn) {
            set_credentials(apn);
        }
        //the context id is fixed to 0
        char command[19+_BC95_APN_MAX_LENGTH+2] = "AT+CGDCONT=0,\"IP\",\"";
        int j = 0;
        int i=19;
        for (; i < _apnLength+19; i++) {
            command[i] = _apn[j++];
        }
        command[i++] = '"';
        command[i] = '\0';
        BC95_INFO("Setting apn %s", _apn);
        if (!(_parser.send(command) && _parser.recv("OK\n"))) {
            BC95_ERROR("BC95 APN set failing");
            _parser.flush();
        }
    }


    bool contextActivationWorking = _parser.send("AT+CGATT?") && _parser.recv("+CGATT:%d\nOK\n", &_state.att);
    if (!contextActivationWorking) {
        BC95_ERROR("BC95 Activation check failing");
        _parser.flush();
    }

    if (_state.att == 0 || !contextActivationWorking) {
        // User "AT+CGATT=1"to activate context profile 
        //
        //[2017-07-28 16:29:24:417_S:] AT+CGATT=1
        //[2017-07-28 16:29:24:432_R:]
        //[2017-07-28 16:29:24:434_R:] OK
        //
        BC95_INFO(">>>> Activate context profile and start to wait for activation, max 30s");
        contextActivationWorking = _parser.send("AT+CGATT=1") && _parser.recv("OK\n");
        if (!contextActivationWorking) {
            BC95_ERROR("BC95 Activation Failing");
            wait(2);
        }

        for(int i=0; i<BC95_CGATT_RETRY; i=i+2) {
            // Query the status of the context profile,You may have to wait for several seconds
            //
            //[2017-07-28 16:29:24:441_S:] AT+CGATT?
            //[2017-07-28 16:29:24:455_R:]
            //[2017-07-28 16:29:24:457_R:] +CGATT:1
            //[2017-07-28 16:29:24:468_R:]
            //[2017-07-28 16:29:24:470_R:] OK
            //
            if (!contextActivationWorking) {
                contextActivationWorking = _parser.send("AT+CGATT=1") && _parser.recv("OK\n");
            }

            bool contextCheckWorking = _parser.send("AT+CGATT?") && _parser.recv("+CGATT:%d\nOK\n", &_state.att);

            BC95_INFO("\t>>>> Query the status of the context profile - %d second", i);
            if(_state.att && contextActivationWorking && contextCheckWorking) {
                BC95_INFO(">>>>Succesfully attached to the network!");
                break;
            }
            wait(2);
        }
    }

    if(_state.att == 0) {
        _serialMutex.unlock();
        BC95_ERROR("BC95 Activation Fail");
        return false;
    }




    // TBD. Use AT+CEREG =1 to enable URC (network registration unsolicited) result code
    //
    //[2017-07-28 16:29:24:587_S:] AT+CEREG=1
    //[2017-07-28 16:29:24:603_R:]
    //[2017-07-28 16:29:24:605_R:] OK

    //BC95_ASSERT(_parser.send("AT+CEREG=1"));
    //BC95_ASSERT(_parser.recv("OK\n"));
    //

    //Use AT+CEREG? to query current EPS Network Registration Status
    //
    //[2017-07-13 20:09:55:270_S:] AT+CEREG?
    //[2017-07-13 20:09:55:284_R:]
    //[2017-07-13 20:09:55:287_R:] +CEREG:1,1
    //[2017-07-13 20:09:55:300_R:]
    //[2017-07-13 20:09:55:301_R:] OK
    //
    bool regCheckWorking = true;
    for(int i=0; i<BC95_EPS_REG_RETRY && regCheckWorking; i++) {

        _parser.send("AT+CEREG?");
        regCheckWorking = _parser.recv("+CEREG:%d,%d\n",&_state.reg_en, &_state.reg_stat) && _parser.recv("OK\n");
        //
        //QUECTEL:
        //0 Not registered, MT is not currently searching an operator to register to
        //1 Registered, home network
        //2 Not registered, but MT is currently trying to attach or searching an operator to register
        //
        BC95_INFO(">>>>_state.reg_en = %d, _state.reg_stat = %d", _state.reg_en, _state.reg_stat);
        if(_state.reg_stat == 1) {
            break;
        }
        wait(1);
    }

    if(_state.reg_stat != 1 || !regCheckWorking) {
        BC95_ERROR("EPS Network Registration Fail!");
        _parser.flush();
        _serialMutex.unlock();
        return false;
    }

    _serialMutex.unlock();

    BC95_INFO("BC95 CONNECT, End & Success ! <<<<");
    BC95_INFO("======================================================\n");

    return true;
}

bool BC95::disconnect(void)
{
    bool ret;
    BC95_INFO(">>>>BC95:: disconnect");

    _serialMutex.lock();

    setTimeout(_BC95_MISC_TIMEOUT);

    _parser.send("AT+CGATT=0");
    ret = _parser.recv("OK\n");

    if (_plmn) {
        if (!(_parser.send("AT+COPS=2,2,\"%d\"",_plmn) && _parser.recv("OK\n"))) {
            BC95_ERROR("BC95 deregister from network failing");
            ret = false;
            _parser.flush();
        }
    }

    _serialMutex.unlock();

    return ret;
}


const char *BC95::getIPAddress(void)
{
    int cid;
    char isItcomma;

    _serialMutex.lock();

    // TBD. Currently get the device ip from the default cid:0, consider PDP in the future
    _parser.send("AT+CGPADDR");
    _parser.recv("+CGPADDR:%d%c", &cid, &isItcomma);
    if (isItcomma == ',') {
        //there will be an IP address, if the response includes a comma
        _parser.recv("%15[^\n]\n", _ip);
    }

    if (_parser.recv("OK\n")) {
        BC95_INFO(">>>>BC95:: GET IP: %s", _ip);
    } else {
        BC95_ERROR("BC95 failed to get the IP address");
        _ip[0] = '\0';
    }

    _serialMutex.unlock();

    return _ip;
}


const char *BC95::getIMSI()
{
    // International mobile subscriber identification    

    int imsiTrials = 0;
    wait(BC95_WAIT_FOR_IMSI_SECS);

    _serialMutex.lock();
    while (!(_parser.send("AT+CIMI") && _parser.recv("%15[^\n]\nOK\n", _imsi))) {
        BC95_ERROR("Reading IMSI failed");
        imsiTrials++;
        if (imsiTrials >= BC95_MAX_IMSI_READ_RETRY) {
            break;
        }
        _parser.flush();
        wait(BC95_WAIT_FOR_IMSI_SECS);
    } 
    _serialMutex.unlock();

    if (imsiTrials < BC95_MAX_IMSI_READ_RETRY) {
        BC95_INFO("BC95 IMSI %s",_imsi);
    } else {
        _imsi[0] = '\0';
    }        


    return _imsi;
}


const char *BC95::getMACAddress(void)
{
    /* TBD. No mac address.*/
    BC95_ERROR(">>>>BC95:: TBD. Not support getMACAddress yet!");
    return NULL;
}

const char *BC95::getGateway()
{
    /* TBD. No mac address.*/
    BC95_ERROR(">>>>BC95:: TBD. Not support getGateway yet!");
    return NULL;
}


const char *BC95::getNetmask()
{
    /* TBD. No mac address.*/
    BC95_ERROR(">>>>BC95:: TBD. Not support getNetmask yet!");
    return NULL;
}

int8_t BC95::getRSSI()
{

    /* Use AT+CSQ to query current signal quality */
    /*
    <rssi> Integer type
    0 -113dBm or less
    1 -111dBm
    2...30 -109dBm... -53dBm
    31 -51dBm or greater
    99 Not known or not detectable
    <ber> Integer type; channel bit error rate (in percent)
    0...7 As RXQUAL values (please refer to 3GPP specifications)
    99 Not known or not detectable
    */

    _serialMutex.lock();

    _parser.send("AT+CSQ");
    bool rssiCheckWorking = _parser.recv("+CSQ:%d,%d\n", &_signal.rssi, &_signal.ber) && _parser.recv("OK\n");
    if (!rssiCheckWorking) {
        BC95_ERROR("BC95 no response to RSSI check!");
        _parser.flush();
        _serialMutex.unlock();
        return 0;
    }

    _serialMutex.unlock();

    /* TBD. QUECTEL: BER is currently not implemented, and will always be 99 */
    BC95_INFO(">>>>BC95::getRSSI signal rssi = %d", _signal.rssi);

    if(_signal.rssi == 99) {
        BC95_ERROR("BC95 signal is unknow or not detectable!");
    }
    if(_signal.rssi <20) {
        BC95_INFO(">>>>BC95 signal is below -73dBm which is very weak!");
    }

    return (int8_t)(_signal.rssi&&0xff);
}

bool BC95::isConnected(void)
{
    /* TBD. Reactive the connection here ?
    Active (CONNECT), Standby (IDLE), Deep-sleep (PSM)
    */

    /* TBD. Need to check CGATT and CFUN ? */

    /*Use AT+CEREG? to query current EPS Network Registration Status*/
    /*
    [2017-07-13 20:09:55:270_S:] AT+CEREG?
    [2017-07-13 20:09:55:284_R:]
    [2017-07-13 20:09:55:287_R:] +CEREG:1,1
    [2017-07-13 20:09:55:300_R:]
    [2017-07-13 20:09:55:301_R:] OK
    */
    _serialMutex.lock();

    _parser.send("AT+CEREG?");
    bool regCheckWorking = _parser.recv("+CEREG:%d,%d\n",&_state.reg_en, &_state.reg_stat) && _parser.recv("OK\n");

    if (regCheckWorking) {
        BC95_INFO(">>>>BC95::isConnected _state.reg_en = %d, _state.reg_stat = %d", _state.reg_en, _state.reg_stat);

        if(_state.reg_stat == 1) {
            _serialMutex.unlock();
            return true;
        }
    } else {
        BC95_ERROR("BC95 could not read registration status!");
        _parser.flush();
    }

    _serialMutex.unlock();

    return false;
}

nsapi_error_t BC95::open(const char *type, nsapi_socket_t *socket_handle)
{
    /* TBD. Quectel:
    A maximum of 7 sockets are supported, but other services may reduce this number.
    Only UDP protocol 17, is supported.
    <type>=RAW and <protocol>=6 will be accepted, but are not supported and should not be used.
    <listen port> cannot be set as 5683 for B656SP2 version or later.
    */

    /* TBD. ignore "type" since only support UDP now, to be decided if need to support COAP in the future */

    // Look for an unused socket
    int index = -1;
    for (int i = 0; i < BC95_SOCKET_MAX; i++) {
        if (!_socket[i]) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return NSAPI_ERROR_NO_SOCKET;
    }

    // create local socket structrue, socket on modem is created when app calls sendto/recvfrom
    _socket[index] = new struct Socket_S;
    Socket_S *psock;
    psock = _socket[index];
    memset(psock, 0, sizeof(struct Socket_S));
    psock->localPort = UDP_SOCKET_PORT;
    *socket_handle = psock;

    return NSAPI_ERROR_OK;
}

nsapi_error_t BC95::_create_socket(struct Socket_S *socket)
{
    int sock_id;

    _serialMutex.lock();

    // Protocol should be 17, the port use fixed 4587 right now, need to support other settings.
    bool socketOpenWorking = _parser.send("AT+NSOCR=DGRAM,17,%d,1", socket->localPort) &&
                             _parser.recv("%d\nOK\n", &sock_id);

    if (!socketOpenWorking) {
        _parser.send("AT+NSOCL=%d",0);
        _parser.recv("OK\n");
        socketOpenWorking = _parser.send("AT+NSOCR=DGRAM,17,%d,1", socket->localPort) &&
                            _parser.recv("%d\nOK\n", &sock_id);
    }

    if (!socketOpenWorking) {
        BC95_ERROR("BC95 failed to create a socket!");
        _parser.flush();
        _serialMutex.unlock();
        return NSAPI_ERROR_NO_SOCKET;
    }

    _serialMutex.unlock();

    // Check for duplicate socket id delivered by modem
    for (int i = 0; i < BC95_SOCKET_MAX; i++) {
        Socket_S *sock = _socket[i];
        if (sock && sock->created && sock->sock_id == sock_id) {
            BC95_ERROR("BC95 socket id is a duplicate");
            return NSAPI_ERROR_NO_SOCKET;
        }
    }

    socket->sock_id = sock_id;
    socket->created = true;


    return NSAPI_ERROR_OK;
}

void BC95::socket_connect(nsapi_socket_t handle, const char *ip_address, int port)
{
    struct Socket_S *socket = (struct Socket_S *)handle;
    if (ip_address) {
        socket->remote_port = port;
        strcpy(socket->remote_ip, ip_address);
        socket->connected = true;
    }
}

nsapi_size_or_error_t BC95::sendto(nsapi_socket_t handle, const char* addr, int port, const void *data, int size)
{
    struct Socket_S *socket = (struct Socket_S *)handle;

    if (!socket->created) {
        nsapi_error_t ret_val = _create_socket(socket);
        if (ret_val != NSAPI_ERROR_OK) {
            return ret_val;
        }
    }

    if (!addr) {
        if (!socket->connected) {
            return NSAPI_ERROR_DEVICE_ERROR;
        }
        addr = socket->remote_ip;
        port = socket->remote_port;
    }

    /* Check parameters */
    if (addr == NULL || sizeof(addr) > 14) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    if (size > BC95_MAX_PACKAGE_SIZE) {
        BC95_ERROR("sendto: data size bigger than maximum of %d", BC95_MAX_PACKAGE_SIZE);
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    int sent_len = 0;
    int sock_id = socket->sock_id;
    char hexstr[BC95_MAX_PACKAGE_SIZE*2 + 1] = {0};

    _strparser.ConverToHexString(hexstr, (const char*)data, size);

    _serialMutex.lock();

    setTimeout(_BC95_SEND_TIMEOUT);

    if ((!_parser.send("AT+NSOST=%d,%s,%d,%d,%s", sock_id, addr, port, size, hexstr))
            ||(!_parser.recv("%d,%d\n", &sock_id, &sent_len))
            ||(!_parser.recv("OK\n"))) {
        _parser.flush();
        _serialMutex.unlock();
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    _serialMutex.unlock();

    return sent_len;
}

void BC95::rx_msg_ind()
{
    int sock_id=0, readable_len=0;

    // _parser is locked already on caller
    (void) _parser.recv("%d,%d\n", &sock_id, &readable_len);

    for (int i = 0; i < BC95_SOCKET_MAX; i++) {
        Socket_S *sock = _socket[i];
        if (sock && sock->sock_id == sock_id) {
            if (sock->_cb) {
                sock->_cb(sock->_data);
            }
            break;
        }
    }
}

nsapi_size_or_error_t BC95::_read_packet(int sock_id, void *data, int size, char *recv_ip, int *recv_port)
{
    int recv_len=0;
    int recv_sock_id,recv_remaining;
    char hexstr[BC95_MAX_PACKAGE_SIZE*2 + 1];

    _serialMutex.lock();

    setTimeout(_BC95_RECV_TIMEOUT);

    if ((!_parser.send("AT+NSORF=%d,%d",sock_id,size))
            ||(!_parser.recv("%d,%[^,],%d,%d,",&recv_sock_id, recv_ip, recv_port, &recv_len))
            ||((recv_len == 0 && !_parser.recv(",%d\n",&recv_remaining)) || (recv_len > 0 && !_parser.recv("%[^,],%d\n",hexstr, &recv_remaining)))
            ||(!_parser.recv("OK\n"))) {
        // we also get here when there is no more packets
        //BC95_ERROR("_read_packet: Fetch data AT fail!");
        _parser.flush();
        _serialMutex.unlock();
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    _serialMutex.unlock();

    if(recv_len > 0) {
        _strparser.HexStringToString((unsigned char*) data, recv_len, (const char*) hexstr);
    }

    return recv_len;
}

nsapi_size_or_error_t BC95::recvfrom(nsapi_socket_t handle, void *buffer, int buffer_size, char *ip_address, int *port)
{
    struct Socket_S *socket = (struct Socket_S *)handle;

    int ret_val = NSAPI_ERROR_OK;

    if (!socket->created) {
        ret_val = _create_socket(socket);
        if (ret_val != NSAPI_ERROR_OK) {
            return ret_val;
        }
    }

    if (buffer_size < BC95_MAX_PACKAGE_SIZE) {
        BC95_ERROR("NOT ENOUGH BUFFER FOR A MAX LENGTH MESSAGE OF %d BYTES.", BC95_MAX_PACKAGE_SIZE);
    }

    // Read packet -> AT+NSORF
    ret_val = _read_packet(socket->sock_id, buffer, buffer_size, ip_address, port);

    return ret_val;
}

nsapi_error_t BC95::bind(nsapi_socket_t handle, uint16_t port)
{
    struct Socket_S *socket = (struct Socket_S *)handle;

    socket->localPort = port;

    if (!socket->created) {
        nsapi_error_t ret_val = _create_socket(socket);
        if (ret_val != NSAPI_ERROR_OK) {
            return ret_val;
        }
    }

    return NSAPI_ERROR_OK;
}

nsapi_error_t BC95::close(nsapi_socket_t handle)
{
    struct Socket_S *socket = (struct Socket_S *)handle;
    int sock_id = socket->sock_id;
    nsapi_error_t ret_val = NSAPI_ERROR_DEVICE_ERROR;
    BC95_INFO(">>>>BC95::close start => %d!", sock_id);

    int index = -1;
    for (int i = 0; i < BC95_SOCKET_MAX; i++) {
        if (_socket[i] && _socket[i]->sock_id == sock_id) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        _socket[index] = NULL;
        ret_val = NSAPI_ERROR_OK;
    }

    _serialMutex.lock();

    setTimeout(_BC95_MISC_TIMEOUT);

    if (!(_parser.send("AT+NSOCL=%d", sock_id) && _parser.recv("OK\n"))) {
        ret_val = NSAPI_ERROR_DEVICE_ERROR;
    }

    _serialMutex.unlock();

    delete socket;
    BC95_INFO(">>>>BC95::close end => %d!", sock_id);
    return ret_val;
}


void BC95::setTimeout(int timeout_ms)
{
    if(_parser_timeout != timeout_ms) {
        _parser.set_timeout(timeout_ms);
        _parser_timeout = timeout_ms;
        //BC95_INFO("###TIMEOUT %d####\n\n",timeout_ms);
    }
}

void BC95::socket_attach(nsapi_socket_t handle, void (*cb)(void *), void *data)
{
    Socket_S *socket = (Socket_S *)handle;
    socket->_cb = cb;
    socket->_data = data;

/* not needed due to handle_event() is polling
    _serial.sigio(callback(cb, data));
*/

}

void BC95::handle_event()
{
    pollfh fhs;
    int count;

    fhs.fh = &_serial;
    fhs.events = POLLIN;

    while (true) {
        count = poll(&fhs, 1, -1);
        if (count > 0 && (fhs.revents & POLLIN)) {
            _serialMutex.lock();
            setTimeout(10);
            (void)_parser.recv(UNNATURAL_STRING);
            setTimeout(_BC95_MISC_TIMEOUT);
            _serialMutex.unlock();
        }
    }
}
