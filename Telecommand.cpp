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
    zephyr_tc = NULL_TELECOMMAND;

    // make sure there are still TCs in the buffer
    if (curr_tc++ == num_tcs) {
        return NO_TCs;
    }

    // read the telecommand number
    if (!Get_uint8((uint8_t *) &zephyr_tc, 1)) {
        ClearTC();
        return TC_ERROR;
    }

    if (!ParseTelecommand(zephyr_tc)) {
        ClearTC();
        return TC_ERROR;
    } else {
        return READ_TC;
    }
}

// get the telecommand parameters, if any
bool XMLReader::ParseTelecommand(uint8_t telecommand)
{
    switch (telecommand) {
    // MCB Parameters -------------------------------------
    case DEPLOYx:
        if (!Get_float(&(mcbParam.deployLen),1)) return false;
        break;
    case DEPLOYv:
        if (!Get_float(&(mcbParam.deployVel),1)) return false;
        break;
    case DEPLOYa:
        if (!Get_float(&(mcbParam.deployAcc),1)) return false;
        break;
    case RETRACTx:
        if (!Get_float(&(mcbParam.retractLen),1)) return false;
        break;
    case RETRACTv:
        if (!Get_float(&(mcbParam.retractVel),1)) return false;
        break;
    case RETRACTa:
        if (!Get_float(&(mcbParam.retractAcc),1)) return false;
        break;
    case DOCKx:
        if (!Get_float(&(mcbParam.dockLen),1)) return false;
        break;
    case DOCKv:
        if (!Get_float(&(mcbParam.dockVel),1)) return false;
        break;
    case DOCKa:
        if (!Get_float(&(mcbParam.dockAcc),1)) return false;
        break;
    case TEMPLIMITS:
        if (!Get_float(mcbParam.tempLimits,6)) return false;
        break;
    case TORQUELIMITS:
        if (!Get_float(mcbParam.torqueLimits,2)) return false;
        break;
    case CURRLIMITS:
        if (!Get_float(mcbParam.currLimits,2)) return false;
        break;
    // LPC Parameters -------------------------------------
    case SETSAMPLE:
        if (!Get_uint16(&(lpcParam.samples),1)) return false;
        break;
    case SETWARMUPTIME:
        if (!Get_uint16(&(lpcParam.warmUpTime),1)) return false;
        break;
    case SETCYCLETIME:
        if (!Get_uint8(&(lpcParam.setCycleTime),1)) return false;
        break;
    case SETHGBINS:
        if (!Get_uint8(lpcParam.newHGBins,24)) return false;
        break;
    case SETLGBINS:
        if (!Get_uint8(lpcParam.newLGBins,24)) return false;
        break;
    case SETLASERTEMP:
        if (!Get_uint8(&(lpcParam.setLaserTemp),1)) return false;
        break;
    case SETFLUSH:
        if (!Get_uint8(&(lpcParam.lpc_flush),1)) return false;
        break;
    case SETSAMPLEAVG:
        if (!Get_uint16(&(lpcParam.samplesToAverage),1)) return false;
        break;
    case SETPHA:
        if (!Get_uint16(&(lpcParam.phaBaseline),1)) return false;
        if (!Get_uint16(&(lpcParam.phaHiGainOffset),1)) return false;
        if (!Get_uint16(&(lpcParam.phaLoGainOffset),1)) return false;
        break;
    // DIB Parameters -------------------------------------
    case FTRONTIME:
        if (!Get_uint16(&(dibParam.ftrOnTime),1)) return false;
        break;
    case FTRCYCLETIME:
        if (!Get_uint16(&(dibParam.ftrCycleTime),1)) return false;
        break;
    case SETDIBHKPERIOD:
        if (!Get_uint16(&(dibParam.hkPeriod),1)) return false;
        break;
    case FTRSTATUSLIMIT:
        if (!Get_uint16(&(dibParam.statusLimit),1)) return false;
        break;
    case RAMANLEN:
        if (!Get_uint16(&(dibParam.ramanScanLength),1)) return false;
        break;
    case SETMEASURETYPE:
        if (!Get_uint8(&(dibParam.ftrMeasureType),1)) return false;
        if (!Get_uint8(&(dibParam.ftrBurstLim),1)) return false;
        break;
    // PIB Parameters -------------------------------------
    case SETSZAMIN:
        if (!Get_float(&(pibParam.szaMinimum),1)) return false;
        break;
    case SETPROFILESIZE:
        if (!Get_float(&(pibParam.profileSize),1)) return false;
        break;
    case SETDOCKAMOUNT:
        if (!Get_float(&(pibParam.dockAmount),1)) return false;
        break;
    case SETDWELLTIME:
        if (!Get_uint16(&(pibParam.dwellTime),1)) return false;
        break;
    case SETPROFILEPERIOD:
        if (!Get_uint16(&(pibParam.profilePeriod),1)) return false;
        break;
    case SETNUMPROFILES:
        if (!Get_uint8(&(pibParam.numProfiles),1)) return false;
        break;
    case SETTIMETRIGGER:
        if (!Get_uint32(&(pibParam.timeTrigger),1)) return false;
        break;
    case SETDOCKOVERSHOOT:
        if (!Get_float(&(pibParam.dockOvershoot),1)) return false;
        break;
    case RETRYDOCK:
        if (!Get_float(&(mcbParam.deployLen),1)) return false;
        if (!Get_float(&(mcbParam.retractLen),1)) return false;
        break;
    case MANUALPROFILE:
        if (!Get_float(&(pibParam.profileSize),1)) return false;
        if (!Get_float(&(pibParam.dockAmount),1)) return false;
        if (!Get_float(&(pibParam.dockOvershoot),1)) return false;
        if (!Get_uint16(&(pibParam.dwellTime),1)) return false;
        break;
    case SETPREPROFILETIME:
        if (!Get_uint16(&(pibParam.preprofileTime),1)) return false;
        break;
    case SETPUWARMUPTIME:
        if (!Get_uint16(&(pibParam.warmupTime),1)) return false;
        break;
    case AUTOREDOCKPARAMS:
        if (!Get_float(&(pibParam.autoRedockOut),1)) return false;
        if (!Get_float(&(pibParam.autoRedockIn),1)) return false;
        if (!Get_uint8(&(pibParam.numRedock),1)) return false;
        break;
    case SETMOTIONTIMEOUT:
        if (!Get_uint8(&(pibParam.motionTimeout),1)) return false;
        break;
    case DOCKEDPROFILE:
        if (!Get_uint16(&(pibParam.dockedProfileTime),1)) return false;
        break;
    // PU parameters --------------------------------------
    case PUWARMUPCONFIGS:
        if (!Get_float(&(puParam.flashT),1)) return false;
        if (!Get_float(&(puParam.heater1T),1)) return false;
        if (!Get_float(&(puParam.heater2T),1)) return false;
        if (!Get_uint8(&(puParam.flashPower),1)) return false;
        if (!Get_uint8(&(puParam.tsenPower),1)) return false;
        break;
    case PUPROFILECONFIGS:
        if (!Get_uint32(&(puParam.profileRate),1)) return false;
        if (!Get_uint32(&(puParam.dwellRate),1)) return false;
        if (!Get_uint8(&(puParam.profileFLASH),1)) return false;
        if (!Get_uint8(&(puParam.profileROPC),1)) return false;
        if (!Get_uint8(&(puParam.profileTSEN),1)) return false;
        break;
    case PUDOCKEDCONFIGS:
        if (!Get_uint32(&(puParam.dockedRate),1)) return false;
        if (!Get_uint8(&(puParam.dockedFLASH),1)) return false;
        if (!Get_uint8(&(puParam.dockedROPC),1)) return false;
        if (!Get_uint8(&(puParam.dockedTSEN),1)) return false;
        break;
    // Messages without parameters ------------------------
    default:
        break;
    }

    return true;
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