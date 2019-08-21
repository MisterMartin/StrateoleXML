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
 *  * Version 5 removes DIB/MCB Communications and adds a bigger, safe buffer
 *
 *  This code has the following dependencies:
 *
 *  "CircularBuffer.h"
 *	"Arduino.h"
 */

#include "Arduino.h"
#include <XMLWriter_v5.h>
//#define CRC_DEBUG
//#define TM_DEBUG

const uint16_t reset_crc = 0x1021;
CircularBuffer<uint8_t, TXBUFSIZE> txbuf; // Software buffer

// Constant Device Information
char swDate[] = "20170901,000000";

char swVer[] = "0.1";

char Zproto[] = "1.0";

uint16_t messCount = 1;

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
    rx_crc = reset_crc;
    txbuf.clear();
}
//Call to set names of state flags
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

//Call to set values of state flags
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

//Call to set value of details
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

void XMLWriter::tagOpen(const char* tag)
{
    crcUpdate('<');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate('\n');
}

void XMLWriter::tagClose(const char* tag)
{
    crcUpdate('<');
    crcUpdate('/');
    crcUpdate(tag);
    crcUpdate('>');
    // if CRC problems, REPLACE following crcUpdate('\n')
    // with txbuf.push('\n');
    // Lars and Doug, look here!!
    //txbuf.push('\n');
    crcUpdate('\n');
}

void XMLWriter::writeNode(const char* tag, const char* value)
{ //string
    crcUpdate('\t');
    crcUpdate('<');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate(value);
    crcUpdate('<');
    crcUpdate('/');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate('\n');
}

void XMLWriter::writeNode(const char* tag, String value)
{ //string
    writeNode(tag, value.c_str());
}

void XMLWriter::writeNode(const char* tag, char* value, uint8_t length)
{ //char array
    crcUpdate('\t');
    crcUpdate('<');
    crcUpdate(tag);
    crcUpdate('>');
    for (uint8_t i = 0; i < length; i++) {
        if (value[i] != 0) {
            crcUpdate(value[i]);
        } else {
            break;
        }
    }
    crcUpdate('<');
    crcUpdate('/');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate('\n');
}

void XMLWriter::writeNode(const char* tag, uint8_t value)
{
    crcUpdate('\t');
    crcUpdate('<');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate(value);
    crcUpdate('<');
    crcUpdate('/');
    crcUpdate(tag);
    crcUpdate('>');
    crcUpdate('\n');
}

void XMLWriter::writeNode(String tag, String value)
{
    writeNode(tag.c_str(), value.c_str());
}

void XMLWriter::writeNode(String tag, const char* value)
{
    writeNode(tag.c_str(), value);
}

void XMLWriter::writeCRC()
{
    _stream->print("<CRC>");
    _stream->print(rx_crc, DEC);
    _stream->print("</CRC>\n");
#ifdef LOG
    _log->print("<CRC>");
    _log->print(rx_crc, DEC);
    _log->print("</CRC>\n");
#endif
    crcReset();
    //_stream->flush();
}

void XMLWriter::sendBuf()
{
    uint8_t ret;
    for (uint8_t i = 0; i < TXBUFSIZE; i++) {
        ret = txbuf.shift();
        _stream->print((char)ret);
#ifdef LOG
        _log->print((char)ret);
#endif
        if (txbuf.isEmpty()) {
            break;
        }
    }
    txbuf.clear();
    _stream->flush();
}

uint16_t XMLWriter::crcUpdate(uint8_t* data, uint8_t length)
{
    if (length == 0 || data == 0) {
        return 1;
    }
    uint16_t c;
    uint8_t x;
    for (c = 0; c < length; c++) {
        x = data[c];
        if ((x == 0) & (c != 0)) {
            break;
        }
        crcUpdate(x);
    }
    return 0;
}

uint16_t XMLWriter::crcUpdate(const char* data)
{
    uint16_t c;
    uint8_t x;
    for (c = 0; c < 100; c++) {
        x = data[c];
        if ((x == 0) & (c != 0)) {
            break;
        }
        crcUpdate(x);
    }
    return 0;
}

uint16_t XMLWriter::crcUpdate(uint8_t data)
{
    uint8_t msb = rx_crc >> 8;
    uint8_t lsb = rx_crc & 255;
    uint16_t c;
    txbuf.push(data);
    c = data ^ msb;
    c ^= (c >> 4);
    msb = (lsb ^ (c >> 3) ^ (c << 4)) & 255;
    lsb = (c ^ (c << 5)) & 255;
    rx_crc = (msb << 8) + lsb;
#ifdef CRC_DEBUG
    _log->print(rx_crc);
    _log->print(' ');
    _log->println((char)data);
#endif
    return 0;
}

void XMLWriter::crcReset()
{
    rx_crc = reset_crc;
}

uint16_t XMLWriter::crcValue()
{
    return rx_crc;
}

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
    String temp = inst_ids[instrument];
    temp.trim();
    writeNode("Inst", temp.c_str());
}

void XMLWriter::IMR()
{
    tagOpen("IMR");
    msgNode();
    instNode();
    writeNode("SWDate", swDate);
    writeNode("SWVersion", swVer);
    writeNode("ZProtocolVersion", Zproto);
    tagClose("IMR");
    sendBuf();
    writeCRC();
}

void XMLWriter::IMAck(uint8_t ackval)
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
    sendBuf();
    writeCRC();
}

void XMLWriter::S()
{
    tagOpen("S");
    msgNode();
    instNode();
    tagClose("S");
    sendBuf();
    writeCRC();
}

void XMLWriter::RA()
{
    String temp(inst_ids[instrument]);
    temp.trim();
    if (temp != "RACHUTS") { //Writer does not have inst enum
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
    sendBuf();
    writeCRC();
}
// Pass by reference is buggy
/* void XMLWriter::TM(uint8_t bin[], uint16_t len){
	tagOpen("TM");
	msgNode();
    instNode();
    sendTMBody();
	String buf = String(len);
	writeNode("Length",buf.c_str());
	tagClose("TM");
	#ifdef LOG
	_log->print("Number of items in buffer: ");
	_log->println(txbuf.size());
	#endif
	sendBuf();
	writeCRC();
	if(!bin || !len){
		#ifdef LOG
		_log->print("May have null pointer");
		#endif
		return;
	} else {
		sendBin(bin,len);
	}
} */

void XMLWriter::TM()
{
    if (!tmbuf_len) {
        TMhouse();
        return;
    }
    tagOpen("TM");
    msgNode();
    instNode();
    sendTMBody();
    String buf = String(tmbuf_len);
    writeNode("Length", buf.c_str());
    tagClose("TM");
#ifdef LOG
    _log->print("Number of items in telemetry buffer: ");
    _log->println(tmbuf_len);
#endif
    sendBuf();
    writeCRC();
    if (tmbuf_len) {
        sendBin();
    }
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
    sendBuf();
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
    sendBuf();
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

/* void XMLWriter::sendBin(uint8_t * bin, uint16_t len){
	// Calling function does proper input check
	crcReset();
	_stream->print("START");
	#ifdef LOG
	_log->print("START");
	#endif
	for(uint8_t i = 0; i < len; i++){
		crcUpdate(bin[i]);
	}
	uint16_t binCrc = rx_crc;
	uint8_t send;
	sendBuf();
	crcReset();
	// IF you encounter problems with the OBC accepting
	// the sections below (1 & 2) will need to be flipped
	// This will send the LSB before the MSB
	//1
	send = binCrc>>8;
	_stream->write((byte)send);
	#ifdef LOG
	_log->write((byte)send);
	#endif
	//2
	send = (binCrc & (0x00FF));
	_stream->write((byte)send);
	#ifdef LOG
	_log->write((byte)send);
	#endif
	//end
	_stream->print("END");
	#ifdef LOG
	_log->println(binCrc);
	_log->println("END");
	#endif
} */

void XMLWriter::sendBin()
{
    // Calling function does proper input check
    crcReset();
    _stream->print("START");
#ifdef LOG
    _log->print("START");
#endif
    uint8_t inchar;
    for (uint8_t i = 0; i < tmbuf_len; i++) {
        if (!tmbuf.isEmpty()) {
            inchar = tmbuf.shift();
            crcUpdate(inchar);
        } else {
            break;
        }
    }
    tmbuf_len = 0;
    uint16_t binCrc = rx_crc;
    uint8_t send;
    sendBuf();
    crcReset();
    // IF you encounter problems with the OBC accepting
    // the sections below (1 & 2) will need to be flipped
    // This will send the LSB before the MSB
    //1
    send = binCrc >> 8;
    _stream->write((byte)send);
    //2
    send = (binCrc & (0x00FF));
    _stream->write((byte)send);
    //end
    _stream->print("END");
#ifdef LOG
    _log->println();
    _log->println(binCrc, HEX);
    _log->println("END");
#endif
    return;
}

void XMLWriter::sendEmptyBin()
{
    uint16_t binCrc = rx_crc;
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

void XMLWriter::TCAck(uint8_t ackval)
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
    sendBuf();
    writeCRC();
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

// Interacting with telemetry buffer
uint8_t XMLWriter::addTm(uint8_t inChar)
{
    if (tmbuf_len < TXBUFSIZE) {
        tmbuf.push(inChar);
#ifdef TM_DEBUG
        _stream->print(inChar);
#endif
        tmbuf_len++;
        return 0;
    } else {
        return 1;
    }
}

uint8_t XMLWriter::addTm(uint16_t inWord)
{
    uint8_t outChar;
    if (tmbuf_len + 1 < TXBUFSIZE) {

        outChar = inWord >> 8;
        tmbuf.push(outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;

        outChar = (inWord & 0xFF);
        tmbuf.push((uint8_t)outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;

        return 0;
    } else {
        return 1;
    }
}

uint8_t XMLWriter::addTm(uint32_t inDouble)
{
    uint8_t outChar;
    if (tmbuf_len + 3 < TXBUFSIZE) {
        outChar = inDouble >> 24;
        tmbuf.push(outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;
        outChar = ((inDouble >> 16) & 0x000000FF);
        tmbuf.push(outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;
        outChar = ((inDouble >> 8) & 0x000000FF);
        tmbuf.push(outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;
        outChar = ((inDouble) & 0x000000FF);
        tmbuf.push(outChar);
#ifdef TM_DEBUG
        _stream->print(outChar);
#endif
        tmbuf_len++;
        return 0;
    } else {
        return 1;
    }
}

uint8_t XMLWriter::addTm(String inStr)
{
    uint8_t err = 0;
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

uint8_t XMLWriter::addTmTemp(float tempFloat)
{
    uint16_t tempInt = tempFloat2Bin(tempFloat);
    uint8_t ret = addTm(tempInt);
    return ret;
}

uint8_t XMLWriter::addTmGPS(float gpsFloat)
{
    uint32_t gpsInt = latLongFloat2Bin(gpsFloat);
    uint8_t err = addTm((uint8_t)(gpsInt >> 16));
    if (err) {
        return 1;
    }
    err = addTm((uint16_t)(gpsInt & 0x0000FFFF));
    return err;
}

uint8_t XMLWriter::addTmVolt(uint16_t voltInt)
{
    uint8_t voltBin = voltInt2Short(voltInt);
    uint8_t err = addTm(voltBin);
    return err;
}

uint8_t XMLWriter::remTm()
{
    tmbuf_len--;
    return tmbuf.shift();
}

void XMLWriter::clearTm()
{
    tmbuf.clear();
    tmbuf_len = 0;
}

uint16_t XMLWriter::getTmLen()
{
    return tmbuf_len;
}

// END OF FILE