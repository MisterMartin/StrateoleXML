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
    NULL_TELECOMMAND = 0,

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
    TEMPLIMITS = 13,
    TORQUELIMITS = 14,
    CURRLIMITS = 15,
    IGNORELIMITS = 16,
    USELIMITS = 17,
    GETMCBEEPROM = 18,

    // DIB Commands and Settings
    GOFTRFLIGHT = 50, // go to the flight FTR sub-mode
    GOMCBFLIGHT = 51, // go to the flight MCB sub-mode
    FTRCYCLETIME = 52,
    FTRONTIME = 53,
    SETDIBHKPERIOD = 54,
    FTRSTATUSLIMIT = 55,
    RAMANLEN = 56,
    SETMEASURETYPE = 57,

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
    // IDs 111-115 are defined in TCMessage.py, but not here
    SETPHA = 116, // Pulse height analyzer parameters
    REGENRS41 = 117, // Initiate an RS41 regeneration

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
    RETRYDOCK = 142,
    GETPUSTATUS = 143,
    PUPOWERON = 144,
    PUPOWEROFF = 145,
    MANUALPROFILE = 146,
    OFFLOADPUPROFILE = 147,
    SETPREPROFILETIME = 148,
    SETPUWARMUPTIME = 149,
    AUTOREDOCKPARAMS = 150,
    SETMOTIONTIMEOUT = 151,
    GETPIBEEPROM = 152,
    DOCKEDPROFILE = 153,
    STARTREALTIMEMCB = 154,
    EXITREALTIMEMCB = 155,

    // PU commands and settings
    PUWARMUPCONFIGS = 180,
    PUPROFILECONFIGS = 181,
    PURESET = 182,
    PUDOCKEDCONFIGS = 183,

    // Generic instrument commands
    RESET_INST = 200,
    EXITERROR = 201,
    GETTMBUFFER = 202,
    SENDSTATE = 203,
};

struct DIB_Param_t {
    uint16_t ftrOnTime;
    uint16_t ftrCycleTime;
    uint16_t hkPeriod;
    uint16_t statusLimit;
    uint16_t ramanScanLength;
    uint8_t ftrMeasureType;
    uint8_t ftrBurstLim;
};

struct PIB_Param_t {
    float szaMinimum;
    float profileSize;
    float dockAmount;
    float dockOvershoot;
    float autoRedockOut;
    float autoRedockIn;
    uint32_t timeTrigger;
    uint16_t dwellTime;
    uint16_t profilePeriod;
    uint16_t preprofileTime;
    uint16_t warmupTime;
    uint16_t dockedProfileTime;
    uint8_t numProfiles;
    uint8_t numRedock;
    uint8_t motionTimeout;
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
    uint16_t phaHiGainThreshold;
    uint16_t phaHiGainOffset;
    uint16_t phaLoGainOffset;
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
    float tempLimits[6];
    float torqueLimits[2];
    float currLimits[2];
};

struct PU_Param_t {
    // warmup parameters
    float flashT;
    float heater1T;
    float heater2T;
    uint8_t flashPower;
    uint8_t tsenPower;

    // profile settings
    uint32_t profileRate;
    uint32_t dwellRate;
    uint8_t profileTSEN;
    uint8_t profileROPC;
    uint8_t profileFLASH;

    // docked profile settings
    uint32_t dockedRate;
    uint8_t dockedTSEN;
    uint8_t dockedROPC;
    uint8_t dockedFLASH;
};

#endif /* TELECOMMAND_H */