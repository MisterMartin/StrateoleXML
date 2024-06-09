/*
 * XMLReader_v5.h
 * Author:  Alex St. Clair
 * Created: August 2019
 *
 * This file declares a class to handle reading and parsing Strateole 2 XML
 * messages from the Zephyr gondola's On-Board Computer.
 *
 * The reader uses the serial buffers internal to the Arduino software core,
 * and this reader is designed for use on the Teensy 3.6, for which it is easy
 * and necessary to modify the core to increase the size of these buffers.
 *
 * Version 5 is a complete re-design of the XMLReader
 */

#ifndef XMLREADER_H
#define XMLREADER_H

#include "Telecommand.h"
#include "InstInfo.h"
#include "Arduino.h"
#include <TimeLib.h>
#include <stdint.h>

// The maximum number of fields that a message can contain.
#define MAX_MSG_FIELDS 10

// Message Types
#define MSG_IM      "IM"
#define MSG_SAck    "SAck"
#define MSG_SW      "SW"
#define MSG_RAAck   "RAAck"
#define MSG_TMAck   "TMAck"
#define MSG_TC      "TC"
#define MSG_GPS     "GPS"

enum ZephyrMessage_t {
    // Main HW
    IM,    // Instrument Mode
    SAck,  // Safety Acknowledge
    SW,    // Shutdown Warning
    RAAck, // Rachuts Acknowledge
    TMAck, // Telemetry Acknowledge
    TC,    // Telecommand
    GPS,   // GPS Data
    NO_ZEPHYR_MSG,
    UNKNOWN
};

// used to index StratoCore mode functions
enum InstMode_t {
    MODE_STANDBY = 0,
    MODE_FLIGHT = 1,
    MODE_LOWPOWER = 2,
    MODE_SAFETY = 3,
    MODE_EOF = 4,
    NUM_MODES = 5
};

struct GPSData_t {
    float longitude;
    float latitude;
    float altitude;
    float solar_zenith_angle;
    float diff;
    float vbat;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t quality;
};

// global structs for received parameters
extern DIB_Param_t dibParam;
extern PIB_Param_t pibParam;
extern LPC_Param_t lpcParam;
extern MCB_Param_t mcbParam;
extern PU_Param_t puParam;

class XMLReader {
public:
    // constructors/destructors
    XMLReader(Stream * rxstream, Instrument_t inst);
    ~XMLReader() { };

    // public interface functions
    bool GetNewMessage();
    TCParseStatus_t GetTelecommand(); // implemented in Telecommand.cpp

    // general message results
    ZephyrMessage_t zephyr_message = NO_ZEPHYR_MSG;
    uint16_t message_id = 0;

    // specific message results
    InstMode_t zephyr_mode = MODE_STANDBY;
    bool zephyr_ack = false;
    GPSData_t zephyr_gps = {0};

    // telecommand results
    Telecommand_t zephyr_tc = NULL_TELECOMMAND;
    char tc_buffer[MAX_TC_SIZE + 1] = {0};
    uint16_t tc_length = 0;
    uint8_t num_tcs = 0;
    uint8_t curr_tc = 0;

private:
    // parsing functions
    bool ParseMessage();
    bool ParseGPSMessage();

    // get the next character from the stream and update the CRC
    bool ReadNextChar(char * new_char);

    // read different parts of the message into buffers
    bool MessageTypeOpen(uint32_t timeout);
    bool MessageTypeClose(uint32_t timeout);
    bool ReadField(uint32_t timeout);
    bool ReadVerifyCRC(uint32_t timeout);
    bool ReadBinarySection(uint32_t timeout);

    // generic helpers
    bool ReadSpecificChar(uint32_t timeout, char specific_char);
    bool ReadOpeningTag(uint32_t timeout, char * buffer, uint8_t buff_size);
    bool ReadClosingTag(uint32_t timeout, char * buffer, uint8_t buff_size);

    // after every message or error
    void ResetReader();

    // called for each message, gets parameters (if any) from the tc_buffer
    bool ParseTelecommand(uint8_t telecommand);

    // telecommand parsing utilities (implemented in Telecommand.cpp)
    bool Get_uint8(uint8_t * ret_array, uint8_t num_elements);
    bool Get_uint16(uint16_t * ret_array, uint8_t num_elements);
    bool Get_uint32(uint32_t * ret_array, uint8_t num_elements);
    bool Get_int8(int8_t * ret_array, uint8_t num_elements);
    bool Get_int16(int16_t * ret_array, uint8_t num_elements);
    bool Get_int32(int32_t * ret_array, uint8_t num_elements);
    bool Get_float(float * ret_array, uint8_t num_elements);
    void ClearTC(); // clears the rest of an errant TC

    // serial port for Strateole on-board computer
    Stream * rx_stream;

    // Instrument id
    Instrument_t instrument;

    // CRC-CCITT16 internals
    const uint16_t crc_poly = 0x1021;
    uint16_t working_crc = 0;
    uint16_t crc_result = 0;

    // internal buffers for message parts
    char message_buff[8] = {0};
    char fields[MAX_MSG_FIELDS][8] = {{0}};
    char field_values[MAX_MSG_FIELDS][16] = {{0}};
    uint8_t num_fields = 0;

    // internal telecommand tracking
    uint16_t tc_index = 0;

};

#endif /* XMLREADER_H */