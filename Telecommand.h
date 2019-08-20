/*
 * Telecommand.h
 * Author:  Alex St. Clair
 * Created: August 2019
 *
 * This file defines all of the telecommands and parameter structs
 * used by the XMLReader to parse and store telecommands.
 *
 * The parameter structs are defined here, but instantiated as members
 * of the XMLReader.
 */

#ifndef TELECOMMAND_H
#define TELECOMMAND_H

#include <stdint.h>

// maximum telecommand size supported
// note: 1800 is max for Zephyr
#define MAX_TC_SIZE 1800

enum TCParseStatus_t {
    READ_TC,
    TC_ERROR,
    NO_TCs
};

// Telecommand Messages
enum Telecommand_t : uint8_t {
    NO_TELECOMMAND = 0,

    // Technosoft parameters
    DEPLOYx = 1,
    RETRACTx = 2,
    DEPLOYv = 3,
    RETRACTv = 4,
    DEPLOYa = 5,
    RETRACTa = 6,
    POSERRLIM = 7,
    CURRLIM = 8,
    I2TLIM = 9,

    // MCB Parameters
    MINMCBTEMP = 30,
    MAXMCBTEMP = 31,
    LOWVOLTLIM = 32,
    HIGHVOLTLIM = 33,
    FULLRETRACT = 34,
    UDOCK = 35,
    CANCELMOTION = 36,

    // DIB Parameters
    MINDIBTEMP = 60,
    MAXDIBTEMP = 61,
    MINBATVOLT = 62,
    FIBERSWITCH = 63,
    EFUPERIOD = 64,
    EFUTIME = 65,
    FTRPOWER = 66,
    ETHERNETRESET = 67,
    FTRONTIME = 68,
    FTRCYCLETIME = 69,

    // FTR3000 Settings
    GETSTATUS = 90,
    READTEMP = 91,
    SETFIBPOS = 93,
    SETAVTIME = 94,

    // RACHUTS Settings
    SETAUTO = 95,
    SETMANUAL = 96,
    SETDWELLTIME = 97,

    // LPC Settings
    SETMODE = 100, // Expects mode enum
    SETSAMPLE = 101, // Number of samples per cycle
    SETWARMUPTIME = 102, // Time in seconds
    SETCYCLETIME = 103, // Time in minutes
    GETFILE = 104, // Requested frame number
    SETHGBINS = 105, // Number of bins followed by new bin values
    SETLGBINS = 106, // Number of bins followed by new bin values
    SETLASERTEMP = 107, // Target laser temp
    SETHKPERIOD = 108, // Time in minutes
    SETFLUSH = 109, // Time in seconds for air flush
    SETSAMPLEAVG = 110 // Values to average from PHA

};

struct DIB_Param_t {
    float minBatVolt;
    uint8_t fibSwitchState;
    uint16_t efuPeriod;
    uint8_t efuStart;
    uint8_t ftrPower;
    uint16_t ftrOnTime;
    uint16_t ftrCycleTime;
};

struct PIB_Param_t {
    uint8_t dwellTime;
};

struct LPC_Param_t {
    uint16_t samples;
    uint16_t samplesToAverage;
    uint16_t warmUpTime;
    uint8_t setCycleTime;
    uint32_t getFrameFile;
    uint8_t setHGBins;
    uint8_t newHGBins[24];
    uint8_t setLGBins;
    uint8_t newLGBins[24];
    uint8_t setLaserTemp;
    uint8_t hkPeriod;
    uint8_t lpc_flush;
};

struct MCB_Param_t {
    float deployLen;
    float retractLen;
    float deployVel;
    float retractVel;
    float deployAcc;
    float retractAcc;
    float positionErr;
    float reportedPosition;
    float minTemp;
    float maxTemp;
    float reportedTemp[6];
    float lowVoltLim;
    float highVoltLim;
    float reportedVoltage[4];
    uint8_t lowPowMode;
    uint8_t fullRetract;
    float dock; // left as float for distance, may not be used
    uint8_t telemRequest;
    uint8_t ackVal;
    uint8_t mcbState;
    uint8_t mcbSerial;
    float spoolLevel;
    float current[4];
    float brake;
    float torque[2];
};

#endif /* TELECOMMAND_H */