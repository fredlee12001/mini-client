# NB-IoT demo: Advantech - Quectel

This is an example project how to connect mbed OS board and Quectel BC95 NB-IoT modem via AT serial cable.

## Board support

### ADV_WISE_1570

Wise-1570 board is supported on Advantech branch m2.com-5.5.

Onboard BC95 is connected to UART Tx/Rx pins PC_1 and PC_0.

### NUCLEO_L486RG

You can use NUCLEO_L486RG target with the patch:
git apply --directory mbed-os wise-1570-lpuart-9600.patch

Connect BC95 to UART Tx/Rx pins PC_1 and PC_0.

### K64F

K64F can be used on mbed OS official and Advantech m2.com-5.5 without modifications.

Connect BC95 to UART Tx/Rx pins PTC_17 and PTC_16.

## Integration

### mbed OS v5.5

This repository uses by default mbed OS 5.5 from Advantech branch m2.com-5.5.

### Build

Add the header and source files for BC95:

    BC95.h / BC95.cpp
    BC95Interface.h / BC95Interface.cpp
	HexStringParser.h / HexStringParser.cpp

### Configure

See config.h for configuration flags:

	#define TEST_MQTTSN // use MQTTSN main instead of UDP test server/client
    #define NBIOT_BAND (BC95::BAND_800MHZ_B20)
	#define UDP_SOCKET_PORT 58032
	#define SERVER_IP_ADDR "10.45.0.75"
    #define ASYNC_RECV // for non-blocking sockets

## Usage

### BC95Interface API

UDP socket example over BC95:

    BC95Interface net(UART_TX_BC95, UART_RX_BC95, true);
    net.connect();
    // set blocking / non-blocking socket
    UDPSocket socket;
    socket.open(&net);
	socket.recvfrom / sendto

### UDP echo client/server

Main.cpp has a simple UDP echo loop for testing socket recv/send. See config.h how to setup client or server testing. Application prints IP address / UDP port and test usage at start-up to UART for testing purposes.

### MQTTSN application

MQTTSN was integrated from https://github.com/ADVANTECH-Corp/WISE-1570-HelloMQTTSN/

MQTTSN.lib was not used due to the following changes was needed to the library:
* MQTTSN::Client was moved from stack storage to heap and max packet size was increased to 512.
* EthernetInterface.lib was removed due to that conflicted with mbed OS UDPSocket.
* Patch 0001-mqttsn-for-wise1570-mbed5.patch was applied manually.
* Initialisation of Countdown::countdown_ms was added to wait for response.

See config.h how to setup MQTTSN for testing.
