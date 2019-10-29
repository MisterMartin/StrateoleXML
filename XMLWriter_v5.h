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
 *  * Version 5 removes DIB/MCB Communications, cleans the interface, and
 *              adds bigger, safer, more efficient bufferering
 *
 *  This code has the following dependencies:
 *
 *	"Arduino.h"
 *  "Time.h"
 *  <BitPacking.h>
 */

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "InstInfo.h"
#include "Arduino.h"
#include "Time.h"
#include <BitPacking.h>

#define TMBUF_MAXSIZE   8192
//#define LOG

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
    XMLWriter(Print* stream, Print* log, Instrument_t inst);
#else
    XMLWriter(Print* stream, Instrument_t inst);
#endif

    // Call to set names of state flags
    void setStateFlags(uint8_t num, String flag);

    // Call to set values of state flags
    void setStateFlagValue(uint8_t num, StateFlag_t stat);

    // Call to set value of details
    void setStateDetails(uint8_t num, String details);

    // Send specific messages
    void IMR();
    void S();
    void RA();
    void IMAck(bool ackval);
    void TCAck(bool ackval);

    // Telemetry packet
    void TM();
    void TM_String(StateFlag_t state_flag, const char * message);

    // Housekeeping Telemetry packet
    void TMhouse();

    // Interacting with telemetry buffer
    bool addTm(uint8_t inChar);
    bool addTm(uint16_t inWord);
    bool addTm(uint32_t inDouble);
    bool addTm(String inStr);
    bool addTm(const uint8_t * buffer, uint16_t size);
    bool addTm(const uint16_t * buffer, uint16_t size);
    bool addTmTemp(float tempFloat);
    bool addTmGPS(float gpsFloat);
    bool addTmVolt(uint16_t voltInt);
    void clearTm();
    uint16_t getTmLen();
    uint16_t getTmBuffer(uint8_t ** buffer);

private:
    void reset();

    // Reset crc calculation
    void crcReset();

    // Returns current crc
    uint16_t crcValue();

    // Write the new byte over serial, and update the CRC
    void writeAndUpdateCRC(uint8_t data);
    void writeAndUpdateCRC(uint8_t* data, uint8_t length);
    void writeAndUpdateCRC(const char* data);

    // Writes crc node and resets crc value
    void writeCRC();

    // Sends Msg node
    uint16_t msgNode();

    // Sends Inst Node
    void instNode();

    // <tag>
    void tagOpen(const char* tag);
    // </tag>
    void tagClose(const char* tag);

    // \t<field>value</field>
    void writeNode(const char* tag, const char* value); //string
    void writeNode(const char* tag, String value);
    void writeNode(const char* tag, char* value, uint8_t length); //char array
    void writeNode(const char* tag, uint8_t value);
    void writeNode(String tag, String value);
    void writeNode(String tag, const char* value);

    // Creates and wraps binary section
    // Called from TM
    void sendBin();

    // sends an empty binary section
    void sendEmptyBin();

    // builds TM message body
    void sendTMBody();

    // internal interaction with the tm buffer
    bool addTMByte(uint8_t in_byte);

    // output streams
    Print* _stream;
    Print* _log;

    // working crc to transmit for both XML and binary sections
    uint16_t tx_crc;

    // Instrument id
    Instrument_t instrument;

    // Telemetry state fields
    String StateFlag1 = "StateFlag1";
    String StateFlag2 = "StateFlag2";
    String StateFlag3 = "StateFlag3";
    StateFlag_t flag1 = FINE;
    StateFlag_t flag2 = NOMESS; //Only the first one is mandatory
    StateFlag_t flag3 = NOMESS;
    String details1 = "";
    String details2 = "";
    String details3 = "";

    // Telemetry buffer
    uint8_t tm_buffer[TMBUF_MAXSIZE];
    uint16_t num_tm_elements = 0;
    bool tm_buff_sent = false;

    uint16_t messCount = 1;

};

#endif
// END OF FILE