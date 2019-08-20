/*
 * Telecommand.cpp
 * Author:  Alex St. Clair
 * Created: August 2019
 *
 * This file contains all of the functions for parsing telecommands from the
 * binary section of a Zephyr TC message. All telecommands are of the form:
 *
 * tc_id,param_1,param_2,...param_n;
 */

#include "Telecommand.h"
#include "XMLReader_v5.h"

// --------------------------------------------------------
// Telecommand parsing interface
// --------------------------------------------------------

TCParseStatus_t XMLReader::GetTelecommand()
{
    uint8_t telecommand = NO_TELECOMMAND;
    uint8_t test_array[2] = {0};
    float float_array[2] = {0};

    if (curr_tc++ == num_tcs) {
        return NO_TCs;
    }

    if (!Get_uint8(&telecommand, 1)) {
        ClearTC();
        return TC_ERROR;
    }

    Serial.print("TC: "); Serial.println(telecommand);

    switch (telecommand) {
    case 1:
        if (Get_uint8(test_array,1)) {
            Serial.print("Result: "); Serial.println(test_array[0]);
        } else {
            Serial.println("Read error");
        }
        break;
    case 2:
        if (Get_uint8(test_array,2)) {
            Serial.print("Result: "); Serial.println(test_array[0]);
            Serial.print("Result: "); Serial.println(test_array[1]);
        } else {
            Serial.println("Read error");
        }
        break;
    case 3:
        if (Get_float(float_array,2)) {
            Serial.print("Result: "); Serial.println(float_array[0]);
            Serial.print("Result: "); Serial.println(float_array[1]);
        } else {
            Serial.println("Read error");
        }
        break;
    default:
        break;
    }

    return READ_TC;
}

// --------------------------------------------------------
// Telecommand parsing utilties
// --------------------------------------------------------

bool XMLReader::Get_uint8(uint8_t * ret_array, uint8_t num_elements)
{
    char int_buffer[4] = {0};
    unsigned int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 3; // uint8_t can have 3 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%u", &temp)) return false;
        if (temp > 255) return false;
        ret_array[i] = (uint8_t) temp;
    }

    return true;
}

bool XMLReader::Get_uint16(uint16_t * ret_array, uint8_t num_elements)
{
    char int_buffer[6] = {0};
    unsigned int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 5; // uint16_t can have 5 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%u", &temp)) return false;
        if (temp > 65535) return false;
        ret_array[i] = (uint16_t) temp;
    }

    return true;
}

bool XMLReader::Get_uint32(uint32_t * ret_array, uint8_t num_elements)
{
    char int_buffer[11] = {0};
    unsigned int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 10; // uint32_t can have 10 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%u", &temp)) return false;
        ret_array[i] = temp;
    }

    return true;
}

bool XMLReader::Get_int8(int8_t * ret_array, uint8_t num_elements)
{
    char int_buffer[5] = {0};
    int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 4; // int8_t can have 4 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%d", &temp)) return false;
        if (temp > 127 || temp < -128) return false;
        ret_array[i] = (int8_t) temp;
    }

    return true;
}

bool XMLReader::Get_int16(int16_t * ret_array, uint8_t num_elements)
{
    char int_buffer[7] = {0};
    int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 6; // int16_t can have 6 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%d", &temp)) return false;
        if (temp > 32767 || temp < -32768) return false;
        ret_array[i] = (int16_t) temp;
    }

    return true;
}

bool XMLReader::Get_int32(int32_t * ret_array, uint8_t num_elements)
{
    char int_buffer[12] = {0};
    int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 11; // uint32 can have 11 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%d", &temp)) return false;
        ret_array[i] = temp;
    }

    return true;
}

bool XMLReader::Get_float(float * ret_array, uint8_t num_elements)
{
    char int_buffer[16] = {0};
    float temp_float = 0.0f;
    unsigned int temp = 0;
    uint16_t max_index;

    for (uint8_t i = 0; i < num_elements; i++) {
        max_index = tc_index + 15; // 15 chars max
        temp = 0;

        while (tc_index <= max_index && tc_index < tc_length) {
            if (',' == tc_buffer[tc_index] || ';' == tc_buffer[tc_index] || '\0' == tc_buffer[tc_index]) {
                break;
            }

            int_buffer[temp++] = tc_buffer[tc_index++];
        }

        int_buffer[temp] = '\0';

        // ensure next char is ',' or ';' (if the last element) and move the index past it
        if (',' == tc_buffer[tc_index] || (';' == tc_buffer[tc_index] && i == (num_elements - 1))) {
            tc_index++;
        } else {
            return false;
        }

        // convert the message id
        if (1 != sscanf(int_buffer, "%f", &temp_float)) return false;
        ret_array[i] = temp_float;
    }

    return true;
}

// clears the rest of an errant TC
void XMLReader::ClearTC()
{
    while (tc_index < tc_length && ';' != tc_buffer[tc_index++]);
}