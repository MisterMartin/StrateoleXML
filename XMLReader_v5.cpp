/*
 * XMLReader_v5.cpp
 * Author:  Alex St. Clair
 * Created: August 2019
 *
 * This file implements a class to handle reading and parsing Strateole 2 XML
 * messages from the Zephyr gondola's On-Board Computer.
 *
 * The reader uses the serial buffers internal to the Arduino software core,
 * and this reader is designed for use on the Teensy 3.6, for which it is easy
 * and necessary to modify the core to increase the size of these buffers.
 *
 * Version 5 is a complete re-design of the XMLReader
 */

#include "XMLReader_v5.h"

char inst_ids[4][8] = {"FLOATS", "RACHUTS", "LPC", "RATS"};

// global structs for received parameters
DIB_Param_t dibParam = {0};
PIB_Param_t pibParam = {0};
LPC_Param_t lpcParam = {0};
MCB_Param_t mcbParam = {0};
PU_Param_t puParam = {0};

XMLReader::XMLReader(Stream * rxstream, Instrument_t inst)
{
    rx_stream = rxstream;
    instrument = inst;
}

// get the next character from the stream and update the CRC
inline bool XMLReader::ReadNextChar(char * new_char)
{
    uint16_t c;
    uint8_t ret_char, msb, lsb;

    // make sure the new character is good, if not, return failure
    int read_ret = rx_stream->read();
    //Serial.write(read_ret);
    if (read_ret == -1) return false;
    ret_char = (uint8_t) read_ret;

    // update CRC
    msb = working_crc >> 8;
    lsb = working_crc & 0xFF;
    c = ret_char ^ msb;
    c ^= (c >> 4);
    msb = (lsb ^ (c >> 3) ^ (c << 4)) & 255;
    lsb = (c ^ (c << 5)) & 255;
    working_crc = (msb << 8) + lsb;

    // assign the char to the receive location, return success
    *new_char = (char) ret_char;
    return true;
}

void XMLReader::ResetReader()
{
    working_crc = crc_poly;
    crc_result = 0;
    num_fields = 0;

    // null-terminate all buffer first characters
    message_buff[0] = '\0';
    for (int i = 0; i < MAX_MSG_FIELDS; i++) {
        fields[i][0] = '\0';
        field_values[i][0] = '\0';
    }
}

bool XMLReader::GetNewMessage()
{
    // set a 0.1 second timeout
    uint32_t timeout = millis() + 200;
    char read_char = '\0';

    // read the message type opening tag through the newline, verify the type
    if (!MessageTypeOpen(timeout)) {
        ResetReader();
        return false;
    }

    // as long as there is a tab next, read a full field through the newline
    while (millis() < timeout) {
        if (rx_stream->available()) {
            if ('\t' == rx_stream->peek()) {
                // clear the \t and read the field
                if (!ReadNextChar(&read_char) || !ReadField(timeout)) {
                    ResetReader();
                    rx_stream->flush();
                    return false;
                }
            } else {
                break; // no tab means no more fields
            }
        }
    }

    // read the message type closing tag through the newline
    if (!MessageTypeClose(timeout)) {
        ResetReader();
        rx_stream->flush();
        return false;
    }

    // save the crc result (working_crc will still be updating unnecessarily)
    crc_result = working_crc;

    // read and verify the full CRC through the newline
    if (!ReadVerifyCRC(timeout))  {
        ResetReader();
        rx_stream->flush();
        return false;
    }

    // parse the message
    if (!ParseMessage()) return false;

    // Add an extra 0.1 seconds to the timeout for the binary section
    timeout += 100;

    // read the binary section if it's a telecommand
    if (TC == zephyr_message && !ReadBinarySection(timeout)) {
        ResetReader();
        rx_stream->flush();
        return false;
    }

    ResetReader();
    return true;
}

// --------------------------------------------------------
// Message Parsing
// --------------------------------------------------------

bool XMLReader::ParseMessage()
{
    unsigned int utemp = 0;

    // first field is always message id
    if (0 != strcmp(fields[0], "Msg")) return false;
    if (1 != sscanf(field_values[0], "%u", &utemp)) return false;
    if (utemp > 65535) return false;
    message_id = (uint16_t) utemp;

    // GPS message differs entirely from here on, parse it separately
    if (GPS == zephyr_message) return ParseGPSMessage();

    // verify instrument id
    if (0 != strcmp(fields[1], "Inst")) return false;
    if (0 != strcmp(field_values[1], inst_ids[instrument])) return false;

    switch (zephyr_message) {
    case IM:
        // verify mode field
        if (0 != strcmp(fields[2], "Mode")) return false;

        // get mode
        if (0 == strcmp(field_values[2], "SB")) {
            zephyr_mode = MODE_STANDBY;
        } else if (0 == strcmp(field_values[2], "FL")) {
            zephyr_mode = MODE_FLIGHT;
        } else if (0 == strcmp(field_values[2], "LP")) {
            zephyr_mode = MODE_LOWPOWER;
        } else if (0 == strcmp(field_values[2], "SA")) {
            zephyr_mode = MODE_SAFETY;
        } else if (0 == strcmp(field_values[2], "EF")) {
            zephyr_mode = MODE_EOF;
        } else {
            return false;
        }
        break;
    case SAck:
    case RAAck:
    case TMAck:
        // verify ack field
        if (0 != strcmp(fields[2], "Ack")) return false;

        // get ack value
        if (0 == strcmp(field_values[2], "ACK")) {
            zephyr_ack = true;
        } else if (0 == strcmp(field_values[2], "NAK")) {
            zephyr_ack = false;
        } else {
            return false;
        }
        break;
    case SW:
        // shutdown warning has no more fields
        break;
    case TC:
        // verify binary length field
        if (0 != strcmp(fields[2], "Length")) return false;

        // get the binary length
        if (1 != sscanf(field_values[2], "%u", &utemp)) return false;
        if (utemp > MAX_TC_SIZE) return false;
        tc_length = (uint16_t) utemp;
        break;
    default:
        return false;
    }

    return true;
}

// Parse the GPS message, ensure that the GPS struct only ever contains valid data
bool XMLReader::ParseGPSMessage()
{
    float longtemp, lattemp, alttemp, szatemp, vbattemp, difftemp;
    unsigned int yeartemp, monthtemp, daytemp, hourtemp, minutetemp, secondtemp, qualitytemp;

    // verify the fields
    if (0 != strcmp(fields[1], "Date")) return false;
    if (0 != strcmp(fields[2], "Time")) return false;
    if (0 != strcmp(fields[3], "Lon")) return false;
    if (0 != strcmp(fields[4], "Lat")) return false;
    if (0 != strcmp(fields[5], "Alt")) return false;
    if (0 != strcmp(fields[6], "SZA")) return false;
    if (0 != strcmp(fields[7], "VBAT")) return false;
    if (0 != strcmp(fields[8], "Diff")) return false;
    if (0 != strcmp(fields[9], "Quality")) return false;

    // parse the date (YYYY/MM/DD)
    if (3 != sscanf(field_values[1], "%u/%u/%u", &yeartemp, &monthtemp, &daytemp)) return false;
    if (yeartemp > 2050) return false;
    if (monthtemp > 12) return false;
    if (daytemp > 31) return false;

    // parse the time (HH:MM:SS)
    if (3 != sscanf(field_values[2], "%u:%u:%u", &hourtemp, &minutetemp, &secondtemp)) return false;
    if (hourtemp > 23) return false;
    if (minutetemp > 59) return false;
    if (secondtemp > 59) return false; // don't handle leap seconds

    // parse the longitude
    if (1 != sscanf(field_values[3], "%f", &longtemp)) return false;

    // parse the latitude
    if (1 != sscanf(field_values[4], "%f", &lattemp)) return false;

    // parse the altitude
    if (1 != sscanf(field_values[5], "%f", &alttemp)) return false;

    // parse the solar zenith angle
    if (1 != sscanf(field_values[6], "%f", &szatemp)) return false;

    // parse the vbat
    if (1 != sscanf(field_values[7], "%f", &vbattemp)) return false;

    // parse the diff
    if (1 != sscanf(field_values[8], "%f", &difftemp)) return false;

    // parse the GPS fix quality
    if (1 != sscanf(field_values[9], "%u", &qualitytemp)) return false;
    if (0 == qualitytemp) return false; // ignore these messages

    // only assign values once the message has been parsed successfully
    zephyr_gps.year = (uint16_t) yeartemp;
    zephyr_gps.month = (uint8_t) monthtemp;
    zephyr_gps.day = (uint8_t) daytemp;
    zephyr_gps.hour = (uint8_t) hourtemp;
    zephyr_gps.minute = (uint8_t) minutetemp;
    zephyr_gps.second = (uint8_t) secondtemp;
    zephyr_gps.quality = qualitytemp;
    if (qualitytemp >= 1) {
        zephyr_gps.longitude = longtemp;
        zephyr_gps.latitude = lattemp;
        zephyr_gps.altitude = alttemp;
        zephyr_gps.solar_zenith_angle = szatemp;
        zephyr_gps.diff = difftemp;
        zephyr_gps.vbat = vbattemp;
    }

    return true;
}

// --------------------------------------------------------
// Read specific message parts into buffers
// --------------------------------------------------------

bool XMLReader::MessageTypeOpen(uint32_t timeout)
{
    int stream_peek;

    // wait while the buffer contains characters until the opening '<' is next
    stream_peek = rx_stream->peek();
    while (millis() < timeout && -1 != stream_peek && '<' != stream_peek) {
        rx_stream->read(); // clear the char
        stream_peek = rx_stream->peek(); // look at the next
    }

    // ensure we have the opening character
    if ('<' != stream_peek) return false;

    // read in the message type opening tag and newline
    if (!ReadOpeningTag(timeout, message_buff, 8)) return false;
    if (!ReadSpecificChar(timeout, '\n')) return false;

    // determine the message type
    if (0 == strcmp(MSG_IM, message_buff)) {
        zephyr_message = IM;
    } else if (0 == strcmp(MSG_SAck, message_buff)) {
        zephyr_message = SAck;
    } else if (0 == strcmp(MSG_SW, message_buff)) {
        zephyr_message = SW;
    } else if (0 == strcmp(MSG_RAAck, message_buff)) {
        zephyr_message = RAAck;
    } else if (0 == strcmp(MSG_TMAck, message_buff)) {
        zephyr_message = TMAck;
    } else if (0 == strcmp(MSG_TC, message_buff)) {
        zephyr_message = TC;
    } else if (0 == strcmp(MSG_GPS, message_buff)) {
        zephyr_message = GPS;
    } else { // error
        zephyr_message = UNKNOWN;
        return false;
    }

    return true;
}

bool XMLReader::MessageTypeClose(uint32_t timeout)
{
    char close_type[8] = {0};

    // read the tag and the newline
    if (!ReadSpecificChar(timeout, '<')) return false;
    if (!ReadClosingTag(timeout, close_type, 8)) return false;
    if (!ReadSpecificChar(timeout, '\n')) return false;

    // verify that the closing message type matches the opening type
    if (0 != strcmp(close_type, message_buff)) return false;

    return true;
}

bool XMLReader::ReadField(uint32_t timeout)
{
    int itr = 0;
    char close_field[8] = {0};
    char new_char = '\0';

    // read opening field tag
    if (!ReadOpeningTag(timeout, fields[num_fields], MAX_MSG_FIELDS)) return false;

    // read the field value until start of close tag or error
    while (millis() < timeout && itr < 15) {
        // if there's a character available, parse it
        if (ReadNextChar(&new_char)) {
            if ('<' == new_char) {
                break;
            } else {
                field_values[num_fields][itr++] = new_char;
            }
        }
    }

    // always null-terminate the buffer
    field_values[num_fields][itr] = '\0';

    // verify we've started the closing tag
    if ('<' != new_char && !ReadSpecificChar(timeout, '<')) return false;

    // read closing field tag
    if (!ReadClosingTag(timeout, close_field, 8)) return false;

    // ensure the opening and closing field tags match
    if (0 != strcmp(close_field, fields[num_fields])) return false;

    // get the newline
    if (!ReadSpecificChar(timeout, '\n')) return false;

    num_fields++;
    return true;
}

bool XMLReader::ReadVerifyCRC(uint32_t timeout)
{
    int itr = 0;
    unsigned int read_crc = 0;
    char crc_tag[4] = {0};
    char crc_value[6] = {0};
    char new_char = '\0';

    // read and verify opening CRC tag
    if (!ReadOpeningTag(timeout, crc_tag, 4)) return false;
    if (0 != strcmp(crc_tag, "CRC")) return false;

    // read the CRC value until start of close tag or error
    while (millis() < timeout && itr < 5) {
        // if there's a character available, parse it
        if (ReadNextChar(&new_char)) {
            if ('<' == new_char) {
                break;
            } else {
                crc_value[itr++] = new_char;
            }
        }
    }

    // always null-terminate the buffer
    crc_value[itr] = '\0';

    // verify we've started the closing tag
    if ('<' != new_char && !ReadSpecificChar(timeout, '<')) return false;

    // read and verify closing crc tag
    if (!ReadClosingTag(timeout, crc_tag, 4)) return false;
    if (0 != strcmp(crc_tag, "CRC")) return false;

    // convert the crc from the message to uint16_t
    if (1 != sscanf(crc_value, "%u", &read_crc)) return false;
    if (read_crc > 65535) return false;

    // get the trailing newline if it's there
    if ('\n' == rx_stream->peek()) {
        rx_stream->read();
    }

    // return the CRC result
    return true; //((uint16_t) read_crc == crc_result);
}

bool XMLReader::ReadBinarySection(uint32_t timeout)
{
    uint16_t itr = 0;
    uint16_t read_crc = 0;
    char rx_char = '\0';

    // read "START" from the stream
    if (!ReadSpecificChar(timeout, 'S')) return false;
    if (!ReadSpecificChar(timeout, 'T')) return false;
    if (!ReadSpecificChar(timeout, 'A')) return false;
    if (!ReadSpecificChar(timeout, 'R')) return false;
    if (!ReadSpecificChar(timeout, 'T')) return false;

    // reset CRC for the binary section
    working_crc = crc_poly;

    // read the binary section into the telecommand buffer
    num_tcs = 0;
    tc_index = 0;
    curr_tc = 0;
    while (millis() < timeout && itr < tc_length) {
        if (ReadNextChar(&rx_char)) {
            tc_buffer[itr++] = rx_char;
            if (';' == rx_char) num_tcs++;
        }
    }

    // verify that we read all of the expected characters
    if (itr != tc_length) return false;

    // TC buffer is parsed as a char array string, so null-terminate it
    tc_buffer[itr] = '\0';

    // store the CRC result for comparison with the transmitted value
    crc_result = working_crc;

    // read the first CRC byte (LSB) from the stream
    while (millis() < timeout && !ReadNextChar(&rx_char));
    read_crc = (uint16_t) rx_char;

    // read the second CRC byte (MSB) from the stream
    while (millis() < timeout && !ReadNextChar(&rx_char));
    read_crc |= (uint16_t) ((uint8_t) rx_char << 8);

    // verify that the stream ends with "END"
    if (!ReadSpecificChar(timeout, 'E')) return false;
    if (!ReadSpecificChar(timeout, 'N')) return false;
    if (!ReadSpecificChar(timeout, 'D')) return false;

    return true; //read_crc == crc_result;
}

// --------------------------------------------------------
// Generic Helper Functions
// --------------------------------------------------------

// read the desired character, fail if timeout or wrong character
bool XMLReader::ReadSpecificChar(uint32_t timeout, char specific_char)
{
    char new_char;

    // wait until there's a character available
    while (millis() < timeout && !rx_stream->available());

    // verify that we get the expected char
    if (!ReadNextChar(&new_char) || specific_char != new_char) return false;

    return true;
}

bool XMLReader::ReadOpeningTag(uint32_t timeout, char * buffer, uint8_t buff_size)
{
    int itr = 0;
    char new_char = '\0';

    if (!ReadSpecificChar(timeout, '<')) return false;

    // read the tag until close or error
    while (millis() < timeout && itr < (buff_size - 1)) {
        // if there's a character available, parse it
        if (ReadNextChar(&new_char)) {
            if ('>' == new_char) {
                break;
            } else {
                buffer[itr++] = new_char;
            }
        }
    }

    // always null-terminate the buffer
    buffer[itr] = '\0';

    if (new_char == '>') {
        return true;
    } else {
        return ReadSpecificChar(timeout, '>');
    }
}

// note: the leading '<' should already have been read before calling
// this way, fields and CRC can read the '<' and know to stop
bool XMLReader::ReadClosingTag(uint32_t timeout, char * buffer, uint8_t buff_size)
{
    int itr = 0;
    char new_char = '\0';

    if (!ReadSpecificChar(timeout, '/')) return false;

    // read the tag until close or error
    while (millis() < timeout && itr < (buff_size - 1)) {
        // if there's a character available, parse it
        if (ReadNextChar(&new_char)) {
            if ('>' == new_char) {
                break;
            } else {
                buffer[itr++] = new_char;
            }
        }
    }

    // always null-terminate the buffer
    buffer[itr] = '\0';

    if (new_char == '>') {
        return true;
    } else {
        return ReadSpecificChar(timeout, '>');
    }
}