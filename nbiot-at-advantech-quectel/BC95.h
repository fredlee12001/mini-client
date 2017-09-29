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

#ifdef DEBUG_TEST_AT
#include "TestAT.h"
#endif

#ifndef BC95_H
#define BC95_H
#include "ATCmdParser.h"
#include "HexStringParser.h"
#include "platform/PlatformMutex.h"

/* TBD. Micros for Debug Only */
#define BC95_DBG 1

#define _BC95_CONNECT_DEFAULT_BAUD  9600
#define _BC95_CONNECT_DELIM         "\r\n"
#define _BC95_CONNECT_BUFFER_SIZE   (2*BC95_MAX_PACKAGE_SIZE + 80 + 80) // AT response + sscanf format
#define _BC95_CONNECT_TIMEOUT       8000
#define _BC95_CONNECT_DEBUG         false
#define _BC95_SEND_TIMEOUT          500
#define _BC95_RECV_TIMEOUT          100
#define _BC95_MISC_TIMEOUT          500
//according to the spec p31 the APN can be at max 82 chars
#define _BC95_APN_MAX_LENGTH        82 

/* TBD. Global Configurations which should be fixed once the module and driver are stable */
#define BC95_ATSYNC_RETRY 30
#define BC95_CGATT_RETRY 30
#define BC95_EPS_REG_RETRY 5    /* EPS Network Registration Statu Retry*/
#define BC95_SOCKET_MAX 7
#define BC95_MAX_IMSI_READ_RETRY 5
#define BC95_WAIT_FOR_IMSI_SECS 5

#if BC95_DBG
#define BC95_INFO(formatstring, ...)    printf("[BC95-INFOR]" formatstring "\r\n",              \
                            ## __VA_ARGS__)

#define BC95_DEBUG(formatstring, ...)   printf("[BC95-DEBUG]" formatstring " \t[%s,%d]\r\n",    \
                            ## __VA_ARGS__,                                                     \
                            __FILE__,                                                           \
                            __LINE__)

#define BC95_ERROR(formatstring, ...)   printf("[BC95-ERROR]" formatstring " \t[%s,%d]\r\n",    \
                            ## __VA_ARGS__,                                                     \
                            __FILE__,                                                           \
                            __LINE__)

/** TBD. Since Debug stage, Just use assert to block function if anything incorrect */
#define BC95_ASSERT(expr)                                   \
do{                                                        \
    if (!(expr)) {                                     \
        mbed_assert_internal(#expr, __FILE__, __LINE__);    \
    }                                                       \
} while (0)
#else
#define BC95_DEBUG(formatstring, ...)  ((void)0)    //printf (formatstring, ##__VA_ARGS__)
#define BC95_INFO(formatstring, ...)    ((void)0)   //printf (formatstring, ##__VA_ARGS__)
#define BC95_ERROR(formatstring, ...)   ((void)0)   //printf (formatstring, ##__VA_ARGS__)
#define BC95_ASSERT(expr)                                   \
do{                                                        \
    if (!(expr)) {                                          \
        mbed_assert_internal(#expr, __FILE__, __LINE__);    \
    }                                                       \
} while (0)
#endif

struct Socket_S;

/** BC95Interface class.
    This is an interface to a BC95 radio.
 */
class BC95
{
public:
    BC95(PinName tx, PinName rx, bool debug=false);

    enum Band_S {
        BAND_850MHZ_B5 = 5,     /*IBC95_B5-China, Korea*/
        BAND_900MHZ_B8 =8,      /*BC95_B8-China, Korea, Australia*/
        BAND_800MHZ_B20 =20,    /*BC95_B20-Europe*/
        BAND_700MHZ_B25 =25,    /*BC95_B28-Latin America*/
    };

    enum ErrorCode_S {
        /* GENERAL ERRORS 27.007 */
        GENERAL_OPRATION_NOT_ALLOWED = 3,     /* Operation not allowed */
        GENERAL_OPRATION_NOT_SUPPORTED = 4,   /*  Operation not supported */
        GENERAL_MEMORY_FAILURE = 23,          /*  Memory failure */
        GENERAL_NO_NETWORK_SERVICE = 30,      /*  No network service */
        GENERAL_INCORRECT_PARAMETERS = 50,    /*  Incorrect parameters */
        GENERAL_CMD_DISABLED = 51,            /* Command implemented but currently disabled */
        GENERAL_CMD_ABORTED = 52,             /* Command aborted by user */
        GENERAL_UPLINK_BUSY = 159,            /* Uplink busy/flow control */
        /* TBD.  GENERAL ERRORS 127.005
        300 ME failure
        301 SMS service of ME reserved
        302 Operation not allowed
        303 Operation not supported
        304 Invalid PDU mode parameter
        305 Invalid text mode parameter
        310 USIM not inserted
        311 USIM PIN required
        312 PH-USIM PIN required
        313 USIM failure
        314 USIM busy
        315 USIM wrong
        316 USIM PUK required
        317 USIM PIN2 required
        318 USIM PUK2 required
        320 Memory failure
        321 Invalid memory index
        322 Memory full
        330 SMSC address unknown
        331 No network service
        332 Network timeout
        340 No +CNMA acknowledgement expected
        500 Unknown error
        512 Required parameter not configured
        513 TUP not registered
        514 AT internal error
        515 CID is active
        256 Required parameter not configured
        257 TUP not registered
        */
    };

    /**
    * Startup the BC95 Module
    *
    * @param band request, defined in Band_S
    * @return true only if BC95 was setup correctly
    */
    bool startup(int band);

    /**
    * Reset BC95
    *
    * @return true only if BC95 resets successfully
    */
    bool reset(void);

    /**
    * Connect BC95 to AP.
    *
    * @param apn the name of the APn
    * @return true only if BC95 is connected successfully
    */
    bool connect(const char *apn);

    /**
    * Connect BC95 to AP Automatically
    *
    * @return true only if BC95 is connected successfully
    */
    bool connect();

    /**
    * Disconnect BC95 from AP
    *
    * @return true only if BC95 is disconnected successfully
    */
    bool disconnect(void);

    /**
    * Get the IP address of BC95
    *
    * @return null-teriminated IP address or null if no IP address is assigned
    */
    const char *getIPAddress(void);

    /**
    * Get the MAC address of BC95
    *
    * @return null-terminated MAC address or null if no MAC address is assigned
    */
    const char *getMACAddress(void);

    /** Get the local gateway
    *
    *  @return         Null-terminated representation of the local gateway
    *                  or null if no network mask has been recieved
    */
    const char *getGateway();

    /** Get the local network mask
     *
     *  @return         Null-terminated representation of the local network mask
     *                  or null if no network mask has been recieved
     */
    const char *getNetmask();

    /** Return International mobile subscriber identification
     *
     *  @return         The IMSI read from the modem
     */
    const char *getIMSI();


    /* Return RSSI for active connection
     *
     * @return      Measured RSSI
     */
    int8_t getRSSI();


    /* Set the APN
     *
     * @return      Amount of chars in the APN string, not NULL terminated
     */
    int set_credentials(const char *apn);


    /* Set PLMN
     *
     */
    void set_plmn(int plmn);


    /**
    * Check if BC95 is conenected
    *
    * @return true only if the chip has an IP address
    */
    bool isConnected(void);

    /**
    * Open a socketed connection
    *
    * @param  type the type of socket to open "UDP" or "COAP"
    * @param  socket_handle
    * @return returns 0 on success, negative error code on failure
    */
    nsapi_error_t open(const char *type, nsapi_socket_t *socket_handle);

    /**
    * Connects a socket. Sets its remote ip address and port.
    *
    * @param handle socket pointer stored in local array
    * @param ip_address
    * @param port
    * @return
    */
    void socket_connect(nsapi_socket_t handle, const char *ip_address, int port);

    /**
    * Sends data to an open socket
    *
    * @param  handle socket pointer stored in local array
    * @param  addr the IP address of the destination
    * @param  port remote port to send data to
    * @param  data data to be sent
    * @param  amount amount of data to be sent - max 1024
    * @return the number of bytes sent or error code
    */
    nsapi_size_or_error_t sendto(nsapi_socket_t handle, const char* addr, int port, const void *data, int size);

    /**
    * Receives data from an open socket
    *
    * @param  handle socket pointer stored in local array
    * @param  data placeholder for returned information
    * @param  size currently app's buffer size -> will be number of bytes to be received from
    *              +NSONMI indication when such support is added(async support)
    * @param  ip_addr address of system sending the message
    * @param  port remote port that messages was sent from
    * @return the number of bytes received or error code
    */
    nsapi_size_or_error_t recvfrom(nsapi_socket_t handle, void *data, int size, char *ip_address, int *port);

    /**
    * Closes a socket
    *
    * @param  handle socket pointer stored in local array
    * @return returns 0 on success, negative error code on failure
    */
    nsapi_error_t close(nsapi_socket_t handle);

    /**
    * Allows timeout to be changed between commands
    *
    * @param timeout_ms timeout of the connection
    */
    void setTimeout(int timeout_ms);

    /** Bind a server socket to a specific port
     *  @param handle       Socket handle
     *  @param port         Port to listen for incoming connections on
     *  @return             0 on success, negative on failure.
     */
    virtual nsapi_error_t bind(nsapi_socket_t handle, uint16_t port);

    void socket_attach(nsapi_socket_t handle, void (*callback)(void *), void *data);

private:
#ifndef DEBUG_TEST_AT
    UARTSerial _serial;
#else
    TestAT _serial;
#endif // DEBUG_TEST_AT
    ATCmdParser _parser;
    HexStringParser _strparser;
    PlatformMutex _mutex;

    int _parser_timeout;

    bool _atsync();
    bool _debugIsOn;

    nsapi_size_or_error_t _read_packet(int id, void *data, int amount, char *recv_ip, int *recv_port);

    struct Socket_S {
        // Id from modem: returned by AT+NSOCR
        int sock_id;
        // Indicates if socket is connected or not.
        // Being connected means remote ip address and port are set
        // and can be used for send and recv
        bool connected;
        // Remote ip address in case socket is connected
        // 16 - IPv4 size
        char remote_ip[16];
        // Remote port set in case socket is connected
        int remote_port;
        //mbed::Callback<void()> _cb;
        void (*_cb)(void *);
        void *_data;
        bool created;
        int localPort;
    } *_socket[BC95_SOCKET_MAX];

    struct Packet_S {
        struct Packet_S *next;
        int id;
        int received_len;
        // data follows
    } *_packets, **_packets_end;

    //BC95 variables and Structures
    struct _State_S {
        int cfun;
        int att;
        int connect_type;
        int connect_stat;
        int reg_en;
        int reg_stat;
    } _state;

    struct _Signal_S {
        int ber;    /* Not implemented, will be always 99 */
        int rssi;
    } _signal;

    struct _Config_S {
        int band;
        int autoconnect;
        int scrambling;
        int si_avoid;
    } _cfg;

    struct _Operator_S {
        int mode;
        int format;
        char id[16];
    } _op;

    char _ip[16];
    char _imsi[16];
    int _apnLength;
    char _apn[_BC95_APN_MAX_LENGTH];
    int _plmn;

    nsapi_error_t _create_socket(struct Socket_S *socket);

    Thread _event_thread;
    void handle_event();
    Mutex _serialMutex;
    void rx_msg_ind();
};

#endif
