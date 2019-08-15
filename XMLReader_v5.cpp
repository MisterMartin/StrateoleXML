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
 */

#include "XMLReader_v5.h"

XMLReader::XMLReader(Stream * rxstream, Instrument_t inst)
{
    rx_stream = rxstream;
    instrument = inst;
}

// get the next character from the stream and update the CRC
bool XMLReader::ReadNextChar(char * new_char)
{
    uint16_t c;
    uint8_t ret_char, msb, lsb;

    // make sure the new character is good, if not return failure
    int read_ret = rx_stream->read();
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
    zephyr_message = NO_MESSAGE;
}

bool XMLReader::GetNewMessage()
{
    // set a 0.25 second timeout
    uint32_t timeout = millis() + 250;
    char read_char = '\0';

    // read the message type opening tag through the newline, verify the type
    if (!MessageTypeOpen(timeout)) {
        ResetReader();
        return false;
    }

    // as long as there is a tab next, read a full field through the newline
    while (millis() < timeout) {
        if (rx_stream->available() && '\t' == rx_stream->peek()) {
            // clear the \t and read the field
            if (!ReadNextChar(&read_char) || !ReadField(timeout)) {
                ResetReader();
                rx_stream->flush();
                return false;
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
    if (0 != strcmp(fields[0], "Inst")) return false;
    if (0 != strcmp(field_values[0], inst_ids[instrument])) return false;

    return true;
}

bool XMLReader::ParseGPSMessage()
{
    return false;
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
    if (!ReadOpeningTag(timeout, fields[num_fields], 8)) return false;

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
    if ('<' != new_char) return false;

    // read closing field tag
    if (!ReadClosingTag(timeout, close_field, 8)) return false;

    // ensure the opening and closing field tags match
    if (0 != strcmp(close_field, field_values[num_fields])) return false;

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
    if ('<' != new_char) return false;

    // read and verify closing crc tag
    if (!ReadClosingTag(timeout, crc_tag, 4)) return false;
    if (0 != strcmp(crc_tag, "CRC")) return false;

    // convert the crc from the message to uint16_t
    if (1 != sscanf(crc_value, "%u", &read_crc)) return false;
    if (read_crc > 65535) return false;

    // return the CRC result
    return true; //((uint16_t) read_crc == crc_result);
}

bool XMLReader::ReadBinarySection(uint32_t timeout)
{
    return false;
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

    return (new_char == '>');
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

    return (new_char == '>');
}