#if !defined(MQTTSOCKET_H)
#define MQTTSOCKET_H

#include "MQTTmbed.h"
#include "UDPSocket.h"

#if defined(TARGET_ADV_WISE_1570) || defined (TARGET_MCU_K64F)
UDPSocket mysock;

class MQTTSNUDP
{
public:    
    int connect(char* hostname, int port, NetworkStack *net, int timeout=1000)
    {
        net->gethostbyname((const char *)hostname, &remote);
        remote.set_port(port);
        return mysock.open(net);
    }

    int read(unsigned char* buffer, int len, int timeout)
    {
        mysock.set_blocking(false); 
        return mysock.recvfrom(&remote, (void*)buffer, len);
    }
    
    int write(unsigned char* buffer, int len, int timeout)
    {
        mysock.set_blocking(false);  
        return mysock.sendto(remote, (void*)buffer, len);
    }
    
    int disconnect()
    {
        return mysock.close();
    }
    
private:  
    //UDPSocket mysock; // Comment for ignored compilation issue
    SocketAddress remote;
    
};

#else

class MQTTSNUDP
{
public:
    int connect(char* hostname, int port, int timeout=1000)
    {
        mysock.init();
        //mysock.set_blocking(false, timeout);    // 1 second Timeout 
        return remote.set_address((const char *)hostname, port);
    }

    int read(unsigned char* buffer, int len, int timeout)
    {
        mysock.set_blocking(false, timeout);  
        return mysock.receiveFrom(remote, (char *)buffer, len);
    }
    
    int write(unsigned char* buffer, int len, int timeout)
    {
        mysock.set_blocking(false, timeout);  
        return mysock.sendTo(remote, (char*)buffer, len);
    }
    
    int disconnect()
    {
        return mysock.close();
    }
    
private:

    UDPSocket mysock;
    Endpoint remote;
    
};

#endif // TARGET_ADV_WISE_1570

#endif
