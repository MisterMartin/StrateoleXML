/*
 *  XMLWriter_v5.cpp
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
 *  "SafeBuffer.h"
 *	"Arduino.h"
 */

#include <XMLWriter_v5.h>

const uint16_t reset_crc = 0x1021;

// Constant Device Information
char swDate[] = "20170901,000000";
char swVer[] = "0.1";
char Zproto[] = "1.0";

// --------------------------------------------------------
// Constructor and reset
// --------------------------------------------------------

#ifdef LOG
XMLWriter::XMLWriter(Print* stream, Print* log, Instrument_t inst)
{
    _log = log;
    _log->println("Written messages with be logged.");
#else
XMLWriter::XMLWriter(Print* stream, Instrument_t inst)
{
#endif
    instrument = inst;
    _stream = stream;
    reset();
}

void XMLWriter::reset()
{
    tx_crc = reset_crc;
    clearTm();
}

// --------------------------------------------------------
// Telemetry state fields functions
// --------------------------------------------------------

void XMLWriter::setStateFlags(uint8_t num, String flag)
{
    if (num == 1) {
        StateFlag1 = flag;
    } else if (num == 2) {
        StateFlag2 = flag;
    } else if (num == 3) {
        StateFlag3 = flag;
    }
}

void XMLWriter::setStateFlagValue(uint8_t num, StateFlag_t stat)
{
    if (num == 1) {
        flag1 = stat;
    } else if (num == 2) {
        flag2 = stat;
    } else if (num == 3) {
        flag3 = stat;
    }
}

void XMLWriter::setStateDetails(uint8_t num, String details)
{
    if (num == 1) {
        details1 = details;
    } else if (num == 2) {
        details2 = details;
    } else if (num == 3) {
        details3 = details;
    }
}

// --------------------------------------------------------
// Tag functions
// --------------------------------------------------------

void XMLWriter::tagOpen(const char* tag)
{
    writeAndUpdateCRC('<');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC('\n');
}

void XMLWriter::tagClose(const char* tag)
{
    writeAndUpdateCRC('<');
    writeAndUpdateCRC('/');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC('\n');
}

// --------------------------------------------------------
// Overloaded node write functions
// --------------------------------------------------------

void XMLWriter::writeNode(const char* tag, const char* value)
{
    writeAndUpdateCRC('\t');
    writeAndUpdateCRC('<');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC(value);
    writeAndUpdateCRC('<');
    writeAndUpdateCRC('/');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC('\n');
}

void XMLWriter::writeNode(const char* tag, String value)
{
    writeNode(tag, value.c_str());
}

void XMLWriter::writeNode(const char* tag, char* value, uint8_t length)
{
    writeAndUpdateCRC('\t');
    writeAndUpdateCRC('<');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    for (uint8_t i = 0; i < length; i++) {
        if (value[i] != 0) {
            writeAndUpdateCRC(value[i]);
        } else {
            break;
        }
    }
    writeAndUpdateCRC('<');
    writeAndUpdateCRC('/');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC('\n');
}

void XMLWriter::writeNode(const char* tag, uint8_t value)
{
    writeAndUpdateCRC('\t');
    writeAndUpdateCRC('<');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC(value);
    writeAndUpdateCRC('<');
    writeAndUpdateCRC('/');
    writeAndUpdateCRC(tag);
    writeAndUpdateCRC('>');
    writeAndUpdateCRC('\n');
}

void XMLWriter::writeNode(String tag, String value)
{
    writeNode(tag.c_str(), value.c_str());
}

void XMLWriter::writeNode(String tag, const char* value)
{
    writeNode(tag.c_str(), value);
}

// --------------------------------------------------------
// CRC control functions
// --------------------------------------------------------

void XMLWriter::crcReset()
{
    tx_crc = reset_crc;
}

uint16_t XMLWriter::crcValue()
{
    return tx_crc;
}

void XMLWriter::writeCRC()
{
    _stream->print("<CRC>");
    _stream->print(tx_crc, DEC);
    _stream->print("</CRC>\n");
#ifdef LOG
    _log->print("<CRC>");
    _log->print(tx_crc, DEC);
    _log->print("</CRC>\n");
#endif
    crcReset();
}

// --------------------------------------------------------
// Send over serial and update the CRC
// --------------------------------------------------------

void XMLWriter::writeAndUpdateCRC(uint8_t* data, uint8_t length)
{
    if (length == 0 || data == NULL) {
        return;
    }
    uint16_t c;
    uint8_t x;
    for (c = 0; c < length; c++) {
        x = data[c];
        if ((x == 0) & (c != 0)) {
            break;
        }
        writeAndUpdateCRC(x);
    }
    return;
}

void XMLWriter::writeAndUpdateCRC(const char* data)
{
    uint16_t c;
    uint8_t x;
    for (c = 0; c < 100; c++) {
        x = data[c];
        if ((x == 0) & (c != 0)) {
            break;
        }
        writeAndUpdateCRC(x);
    }
    return;
}

void XMLWriter::writeAndUpdateCRC(uint8_t data)
{
    uint8_t msb = tx_crc >> 8;
    uint8_t lsb = tx_crc & 255;
    uint16_t c;
    _stream->write(data);
#ifdef LOG
    _log->write(data);
#endif
    c = data ^ msb;
    c ^= (c >> 4);
    msb = (lsb ^ (c >> 3) ^ (c << 4)) & 255;
    lsb = (c ^ (c << 5)) & 255;
    tx_crc = (msb << 8) + lsb;
    return;
}

// --------------------------------------------------------
// Write specific fields
// --------------------------------------------------------

uint16_t XMLWriter::msgNode()
{
    String buf = String(messCount);
    writeNode("Msg", buf.c_str());
    messCount++;
    if (messCount == 65534) {
        messCount = 1;
    }
    return messCount;
}

void XMLWriter::instNode()
{
    writeNode("Inst", inst_ids[instrument]);
}

// --------------------------------------------------------
// Send specific Zephyr messages
// --------------------------------------------------------

void XMLWriter::IMR()
{
    tagOpen("IMR");
    msgNode();
    instNode();
    writeNode("SWDate", swDate);
    writeNode("SWVersion", swVer);
    writeNode("ZProtocolVersion", Zproto);
    tagClose("IMR");
    writeCRC();
}

void XMLWriter::S()
{
    tagOpen("S");
    msgNode();
    instNode();
    tagClose("S");
    writeCRC();
}

void XMLWriter::RA()
{
    if (RACHUTS != instrument) { //Writer does not have inst enum
#ifdef LOG
        _log->print("Invalid devId: ");
        _log->println(inst_ids[instrument]);
#endif
        return;
    }
    tagOpen("RA");
    msgNode();
    writeNode("Inst", "RACHUTS");
    tagClose("RA");
    writeCRC();
}

void XMLWriter::IMAck(bool ackval)
{
    tagOpen("IMAck");
    msgNode();
    instNode();
    if (ackval) {
        writeNode("Ack", "ACK");
    } else {
        writeNode("Ack", "NACK");
    }

    tagClose("IMAck");
    writeCRC();
}

void XMLWriter::TCAck(bool ackval)
{
    tagOpen("TCAck");
    msgNode();
    instNode();
    if (ackval) {
        writeNode("Ack", "ACK");
    } else {
        writeNode("Ack", "NACK");
    }
    tagClose("TCAck");
    writeCRC();
}

// --------------------------------------------------------
// Send telemetry messages
// --------------------------------------------------------

void XMLWriter::TM()
{
    tagOpen("TM");
    msgNode();
    instNode();
    sendTMBody();
    String buf = String(num_tm_elements);
    writeNode("Length", buf.c_str());
    tagClose("TM");
#ifdef LOG
    _log->print("Number of items in telemetry buffer: ");
    _log->println(num_tm_elements);
#endif
    writeCRC();
    sendBin();
}

void XMLWriter::TM_String(StateFlag_t state_flag, const char * message)
{
    tagOpen("TM");
    msgNode();
    instNode();

    // write the state flag
    if (state_flag == FINE) {
        writeNode("StateFlag1", "FINE");
    } else if (state_flag == WARN) {
        writeNode("StateFlag1", "WARN");
    } else if (state_flag == CRIT) {
        writeNode("StateFlag1", "CRIT");
    } else {
        writeNode("StateFlag1", "UNKN");
    }

    // write the actual message of up to 100 chars
    writeNode("StateMess1", message);

    writeNode("Length", "0"); // no binary
    tagClose("TM");
    writeCRC();
    sendEmptyBin(); // expected, even if empty
}

void XMLWriter::TMhouse()
{ // Sends only housekeeping data
    // Intended for use in low power and standby
    tagOpen("TM");
    msgNode();
    instNode();
    sendTMBody();

    writeNode("Length", "0");
    tagClose("TM");
    writeCRC();
    _stream->print("START");
    _log->print("START");
    byte temp_byte = reset_crc >> 8;
    _log->print((byte)temp_byte, HEX);
    _stream->write((byte)temp_byte);
    temp_byte = (reset_crc && (0x00FF));
    _stream->write((byte)temp_byte);
    _stream->print("END");
    _log->print((byte)temp_byte, HEX);
    _log->print("END");
}

void XMLWriter::sendBin()
{
    // Calling function does proper input check
    crcReset();
    _stream->print("START");
#ifdef LOG
    _log->print("START");
#endif

    for (uint16_t i = 0; i < num_tm_elements; i++) {
        writeAndUpdateCRC(tm_buffer[i]);
    }

    uint16_t binCrc = tx_crc;
    uint8_t send;
    crcReset();

    send = binCrc >> 8;
    _stream->write((byte)send);
    send = (binCrc & (0x00FF));
    _stream->write((byte)send);

    _stream->print("END");
#ifdef LOG
    _log->println();
    _log->println(binCrc, HEX);
    _log->println("END");
#endif

    tm_buff_sent = true;

    return;
}

void XMLWriter::sendEmptyBin()
{
    uint16_t binCrc = tx_crc;
    uint8_t send;

    // Calling function does proper input check
    crcReset();
    _stream->print("START");
#ifdef LOG
    _log->print("START");
#endif

    send = binCrc >> 8;
    _stream->write(send);
    send = (binCrc & (0x00FF));
    _stream->write(send);

    //end
    _stream->print("END");
#ifdef LOG
    _log->println();
    _log->println(binCrc, HEX);
    _log->println("END");
#endif
}

void XMLWriter::sendTMBody()
{
    switch (flag1) {
    case (FINE):
        writeNode(StateFlag1, "FINE");
        break;
    case (CRIT):
        writeNode(StateFlag1, "CRIT");
        break;
    case (WARN):
        writeNode(StateFlag1, "WARN");
        break;
    case (UNKN):
        writeNode(StateFlag1, "UNKN");
        break;
    default:
        writeNode("StateMess1", "UNKN");
    }
    if (details1.length() != 0) {
        writeNode("StateMess1", details1);
    }

    switch (flag2) {
    case (FINE):
        writeNode(StateFlag2, "FINE");
        break;
    case (CRIT):
        writeNode(StateFlag2, "CRIT");
        break;
    case (WARN):
        writeNode(StateFlag2, "WARN");
        break;
    case (UNKN):
        writeNode(StateFlag2, "UNKN");
        break;
    case (NOMESS):
        break;
    default:
        writeNode(StateFlag2, "UNKN");
    }
    if (details2.length() != 0) {
        writeNode("StateMess2", details2);
    }

    switch (flag3) {
    case (FINE):
        writeNode(StateFlag3, "FINE");
        break;
    case (CRIT):
        writeNode(StateFlag3, "CRIT");
        break;
    case (WARN):
        writeNode(StateFlag3, "WARN");
        break;
    case (UNKN):
        writeNode(StateFlag3, "UNKN");
        break;
    case (NOMESS):
        break;
    default:
        writeNode(StateFlag3, "UNKN");
    }
    if (details3.length() != 0) {
        writeNode("StateMess3", details3);
    }
}

// --------------------------------------------------------
// Telemetry buffer interface functions
// --------------------------------------------------------

bool XMLWriter::addTm(uint8_t inChar)
{
    return addTMByte(inChar);
}

bool XMLWriter::addTm(uint16_t inWord)
{
    uint8_t outChar;

    outChar = inWord >> 8;
    if (!addTMByte(outChar)) return false;

    outChar = (inWord & 0xFF);
    return addTMByte(outChar);

}

bool XMLWriter::addTm(uint32_t inDouble)
{
    uint8_t outChar;

    outChar = inDouble >> 24;
    if (!addTMByte(outChar)) return false;

    outChar = ((inDouble >> 16) & 0x000000FF);
    if (!addTMByte(outChar)) return false;

    outChar = ((inDouble >> 8) & 0x000000FF);
    if (!addTMByte(outChar)) return false;

    outChar = ((inDouble) & 0x000000FF);
    return addTMByte(outChar);
}

bool XMLWriter::addTm(String inStr)
{
    bool err = false;
    for (uint8_t i = 0; i < inStr.length(); i++) {
        err = addTm((uint8_t)inStr.charAt(i));
#ifdef TM_DEBUG
        _stream->print((uint8_t)inStr.charAt(i));
#endif
        if (err)
            break;
    }
    return err;
}

bool XMLWriter::addTm(const uint8_t * buffer, uint16_t size)
{
    if (NULL == buffer) return false;

    for (uint16_t i = 0; i < size; i++) {
        if (!addTMByte(buffer[i])) return false;
    }

    return true;
}

bool XMLWriter::addTmTemp(float tempFloat)
{
    uint16_t tempInt = tempFloat2Bin(tempFloat);
    return addTm(tempInt);
}

bool XMLWriter::addTmGPS(float gpsFloat)
{
    uint32_t gpsInt = latLongFloat2Bin(gpsFloat);
    uint8_t err = addTm((uint8_t)(gpsInt >> 16));
    if (err) {
        return 1;
    }
    return addTm((uint16_t)(gpsInt & 0x0000FFFF));
}

bool XMLWriter::addTmVolt(uint16_t voltInt)
{
    uint8_t voltBin = voltInt2Short(voltInt);
    return addTm(voltBin);
}

void XMLWriter::clearTm()
{
    num_tm_elements = 0;
    tm_buff_sent = false;
}

uint16_t XMLWriter::getTmLen()
{
    return num_tm_elements;
}

uint16_t XMLWriter::getTmBuffer(uint8_t ** buffer)
{
    *buffer = tm_buffer;
    return num_tm_elements;
}

inline bool XMLWriter::addTMByte(uint8_t in_byte)
{
    // if we're adding to the buffer after it's been sent, reset it
    if (tm_buff_sent) {
        tm_buff_sent = false;
        num_tm_elements = 0;
    }

    if (TMBUF_MAXSIZE == num_tm_elements) return false;

    tm_buffer[num_tm_elements++] = in_byte;
    return true;
}

// END OF FILE