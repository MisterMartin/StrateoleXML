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
    DEPLOYx = 1,          // Reel out, param0: deploy length in revolutions
    DEPLOYv = 2,          // Set deploy velocity, param0: deploy velocity in revs/sec
    DEPLOYa = 3,          // Set deploy acceleration, param0: deploy acceleration in revs/sec^2
    RETRACTx = 4,         // Reel in, param0: retract length in revolutions
    RETRACTv = 5,         // Set retract velocity, param0: retract velocity in revs/sec
    RETRACTa = 6,         // Set retract acceleration, param0: retract acceleration in revs/sec^2
    DOCKx = 7,            // Set dock length, param0: dock length in revolutions
    DOCKv = 8,            // Set dock velocity, param0: dock velocity in revs/sec
    DOCKa = 9,            // Set dock acceleration, param0: dock acceleration in revs/sec^2
    FULLRETRACT = 10,     // Full retract
    CANCELMOTION = 11,    // Cancel any ongoing motion
    ZEROREEL = 12,        // Zero the reel position
    TEMPLIMITS = 13,      // Set temperature limits, param0-5: unknown0, unknown1, unknown2, unknown3, unknown4, unknown5
    TORQUELIMITS = 14,    // Set torque limits, param0-1: unknown0, unknown1
    CURRLIMITS = 15,      // Set current limits, param0-1: unknown0, unknown1
    IGNORELIMITS = 16,    // Ignore limits
    USELIMITS = 17,       // Enable limits
    GETMCBEEPROM = 18,    // Get MCB EEPROM
    GETMCBVOLTS = 19,     // Get MCB voltages
    CONTROLLERSON = 20,   // Turn MCB controllers on
    CONTROLLERSOFF = 21,  // Turn MCB controllers off
    CENTERLW = 22,        // Center the level wind (MCB); no params

    // DIB Commands and Settings
    GOFTRFLIGHT = 50, // go to the flight FTR sub-mode
    GOMCBFLIGHT = 51, // go to the flight MCB sub-mode
    FTRCYCLETIME = 52,
    FTRONTIME = 53,
    SETDIBHKPERIOD = 54,
    FTRSTATUSLIMIT = 55,
    RAMANLEN = 56,
    SETMEASURETYPE = 57,

    // RATS Commands and Settings
    RATSECUDECIMATEFACTOR = 60, // Set ECU decimate factor, must be > 0, param0: 1==none, 2=every second one, etc.)
    RATSGETEEPROM = 61,         // Fetch the mainboard EEPROM
    RATSREALTIMEMCBON = 62,     // Enable real-time MCB reporting
    RATSREALTIMEMCBOFF = 63,    // Disable real-time MCB reporting
    RATSLORATXTESTON = 64,      // Enable LoRa TX test mode
    RATSLORATXTESTOFF = 65,     // Disable LoRa TX test mode
    RATSECUTEMP = 66,           // Set the ECU temperature setpoint, param0: temperature in C
    RATSECUPWRON = 67,          // Manual ECU power on
    RATSECUPWROFF = 68,         // Manual ECU power off
    RATSRS41REGEN = 69,         // RS41 regeneration on. Command is sent to ECU; it will put the RS41 in regen mode, which will timeout later.
    RATSRS41ENON = 70,          // RS41 power on. Command is sent to ECU
    RATSRS41ENOFF = 71,         // RS41 power off. Command is sent to ECU
    RATSTSENPOWON = 72,         // TSEN power on. Command is sent to ECU
    RATSTSENPOWOFF = 73,        // TSEN power off. Command is sent to ECU
    RATSECURS41METADATA = 74,   // Request RS41 metadata from ECU
    RATSPAIREDCEU = 75,         // Set the paired ECU ID, param0: ECU ID (uint8_t)
    RATSINFO = 76,              // Send a RATSTEXT TM with firmware version and LoRa config
    RATSSETMOTIONTIMEOUT = 77,  // Set MCB motion timeout, param0: timeout in seconds (uint16)
    RATSLORASUSPEND = 78,       // Suspend ECU LoRa TX, param0: duration in seconds (uint16); 0 cancels a suspend

    // LPC Settings
    SETMODE = 100,       // Expects mode enum
    SETSAMPLE = 101,     // Number of samples per cycle
    SETWARMUPTIME = 102, // Time in seconds
    SETCYCLETIME = 103,  // Time in minutes
    GETFILE = 104,       // Requested frame number
    SETHGBINS = 105,     // Number of bins followed by new bin values
    SETLGBINS = 106,     // Number of bins followed by new bin values
    SETLASERTEMP = 107,  // Target laser temp
    SETHKPERIOD = 108,   // Time in minutes
    SETFLUSH = 109,      // Time in seconds for air flush
    SETSAMPLEAVG = 110,  // Values to average from PHA
    // IDs 111-115 are defined in TCMessage.py, but not here
    SETPHA = 116,        // Pulse height analyzer parameters
    REGENRS41 = 117,     // Initiate an RS41 regeneration
    SETFLOW = 118,       // set the BEMF setpoint for both pumps
    SETPUMPTEMP = 119,   // set the minimum temperature for the pumps
    SETRS41RATE = 120,   // Set RS41 sample period, param0: period in seconds (uint16)

    // RACHUTS Commands and Settings
    // Autonomous mode was removed from RACHUTS (2026-07). Its telecommands are
    // commented out below rather than deleted: the IDs (130-132, 136-140) stay
    // reserved so the numbering is preserved, and any other app still referencing
    // them fails to compile -- surfacing a latent dependency. Do not reuse the IDs.
    // SETAUTO = 130,           // Switch to autonomous flight mode (restarts flight mode)
    // SETMANUAL = 131,         // Switch to manual flight mode (restarts flight mode)
    // SETSZAMIN = 132,         // Set minimum SZA for autonomous profile trigger. param0: SZA (float)
    SETPROFILESIZE = 133,       // Set profile deploy length. param0: size (float, reel revolutions)
    SETDOCKAMOUNT = 134,        // Set dock retract length. param0: amount (float, revolutions)
    SETDWELLTIME = 135,         // Set dwell time at profile bottom. param0: time (uint16, seconds)
    // SETPROFILEPERIOD = 136,  // Set period between autonomous profiles. param0: period (uint16, seconds)
    // SETNUMPROFILES = 137,    // Set number of profiles per autonomous session. param0: count (uint)
    // USESZATRIGGER = 138,     // Use SZA threshold to trigger autonomous profiles
    // USETIMETRIGGER = 139,    // Use time trigger for autonomous profiles
    // SETTIMETRIGGER = 140,    // Set Unix timestamp for autonomous profile trigger. param0: timestamp (uint32)
    SETDOCKOVERSHOOT = 141,     // Set dock overshoot distance. param0: overshoot (float, revolutions)
    RETRYDOCK = 142,            // Manual redock command. param0: deploy length (rev), param1: retract length (rev)
    GETPUSTATUS = 143,          // Request RPU status via dock serial
    PUPOWERON = 144,            // Enable RPU dock power
    PUPOWEROFF = 145,           // Disable RPU dock power
    PROFILE = 146,              // Execute a profile. param0: profile size (rev), param1: dock amount (rev), param2: dock overshoot (rev), param3: dwell time (s), param4: RPU sample rate (s)
    OFFLOADPUPROFILE = 147,     // Offload stored RPU profile data
    SETPREPROFILETIME = 148,    // Set pre-profile wait time after RPU enters measure mode. param0: time (uint16, seconds)
    AUTOREDOCKPARAMS = 150,     // Set auto-redock parameters. param0: redock out (rev), param1: redock in (rev), param2: max retries
    SETMOTIONTIMEOUT = 151,     // Set motion timeout. param0: timeout (uint16, seconds)
    GETPIBEEPROM = 152,         // Request PIB EEPROM contents as TM
    DOCKEDPROFILE = 153,        // Execute a docked profile. param0: duration (s), param1: sample rate (s)
    STARTREALTIMEMCB = 154,     // Enable real-time MCB data streaming mode
    EXITREALTIMEMCB = 155,      // Disable real-time MCB data streaming mode
    CANCELMEASURE = 156,        // Cancel an in-progress docked profile (RPU measurement); no params. Will also be wired to cancel a manual profile in the future.
    SETDOCKEDOFFLOADPERIOD = 157, // Set the docked profile's periodic offload interval. param0: period (uint16, seconds; 0 = offload once at the end, the legacy behavior)
    RAACKOVERRIDEON = 158,      // Bypass the RA-ack requirement (emergency use, not persisted, resets to off on reboot)
    RAACKOVERRIDEOFF = 159,     // Restore the normal RA-ack requirement

    // RPU commands and settings
    RPUCONFIG = 180,        // Configure RPU sensor enables. param0: enable ROPC, param1: enable TDLAS, param2: enable TSEN, param3: enable RS41
    RPUSTATUSPERIOD = 181,  // Set the period for RPU status reports, in seconds. param0: period in seconds
    RPUBATTEMP = 182,       // Set RPU battery temperature threshold. param0: temperature (float, degC)
    RPURESET = 183,         // Reboot the RPU via dock serial

    // Development testing only - not used in flight operations
    RPUGOSTANDBY = 184,     // Go to STANDBY mode
    RPUGOMEASURE = 185,     // Send go-measure command to RPU. param0: duration (s), param1: sample rate (s). Sensor enables/batt temp from stored config.

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
    float profileSize;
    float dockAmount;
    float dockOvershoot;
    float autoRedockOut;
    float autoRedockIn;
    uint16_t dwellTime;
    uint16_t sampleRate;        // RPU measurement sample rate for a manual profile (s)
    uint16_t preprofileTime;
    uint16_t dockedProfileTime;
    uint16_t dockedProfileRate;
    uint16_t dockedOffloadPeriod;
    uint8_t numRedock;
    uint8_t motionTimeout;
};

struct RATS_Param_t {
    uint8_t decimate_factor;       // must be > 0; 1==none, 2==every second one, etc.
    uint16_t deploy_revs;
    uint16_t deploy_velocity;
    uint16_t retract_revs;
    uint16_t retract_velocity;
    float ecu_tempC;
    uint8_t paired_ecu;
    uint16_t motion_timeout;
    uint16_t lora_suspend_sec;
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
    float flowSetpoint;
    float pumpMinTemp;
    uint16_t rs41SamplePeriod;
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

struct RPU_Param_t {
    // GO_MEASUREMENT settings
    uint16_t measDurationSecs;
    uint16_t measRateSecs;
    uint8_t enableROPC;
    uint8_t enableTDLAS;
    uint8_t enableTSEN;
    uint8_t enableRS41;

    // STATUSPERIOD setting
    uint16_t statusPeriodSecs;

    // RPUBATTEMP setting
    float batTemp;
};

#endif /* TELECOMMAND_H */