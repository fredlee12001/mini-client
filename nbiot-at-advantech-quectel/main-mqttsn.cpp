/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/
 
 /**
  This is a sample program to illustrate the use of the MQTTSN Client library
  on the mbed platform.  The Client class requires two classes which mediate
  access to system interfaces for networking and timing.  As long as these two
  classes provide the required public programming interfaces, it does not matter
  what facilities they use underneath. In this program, they use the mbed
  system libraries.
 
 */
#include "config.h"

#ifdef TEST_MQTTSN

 #define WARN printf

#if defined(TARGET_ADV_WISE_1570) || defined (TARGET_MCU_K64F)
#include "BC95Interface.h"

#ifdef TARGET_MCU_K64F
#define UART_TX_BC95 PTC17
#define UART_RX_BC95 PTC16
#else
#define UART_TX_BC95 PC_1
#define UART_RX_BC95 PC_0
#endif

//
// SIM pin code and network credentials
//
#define PIN_CODE    "1234"
#define APN			"APN"
#define USERNAME	"test"
#define PASSWORD	"test"

// 
// Number of retries
//
#define RETRY_COUNT 3

//
// Network interface for BC95
//
static BC95Interface net(UART_TX_BC95, UART_RX_BC95, true);

#else
#include "EthernetInterface.h"
#include "C12832.h"

#if defined(TARGET_LPC1768)
#warning "Compiling for mbed LPC1768"
#include "LPC1768.h"
#elif defined(TARGET_K64F)
#warning "Compiling for mbed K64F"
#include "K64F.h"
#endif
#endif // TARGET_ADV_WISE_1570

#define MQTTSNCLIENT_QOS2 1
#include "MQTTSNUDP.h"
#include "MQTTSNClient.h"

int arrivedcount = 0;

void messageArrived(MQTTSN::MessageData& md)
{
    MQTTSN::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d \n\r", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s \n\r", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

#if defined(TARGET_ADV_WISE_1570) || defined (TARGET_MCU_K64F)
nsapi_error_t do_connect()
{
    nsapi_error_t retcode;
    uint8_t retry_counter = 0;

    while (!net.is_connected()) {

        printf("\n\nTry to connect\n\r");
        retcode = net.connect();
        printf("\n\nConnect %d\n\r", retcode);
        if (retcode == NSAPI_ERROR_AUTH_FAILURE) {
            printf("\n\nAuthentication Failure. Exiting application\n\r");
            return retcode;
        } else if (retcode != NSAPI_ERROR_OK) {
            printf("\n\nCouldn't connect: %d, will retry\n\r", retcode);
            retry_counter++;
            continue;
        } else if (retcode != NSAPI_ERROR_OK && retry_counter > RETRY_COUNT) {
            printf("\n\nFatal connection failure: %d\n\r", retcode);
            return retcode;
        }

        break;
    }

    printf("\n\nConnection Established.\n\r");

    return NSAPI_ERROR_OK;
}
#endif // TARGET_ADV_WISE_1570   

static MQTTSNUDP ipstack = MQTTSNUDP();
static MQTTSN::Client<MQTTSNUDP, Countdown> client = MQTTSN::Client<MQTTSNUDP, Countdown>(ipstack);

int main(int argc, char* argv[])
{   
    float version = 0.47;
    char* topic = "mbed-sample";
	char* hostname = SERVER_IP_ADDR;
    int port = UDP_SOCKET_PORT;
	int rc;
    
    printf("Version is %f \n\r", version);
             
#if defined(TARGET_ADV_WISE_1570) || defined (TARGET_MCU_K64F)
    //
    // Set Pin code for SIM card and network credentials
    //
    net.set_sim_pin(PIN_CODE);
	net.set_credentials(APN, USERNAME, PASSWORD);

    //
    // Attempt to connect to a cellular network
    //
    printf("\n\rConnecting for NB-IoT...\n\r");
    if (do_connect() != NSAPI_ERROR_OK) {
		printf("\n\rFailure. Exiting \n\n\r");
		return -1;
    }

	printf("Connecting to %s:%d\n\r", hostname, port);
	rc = ipstack.connect(hostname, port, &net);
#else
	EthernetInterface eth;
    eth.init();                          // Use DHCP
    eth.connect();

	printf("Connecting to %s:%d\n\r", hostname, port);
	rc = ipstack.connect(hostname, port);
#endif // TARGET_ADV_WISE_1570
 
    if (rc != 0) {
        printf("rc from UDP connect is %d \n\r", rc);
    } else {
		printf("rc from UDP connect is OK \n\r");
    } 
	wait(3);
    MQTTSNPacket_connectData data = MQTTSNPacket_connectData_initializer;       
    data.clientID.cstring = "mbed-sample";
    data.duration = 60;
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d \n\r", rc);
    
    MQTTSN_topicid topicid;
    topicid.type = MQTTSN_TOPIC_TYPE_NORMAL;
    topicid.data.long_.name = topic;
    topicid.data.long_.len = strlen(topic);
    MQTTSN::QoS grantedQoS;
    if ((rc = client.subscribe(topicid, MQTTSN::QOS1, grantedQoS, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d \n\r", rc);

    MQTTSN::Message message;

    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f \n\r", version);
    message.qos = MQTTSN::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topicid, message);
    while (arrivedcount < 1)
        client.yield(100);
        
    // QoS 1
    sprintf(buf, "Hello World!  QoS 1 message from app version %f \n\r", version);
    message.qos = MQTTSN::QOS1;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topicid, message);
    while (arrivedcount < 2)
        client.yield(100);
        
    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app version %f \n\r", version);
    message.qos = MQTTSN::QOS2;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topicid, message);
    while (arrivedcount < 3)
        client.yield(100);
        
    // n * QoS 2
    for (int i = 1; i <= 10; ++i)
    {
        sprintf(buf, "Hello World!  QoS 2 message number %d from app version %f \n\r", i, version);
        message.qos = MQTTSN::QOS2;
        message.payloadlen = strlen(buf)+1;
        rc = client.publish(topicid, message);
        while (arrivedcount < i + 3)
            client.yield(100);
    }
    
    if ((rc = client.unsubscribe(topicid)) != 0)
        printf("rc from unsubscribe was %d \n\r", rc);
    
    if ((rc = client.disconnect()) != 0)
        printf("rc from disconnect was %d \n\r", rc);
    
    ipstack.disconnect();
    
    printf("Finishing with %d messages received \n\r", arrivedcount);
   
    return 0;
}

#endif // TEST_MQTTSN
