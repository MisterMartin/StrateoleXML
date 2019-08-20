/*
 *  XMLWriter_v5.h
 *  XML writer specifically designed for communicating with
 *	Zephyr Iridium Modem
 *  Author: Marika Schubert
 *  January 2017
 *
 *  Adapted: Alex St. Clair (v4, v5)
 *  August 2019
 *
 *  This library will allow devices on the Strateole project
 *	to write specific message types in XML to the Zephyr Gondola.
 *	This file will write the Tag->Tag contents of a message to a
 *	character output buffer, run the CRC check on this buffer, then
 * 	send the buffer with the proper CRC.
 *
 *  * Version 3 adds DIB/MCB Communications
 *  * Version 4 updates the interface for StratoCore
 *  * Version 5 removes DIB/MCB Communications
 *
 *  This code has the following dependencies:
 *
 *  "CircularBuffer.h"
 *	"Arduino.h"
 *    Time.h"
 *    #<BitPacking.h>
 */

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "Arduino.h"
#include "Time.h"
#include <BitPacking.h>
#include <CircularBuffer.h>
//#include "FloatsHealth.h"

#define TXBUFSIZE (865) // TODO: resize
#define LOG

enum StateFlag_t {
    UNKN,
    FINE,
    WARN,
    CRIT,
    NOMESS
};

class XMLWriter {
public:
#ifdef LOG
    XMLWriter(Print* stream, Print* log);
#else
    XMLWriter(Print* stream);
#endif

    void reset();

    //Call to set names of state flags
    void setStateFlags(uint8_t num, String flag);

    //Call to set values of state flags
    void setStateFlagValue(uint8_t num, StateFlag_t stat);

    //Call to set value of details
    void setStateDetails(uint8_t num, String details);

    // <tag>
    void tagOpen(const char* tag);
    // </tag>
    void tagClose(const char* tag);

    // <tag>value</tag>
    void writeNode(const char* tag, const char* value); //string

    void writeNode(const char* tag, String value);

    void writeNode(const char* tag, char* value, uint8_t length); //char array

    void writeNode(const char* tag, uint8_t value);

    void writeNode(String tag, String value);

    void writeNode(String tag, const char* value);

    void sendBuf(); // Sends the contents of the data buffer

    // Reset crc calculation
    void crcReset();

    // Returns current crc
    uint16_t crcValue();

    // Updates crc with new byte
    uint16_t crcUpdate(uint8_t data);

    // Updates crc with multiple bytes
    uint16_t crcUpdate(uint8_t* data, uint8_t length);

    // Updates crc with string literal
    uint16_t crcUpdate(const char* data);

    // Writes crc node and resets crc value
    void writeCRC();

    // Sends Msg node
    uint16_t msgNode();

    // Sends Inst Node
    void instNode();

    //Send IMR
    void IMR();

    //Send IM Ack
    void IMAck(uint8_t ackval);

    // Instrument has reached safety mode
    void S();

    // Rachuets deploy request will return without sending
    // for the wrong device
    void RA();

    // Telemetry packet
    void TM(uint8_t* bin, uint16_t len); // Not functional
    void TM();

    void TM_String(StateFlag_t state_flag, const char * message);

    // Housekeeping Telemetry packet
    void TMhouse();

    // Creates and wraps binary section
    // Called from TM
    void sendBin();

    // sends an empty binary section
    void sendEmptyBin();

    // Telecommand ackval
    void TCAck(uint8_t ackval);

    // Sets the local device name
    void setDevId(String id);

    // Interacting with telemetry buffer
    uint8_t addTm(uint8_t inChar);
    uint8_t addTm(uint16_t inWord);
    uint8_t addTm(uint32_t inDouble);
    uint8_t addTm(String inStr);
    uint8_t addTmTemp(float tempFloat);
    uint8_t addTmGPS(float gpsFloat);
    uint8_t addTmVolt(uint16_t voltInt);
    uint8_t remTm();
    void clearTm();
    uint16_t getTmLen();

private:
    //builds TM message body
    void sendTMBody();

    // outputstream
    Print* _stream;

    Print* _log;

    uint16_t rx_crc;

    String devId;

    String StateFlag1 = "StateFlag1";
    String StateFlag2 = "StateFlag2";
    String StateFlag3 = "StateFlag3";

    StateFlag_t flag1 = FINE;
    StateFlag_t flag2 = NOMESS; //Only the first one is mandatory
    StateFlag_t flag3 = NOMESS;

    String details1 = "";
    String details2 = "";
    String details3 = "";

    CircularBuffer<uint8_t, TXBUFSIZE> tmbuf; // Buffer for outgoing telemetry data

    uint16_t tmbuf_len = 0;
};

#endif
// END OF FILE