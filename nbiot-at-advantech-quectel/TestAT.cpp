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

#include <TestAT.h>
#include "config.h"

static int stressLoop = 1;

#define _BC95_CONNECT_DELIM         "\r\n"
#define _BC95_CONNECT_BUFFER_SIZE   1024
#define _BC95_CONNECT_TIMEOUT       8000
#define _BC95_TEST_ERRORS           0

struct TestSeq {
    const char *command;
    const char *response;
};

#define NO_MORE_DATA "0\r\nOK\r\n"


#if _BC95_TEST_ERRORS
static TestSeq testSeqStartup[] = {
        // startup
        {"AT\r\n", "somestartgarbageOK\r\n"}, //1
        {"AT\r\n", "somestartgarbageOK\r\n"}, //2
        {"AT\r\n", "somestartgarbageOK\r\n"}, //3
        {"AT\r\n", "somestartgarbageOK\r\n"}, //4
        {"AT\r\n", "somestartgarbageOK\r\n"}, //5
        {"AT\r\n", "somestartgarbageOK\r\n"}, //6
        {"AT\r\n", "somestartgarbageOK\r\n"}, //7
        {"AT\r\n", "somestartgarbageOK\r\n"}, //8
        {"AT\r\n", "somestartgarbageOK\r\n"}, //9
        {"AT\r\n", "somestartgarbageOK\r\n"}, //10
        {"AT\r\n", "somestartgarbageOK\r\n"}, //11
        {"AT\r\n", "somestartgarbageOK\r\n"}, //12
        {"AT\r\n", "somestartgarbageOK\r\n"}, //13
        {"AT\r\n", "somestartgarbageOK\r\n"}, //14
        {"AT\r\n", "somestartgarbageOK\r\n"}, //15
        {"AT\r\n", "somestartgarbageOK\r\n"}, //16
        {"AT\r\n", "somestartgarbageOK\r\n"}, //17
        {"AT\r\n", "somestartgarbageOK\r\n"}, //18
        {"AT\r\n", "somestartgarbageOK\r\n"}, //19
        {"AT\r\n", "somestartgarbageOK\r\n"}, //20
        {"AT\r\n", "somestartgarbageOK\r\n"}, //21
        {"AT\r\n", "somestartgarbageOK\r\n"}, //22
        {"AT\r\n", "somestartgarbageOK\r\n"}, //22
        {"AT\r\n", "somestartgarbageOK\r\n"}, //23
        {"AT\r\n", "somestartgarbageOK\r\n"}, //24
        {"AT\r\n", "somestartgarbageOK\r\n"}, //25
        {"AT\r\n", "somestartgarbageOK\r\n"}, //26
        {"AT\r\n", "somestartgarbageOK\r\n"}, //27
        {"AT\r\n", "somestartgarbageOK\r\n"}, //28
        {"AT\r\n", "somestartgarbageOK\r\n"}, //29
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "garbage\r\n"},
        {"AT\r\n",  "OK\r\n"},
        {"AT+NBAND?\r\n", "+NBAND:8\r\nOK\r\n"},
        {"AT+NBAND=20\r\n", "OK\r\n"},
        {"AT+NCONFIG?\r\n", "+NCONFIG:AUTOCONNECT,TRUE\r\n+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE\r\n+NCONFIG:CR_0859_SI_AVOID,TRUE\r\nOK\r\n"},
#ifdef DEBUG_TEST_APN_PLMN
        {"AT+NCONFIG=AUTOCONNECT,FALSE\r\n","OK\r\n"},
#endif
        {"AT+NRB\r\n", "REBOOTING\r\n"},
        {"AT\r\n",  "OK\r\n"},
        {"AT+NBAND?\r\n",  "+NBAND:20\r\nOK\r\n"},
        {"AT+NCONFIG?\r\n", "garbage\r\n"},
        {"AT+NRB\r\n", "REBOOTING\r\n"},
        {"AT\r\n",  "OK\r\n"},
        {"AT+NBAND?\r\n",  "+NBAND:20\r\nOK\r\n"},
#ifdef DEBUG_TEST_APN_PLMN
        {"AT+NCONFIG?\r\n", "+NCONFIG:AUTOCONNECT,FALSE\r\ngarbage\r\n"},
#else
        {"AT+NCONFIG?\r\n", "+NCONFIG:AUTOCONNECT,TRUE\r\ngarbage\r\n"},
#endif
        {NULL, NULL},
};

static TestSeq testSeqConnect[] = {
        {"AT+CFUN?\r\n",  "+CFUN:0\r\nOK\r\n"},
        {"AT+CFUN=1\r\n",  "OK\r\n"},
        {"AT+CFUN?\r\n",  "+CFUN:1\r\nOK\r\n"},
#ifdef DEBUG_TEST_APN_PLMN
#if DEBUG_TEST_APN_PLMN>0
        {"AT+COPS=1,2,\"50001\"\r\n",  "OK\r\n"},
#endif
#if DEBUG_TEST_APN_PLMN!=1
        {"AT+CGDCONT=0,\"IP\",\"www.example.com\"\r\n",  "OK\r\n"},
#endif
#endif
        {"AT+CGATT?\r\n",  "+CGATT:1\r\ngarbage\r\n"},
        {"AT+CGATT=1\r\n",  "garbage\r\n"},
        {"AT+CGATT=1\r\n",  "garbage\r\n"},
        {"AT+CGATT?\r\n",  "+CGATT:1\r\nOK\r\n"},
        {"AT+CGATT=1\r\n",  "OK\r\n"},
        {"AT+CGATT?\r\n",  "+CGATT:0\r\nOK\r\n"},
        {"AT+CGATT?\r\n",  "+CGATT:0\r\nOK\r\n"},
        {"AT+CGATT?\r\n",  "+CGATT:1\r\nOK\r\n"},
        {"AT+CEREG?\r\n",  "+CEREG:200,100\r\nOK\r\n"},
        {"AT+CEREG?\r\n",  "+CEREG:0,0\r\nOK\r\n"},
        {"AT+CEREG?\r\n",  "+CEREG:0,1\r\nOK\r\n"},
        // getIPAddress
        {"AT+CGPADDR\r\n",  "+CGPADDR:1,192.53.100.53\r\nOK\r\n"},
        {NULL, NULL},
};


#else


static TestSeq testSeqStartup[] = {
        {"AT\r\n", "somestartgarbageOK\r\n"},
        {"AT\r\n", "OK\r\n"},
        {"AT+NBAND?\r\n", "+NBAND:8\r\nOK\r\n"},
        {"AT+NBAND=20\r\n", "OK\r\n"},
        {"AT+NCONFIG?\r\n", "+NCONFIG:AUTOCONNECT,FALSE\r\n+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE\r\n+NCONFIG:CR_0859_SI_AVOID,TRUE\r\nOK\r\n"},
        {"AT+NCONFIG=AUTOCONNECT,TRUE\r\n", "OK\r\n"},
        {"AT+NRB\r\n", "REBOOTING"},
        {"AT\r\n",  "OK\r\n"},
        {"AT+NBAND?\r\n",  "+NBAND:20\r\nOK\r\n"},
        {"AT+NCONFIG?\r\n", "+NCONFIG:AUTOCONNECT,TRUE\r\n+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE\r\n+NCONFIG:CR_0859_SI_AVOID,TRUE\r\nOK\r\n"},
        {NULL, NULL},
};


static TestSeq testSeqConnect[] = {
        {"AT+CFUN?\r\n",  "+CFUN:1\r\nOK\r\n"},
        {"AT+CGATT?\r\n",  "+CGATT:1\r\nOK\r\n"},
        {"AT+CEREG?\r\n",  "+CEREG:0,1\r\nOK\r\n"},
        // getIPAddress
        {"AT+CGPADDR\r\n",  "+CGPADDR:1,192.53.100.53\r\nOK\r\n"},
        {NULL, NULL},
};
#endif


static TestSeq testSeqOpen[] = {
        {"AT+NSOCR=DGRAM,17,58032,1\r\n",  "0\r\nOK\r\n"},
        {NULL, NULL},
};

static TestSeq testSeqSocket[] = {
        // recvfrom, sendto
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,4,31323334,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,4,31323334\r\n", "0,4\r\nOK\r\n"},
        // test NSONMI right after NSORF
        {"AT+NSORF=0,512\r\n",  "+NSONMI:0,2\r\n"},
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,1,31,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,1,31\r\n", "0,1\r\nOK\r\n"},
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,2,3132,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,2,3132\r\n", "0,2\r\nOK\r\n"},
        // zero length packet
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,0,,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,0,\r\n", "0,0\r\nOK\r\n"},
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,512,"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "313233343132333431323334,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,512,"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "3132333431323334313233343132333431323334313233343132333431323334313233343132333431323334313233343132"
         "3334313233343132333431323334313233343132333431323334313233343132333431323334313233343132333431323334"
         "313233343132333431323334\r\n", "0,512\r\nOK\r\n"},
        {NULL, NULL},
};

static TestSeq testSeqInd[] = {
        {"AT+NSORF=0,512\r\n", NO_MORE_DATA}, // no data
        {NULL,  "+NSONMI:0,1\r\n"},
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,1,31,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,1,31\r\n", "0,1\r\nOK\r\n"},
        {NULL, NULL},
};

static TestSeq testSeqClose[] = {
        {"AT+NSORF=0,512\r\n",  "0,192.53.100.53,58032,4,51554954,0\r\nOK\r\n"},
        {"AT+NSOST=0,192.53.100.53,58032,4,51554954\r\n", "0,1\r\nOK\r\n"},
        {"AT+NSOCL=0\r\n", "OK\r\n"},
        {NULL, NULL},
};

static TestSeq testSeqDisconnect[] = {
        {"AT+CGATT=0\r\n", "OK\r\n"},
        {NULL, NULL},
};

static TestSeq testSeqEnd[] = {
        {"AT+NSORF=0,512\r\n", NO_MORE_DATA}, // no data
        {NULL, NULL},
};


static TestSeq *testSeq = testSeqStartup;

static size_t testSeqIndex;

static const char* testSeqName;

void TestAT::nextTestSeq()
{
    if (testSeq[testSeqIndex+1].command /*|| testSeq[testSeqIndex+1].response*/) {
        testSeqIndex++;
    } else {
        if (testSeq == testSeqStartup) {
            testSeq = testSeqConnect;
            testSeqName = "Connect";
        } else if (testSeq == testSeqConnect) {
            testSeq = testSeqOpen;
            testSeqName = "Open";
        } else if (testSeq == testSeqOpen) {
            testSeq = testSeqSocket;
            testSeqName = "Socket";
        } else if (testSeq == testSeqSocket) {
            testSeq = testSeqInd;
            testSeqName = NULL; // don't print from timer callback
            // delay to start right after socket tests
            _responseTimeout.attach(Callback<void()>(this, &TestAT::rx_ind), 3);
        } else if (testSeq == testSeqInd) {
            if (--stressLoop > 0) {
                testSeq = testSeqClose;
                testSeqName = "Close";
            } else {
                testSeq = testSeqEnd;
                testSeqName = "End";
            }
        } else if (testSeq == testSeqClose) {
            testSeq = testSeqDisconnect;
            testSeqName = "Close";
        } else if (testSeq == testSeqDisconnect) {
            testSeq = testSeqStartup;
            testSeqName = "Startup";
        } else {
            printf("\r\nAT TEST: ERROR\r\n");
        }
        testSeqIndex = 0;
    }
    if (testSeqName) {
        printf("\r\n#########################################################\r\n");
        printf("AT TEST: %s, index=%d, loop=%d\r\n", testSeqName, testSeqIndex, stressLoop);
        printf("#########################################################\r\n");
    }
}

static char rxbuf[1500];

TestAT::TestAT(PinName tx, PinName rx, int baud) :
    _started(false), _readOffset(0), _writeOffset(0), _confirmed(false), _responseReadable(false)
{
    printf("\r\n\r\n########################################################################\r\n");
    printf("\r\n\r\nAT TEST: BEGIN\r\n");
}

TestAT::~TestAT()
{
}

void TestAT::start()
{
    if (!_started) {
        _started = true;
        _eventThread.start(callback(&_queue, &EventQueue::dispatch_forever));
    }
}


short TestAT::poll(short events) const {
    if ( _responseReadable ) return POLLIN | POLLOUT;
    else return POLLOUT;
}


ssize_t TestAT::read(void *buffer, size_t size)
{
    if (!_responseReadable) {
        return 0;
    }

    wait(0.01f);

    start();

    const char *response = testSeq[testSeqIndex].response;

    size_t testSeqLen = strlen(response);
    size_t len = (size <= testSeqLen - _readOffset) ? size : (testSeqLen-_readOffset);
    memcpy(buffer, &response[_readOffset], len);
    _readOffset += len;
    if (_readOffset >= testSeqLen) {
        _responseReadable = false;
    }

    if (len > 0) {
        if (_readOffset == testSeqLen-1) {
            _confirmed = true;
        }
    }

    return len;
}

ssize_t TestAT::write(const void *buffer, size_t size)
{
    wait(0.01f);

    start();
    
    if (_confirmed) {
        _confirmed = false;
        _responseReadable = false;
        printf("\r\n<<< AT RESPONSE (%d)\r\n%s\r\n", testSeqIndex, testSeq[testSeqIndex].response);
        nextTestSeq();
        _writeOffset = 0;
    }

    memcpy(&rxbuf[_writeOffset], buffer, size);
    _writeOffset += size;
    rxbuf[_writeOffset] = '\0';
    if (strncmp(testSeq[testSeqIndex].command, rxbuf, _writeOffset) != 0) {
        printf("\r\nERROR (%d):\r\nExpected: %s\r\nReceived: %s\r\n", testSeqIndex, testSeq[testSeqIndex].command, rxbuf);
    } else {
        if (_writeOffset == strlen(testSeq[testSeqIndex].command)) {
            _queue.call(Callback<void()>(this, &TestAT::command));
        }
    }

    return size;
}

off_t TestAT::seek(off_t offset, int whence)
{
    return 0;
}

int TestAT::close()
{
    return 0;
}

void TestAT::sigio(Callback<void()> eventCallback)
{
    _eventCallback = eventCallback;
}

void TestAT::rx_ind(void)
{
    /*if (_eventCallback) {
        _eventCallback();
    }*/
    //nextTestSeq();
    testSeqIndex++;
    enableReading();
    // to signal next notification
    //_responseTimeout.attach(Callback<void()>(this, &TestAT::event), 10);
}

void TestAT::command()
{
    enableReading();
    printf("\r\n>>> AT COMMAND (%d)\r\n%s\r\n", testSeqIndex, testSeq[testSeqIndex].command);
}

void TestAT::enableReading()
{
    _readOffset = 0;
    _responseReadable = true;
}
