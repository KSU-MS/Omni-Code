
#include "CanNetwork.h"
#include "Arduino.h"

FlexCAN_T4<CAN1,RX_SIZE_256,TX_SIZE_16> FLEXCAN_;
// The pin here is the board select pin to SPI sometimes marked (SS)
// Note: you can make this any pin, as long as it's connected to the MCP chip's SS
CanNetwork::CanNetwork(int pin) : _CAN(new MCP_CAN(pin))
{
    _debug = false;
    millis_absolute_time_offset = 0;
}
void CanNetwork::debug()
{
    _debug = true;
}
void CanNetwork::initFlexcan(uint32_t baud){
    FLEXCAN_.begin();
    FLEXCAN_.setBaudRate(baud);
    FLEXCAN_.setMaxMB(8);
    for(int i = 0;i<(8);i++){
        FLEXCAN_.setMB((FLEXCAN_MAILBOX)i,RX,STD);
    }
    FLEXCAN_.mailboxStatus();
};
int ReadCAN(CAN_message_t &msg)
{
    return FLEXCAN_.read(msg);
}
bool CanNetwork::init(uint8_t speed)
{
    return (_CAN->begin(speed));
// START_INIT:
//     // Over 256 seems to break some boards
//     if (CAN_OK == _CAN->begin(speed))
//     {
//         if (_debug)
//             Serial.println("CAN BUS Shield init ok!");
//     }
//     else
//     {
//         Serial.println("CAN BUS Shield init fail");
//         Serial.println("Init CAN BUS Shield again");
//         delay(10);
        
//         goto START_INIT;
//     }
}
// This is a testing mode the mcp cards can do where they recieve everything they send (loopback)
void CanNetwork::loopback()
{
    _CAN->mcp2515_setCANCTRL_Mode(MODE_LOOPBACK);
}
// Using to save a time offset from millis so we can generate absolute timestamps
void CanNetwork::setTime(uint64_t unix_time_seconds) {
    CanPacket packet = {millis(),1,unix_time_seconds,CAN_PACKET_DELIM};
    send(&packet);
}
void CanNetwork::send(CanPacket * packet)
{
    byte sndStat = _CAN->sendMsgBuf(packet->id, 0, 8, packet->data);

    if (sndStat == CAN_OK)
    {
        if (_debug)
            Serial.println("Message Sent Successfully!");
    }
    else
    {
        Serial.println("Error Sending Message...");
    }
}
// NOTE: Timestamp is 0 if nothing was received
CanPacket CanNetwork::receive()
{

        CanPacket received = {
            timestamp: 0,
            id : _CAN->getCanId(),
            data : {0, 0, 0, 0, 0, 0, 0, 0},
            delim : CAN_PACKET_DELIM
        };
    if (CAN_MSGAVAIL == _CAN->checkReceive()) // Check if can data is avail
    {
        received.timestamp =  millis();
        uint8_t message_length_bytes;
        _CAN->readMsgBuf(&message_length_bytes, received.data); // read data,  len: data length, buf: data buf
        
        if (_debug)
        {

            Serial.print("CAN PACKET ID: ");
            Serial.print(received.id);
            Serial.print(" DATA: ");
            for (int i = 0; i < message_length_bytes; i++) // print each byte as hex
            {
                Serial.print(received.data[i], HEX);
                Serial.print("\t");
            }
            Serial.println();

            // Check for errors reported from board
            if (CAN_CTRLERROR == _CAN->checkError())
            {
                Serial.print("Recieve Error: ");
                Serial.println(_CAN->checkError(),BIN);
            }
        }
    }
        return received;
}
CanPacket CanNetwork::receiveFlexcan()
{
    CanPacket received = {
        timestamp: 0,
        id : 0,
        data : {0, 0, 0, 0, 0, 0, 0, 0},
        delim : CAN_PACKET_DELIM
    };
    CAN_message_t rxMsg;
    if (ReadCAN(rxMsg)) // Check if can data is avail
    {
        received.id = rxMsg.id;
        memcpy(received.data,rxMsg.buf,sizeof(rxMsg.buf));
        received.timestamp =  millis();        
        if (_debug)
            {
                Serial.print("CAN PACKET ID: ");
                Serial.print(received.id);
                Serial.print(" DATA: ");
                for (int i = 0; i < rxMsg.len; i++) // print each byte as hex
                {
                    Serial.print(received.data[i], HEX);
                    Serial.print("\t");
                }
                Serial.println();
                CAN_error_t error;
                if(FLEXCAN_.error(error,1)){
                    Serial.println("There was an error i guess");
                }
            }
    }
    return received;
}

