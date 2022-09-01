#ifndef CanNetwork_h
#define CanNetwork_h

#include "Arduino.h"
#include <SPI.h>
#include "mcp_can_2.h"
#include "CanPacket.h"
#include <FlexCAN_T4.h>
class CanNetwork
{
public:
    CanNetwork(int pin);
    bool init(uint8_t speed); //return FALSE if NOT FAILED, return TRUE if FAILED
    void send(CanPacket* packet);
    void debug();
    CanPacket receive();
    void loopback();
    void initFlexcan(uint32_t baud);
    void setTime(uint64_t unix_time_seconds);
    CanPacket receiveFlexcan();
    MCP_CAN *_CAN;
private:
    bool _debug;
    CanPacket _sent;
    uint64_t millis_absolute_time_offset;
};

#endif
