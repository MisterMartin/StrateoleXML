# StrateoleXML

This repository maintains XML interface code for [LASP](https://lasp.colorado.edu/home/) instruments flying on the [Stratéole 2](https://strat2.org/) campaign with CNES. The software is divided into two interface classes: `XMLReader` and `XMLWriter`. The LASP core software, [StratoCore](https://github.com/dastcvi/StratoCore), instantiates one of each.

## Serial Buffering

The XMLReader is designed to be called at 1 Hz with best-effort timing, and is responsible for reading the serial stream and parsing out messages. As such, there must be software buffering of the serial stream or messages will be missed. For this, we take advantage of the Teensy drivers' serial buffers. We need to modify the drivers, however, to increase the buffer size. The [PIBBufferGuard.h](https://github.com/dastcvi/StratoPIB/blob/master/PIBBufferGuard.h) file in StratoPIB contains comments explaining how to do this, and provides an example showing how to add *compile-time verification* that the buffers are the correct size.

## XMLReader

As stated, the XMLReader is responsible for reading the serial stream and parsing out messages. StratoCore implements most of this interface for the instrument, so these details will not be discussed. The instrument classes must, however, handle telecommands individually and know how to add/modify them.

### Telecommand Structure

The Stratéole 2 protocol for telecommands is to include a binary buffer of commands at the end of a `TC` XML message. For LASP instruments, we have elected to use ASCII-only telecommands to maintain human readability. Each telecommand is associated with a numerical ID, stored as an 8-bit unsigned integer (0-255). Telecommands can also contain comma-separated parameters. Telecommands are always followed by a semicolon. Thus, the format is:

Generic telecommand format: `<telecommand_id>,<param_1>,<param_2>,...,<param_n>;`

### Adding Telecommands

1. Add the telecommand name and unique ID number to the `Telecommand_t` enum in `Telecommand.h`
2. If there are parameters:
   1. Add them to the instrument struct (e.g. `PIB_Param_t`) in `Telecommand.h`
   2. Add a case for the TC to the `ParseTelecommand` function in `Telecommand.cpp`. *Note: the order in which the parameters are parsed is the order in which they must be sent in the telecommand*

### Handling Telecommands in an Instrument Class Derived from StratoCore

In StratoCore-derived classes, telecommands are handled in the `TCHandler` function defined as pure virtual in StratoCore. Add a case for each telecommand and perform handling (such as scheduling actions or setting configuration parameters). To access a telecommand parameter, just access its `Telecommand.h` struct. For example: `mcbParam.deployLen`.

## XMLWriter

The `XMLWriter` provides an interface that makes it easy to send all of the types of XML messages defined for Stratéole 2. For all messages except for telemetry messages, this is as simple as a function call:

```C++
void IMR(); // instrument mode request
void S(); // safety
void RA(); // RACHuTS authorization
void IMAck(bool ackval); // instrument mode acknowledgement
void TCAck(bool ackval); // telecommand acknowledgement
```

Sending messages with the `XMLWriter` can happen asynchronously anywhere in an instrument class's code.

### Telemetry Messages Without Data

Simple telemetry messages that do not require sending any data (e.g. run-time information or errors) can be sent with the `TM_String` function. However, it is more clear to use StratoCore's `ZephyrLogFine`, `ZephyrLogWarn`, and `ZephyrLogCrit` functions that serve as wrappers for the `TM_String` function.

### Telemetry Messages With Data

The Stratéole 2 XML protocol allows telemetry (`TM`) messages to contain up to 8192 bytes of binary-encoded data. The `XMLWriter` maintains a private, statically-allocated buffer of this size, and provides the following functions for adding data to the buffer. These functions can be called asynchronously, and data can be added to the buffer many loop cycles before it is sent.

```C++
bool addTm(uint8_t inChar);
bool addTm(uint16_t inWord);
bool addTm(uint32_t inDouble);
bool addTm(String inStr);
bool addTm(const uint8_t * buffer, uint16_t size);
bool addTm(const uint16_t * buffer, uint16_t size);
```

These functions will return false if there is an error adding the amount of data to the buffer. **Note that before each message, it is recommended to call the `clearTm` function to empty the buffer of its last message.**

XML `TM` messages also contain up to three State Flag + State Message pairs. They must contain at least one. The state flag specifies the severity of the message: `FINE`, `WARN`, or `CRIT`. If there isn't a message, `NOMESS` can be used. So, after the `TM` buffer has been written to and a message is ready to send, it can be sent as follows from a StratoCore class:

```C++
// there must be at least the first state flag/message pair
zephyrTX.setStateDetails(1, "I am fine, thanks");
zephyrTX.setStateFlagValue(1, FINE);

// we can se the other two as non-existent
zephyrTX.setStateFlagValue(2, NOMESS);
zephyrTX.setStateFlagValue(3, NOMESS);

zephyrTX.TM();
```