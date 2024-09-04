/*
 * InstInfo.h
 * Author:  Alex St. Clair
 * Created: August 2019
 *
 * Contains instrument definitions common between the XMLReader and XMLWriter
 */

#ifndef INSTINFO_H
#define INSTINFO_H

// used to index id string array
enum Instrument_t {
    FLOATS = 0,
    RACHUTS = 1,
    LPC = 2,
    RATS = 3
};

// indexed by Instrument_t enum, declared in XMLReader_v5.cpp
extern char inst_ids[4][8];

#endif /* INSTINFO_H */