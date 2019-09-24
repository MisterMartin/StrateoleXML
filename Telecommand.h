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

    // MCB commands and parameters
    DEPLOYx = 1,
    DEPLOYv = 2,
    DEPLOYa = 3,
    RETRACTx = 4,
    RETRACTv = 5,
    RETRACTa = 6,
    DOCKx = 7,
    DOCKv = 8,
    DOCKa = 9,
    FULLRETRACT = 10,
    CANCELMOTION = 11,
    ZEROREEL = 12,

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
    SETSAMPLEAVG = 110, // Values to average from PHA

    // RACHUTS Commands and Settings
    SETAUTO = 130,
    SETMANUAL = 131,
    SETSZAMIN = 132,
    SETPROFILESIZE = 133,
    SETDOCKAMOUNT = 134,
    SETDWELLTIME = 135,
    SETPROFILEPERIOD = 136,
    SETNUMPROFILES = 137,
    USESZATRIGGER = 138,
    USETIMETRIGGER = 139,
    SETTIMETRIGGER = 140,
    SETDOCKOVERSHOOT = 141,

    // Generic instrument commands
    RESET_INST = 200,
    EXITERROR = 201,
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
    float szaMinimum;
    float profileSize;
    float dockAmount;
    float dockOvershoot;
    uint32_t timeTrigger;
    uint16_t dwellTime;
    uint16_t profilePeriod;
    uint8_t numProfiles;
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
    float deployVel;
    float deployAcc;
    float retractLen;
    float retractVel;
    float retractAcc;
    float dockLen;
    float dockVel;
    float dockAcc;
};

#endif /* TELECOMMAND_H */