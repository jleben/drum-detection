//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// MIDI Class Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <string.h>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "file.h"
#include "midicsv.h"

// C O N S T A N T S
const char *Midicsv::spcEventNoteOn     = "Note_on_c";
const char *Midicsv::spcEventHeader     = "Header";
const char *Midicsv::spcEventTempo      = "Tempo";
const char *Midicsv::spcEventStartTrack = "Start_track";
const char *Midicsv::spcEventEndTrack   = "End_track";
const char *Midicsv::spcEventEndOfFile  = "End_of_file";

// P U B L I C  M E T H O D S
Midicsv::Midicsv(string name, File::teMode eMode) : File(name, eMode)
{
    uint32_t uFormat;
    uint32_t uTracks;
    uint32_t uDivision;
    uint32_t uTempo;

    // Check mode
    if (eModeRead == eMode)
    {
        // Retrive meta data and validate file
        mbValid  = header(uFormat, uTracks, uDivision);
        mbValid &= tempo(uTempo);
        mbValid &= (scuSupportedFormat == uFormat) ? true : false;

        // Compute clock tick period
        mfTick = (float)((float)uTempo/1000000.0f/(float)uDivision);
    } else
    {
        // New file
        mbValid = true;
    }
}

Midicsv::~Midicsv()
{
}

bool Midicsv::valid()
{
    return (File::valid() && mbValid) ? true : false;
}

bool Midicsv::eventRead(float &fTimestamp, uint32_t &uType, float &fStrength)
{
    uint32_t uTimestamp;
    uint32_t uNote;
    uint32_t uVelocity;

    // Get note on event
    if (noteOn(uTimestamp, uNote, uVelocity))
    {
        fTimestamp     = uTimestamp*mfTick;
        uType          = uNote;
        fStrength      = (float)((float)uVelocity/127.0f);

        return true;
    }

    return false;
}

bool Midicsv::eventWrite(float fTimestamp, uint32_t uType, float fStrength)
{
    uint32_t uDelta;
    uint32_t uNote;
    uint32_t uVelocity;
    float    fTick;
    char     buffer[80];

    // Compute tick
    fTick = (float)((float)muTempo/1000000.0f/(float)muDivision);

    // Translate to MIDI
    uDelta    = (uint32_t)(fTimestamp/fTick);
    uNote     = uType;
    uVelocity = (uint32_t)(127*fStrength);

    // Format and write
    sprintf(buffer, "%u, %u, %s, %u, %u, %u", 2, uDelta, spcEventNoteOn, 9, 
        uNote, uVelocity);
    linePut(buffer);

    return true;
}

bool Midicsv::headerWrite(uint32_t uDivision, uint32_t uTempo)
{
    char buffer[80];

    // Store arguments
    muDivision = uDivision;
    muTempo    = uTempo;

    // Format and write header
    sprintf(buffer, "%u, %u, %s, %u, %u, %u", 0, 0, spcEventHeader, 
        scuSupportedFormat, 2, uDivision);
    linePut(buffer);

    // Format and write start track
    sprintf(buffer, "%u, %u, %s", 1, 0, spcEventStartTrack);
    linePut(buffer);

    // Format and write tempo
    sprintf(buffer, "%u, %u, %s, %u", 1, 0, spcEventTempo, uTempo);
    linePut(buffer);

    // Format and write end track
    sprintf(buffer, "%u, %u, %s", 1, 0, spcEventEndTrack);
    linePut(buffer);

    // Format and write start track
    sprintf(buffer, "%u, %u, %s", 2, 0, spcEventStartTrack);
    linePut(buffer);

    return true;
}

bool Midicsv::footerWrite(float fTimestamp)
{
    uint32_t uDelta;
    float    fTick;
    char     buffer[80];

    // Compute tick
    fTick = (float)((float)muTempo/1000000.0f/(float)muDivision);

    // Translate to MIDI
    uDelta    = (uint32_t)(fTimestamp/fTick);

    // Format and write end track
    sprintf(buffer, "%u, %u, %s", 2, uDelta, spcEventEndTrack);
    linePut(buffer);

    // Format and write end of file
    sprintf(buffer, "%u, %u, %s\n", 0, 0, spcEventEndOfFile);
    linePut(buffer);

    return true;
}

bool Midicsv::header(uint32_t &uFormat, uint32_t &uTracks, uint32_t &uDivision)
{
    uint32_t uDummy;

    return event(spcEventHeader, uDummy, uDummy, uFormat, uTracks, uDivision,
            uDummy);
}

bool Midicsv::tempo(uint32_t &uValue)
{
    uint32_t uDummy;

    return event(spcEventTempo, uDummy, uDummy, uValue, uDummy, uDummy,
            uDummy);
}

// P R I V A T E  M E T H O D S
bool Midicsv::event(string name, uint32_t &uTrack, uint32_t &uTimestamp,
                    uint32_t &uArg0, uint32_t &uArg1, uint32_t &uArg2, 
                    uint32_t &uArg3)
{
    uint16_t uColumn;
    string   line;
    string   field;
    string   tag;

   // Search for event
    while (lineGet(line))
    {
        // Remove white spaces
        line.erase(remove_if(line.begin(), line.end(), ::isspace),line.end());

        // Create stream for tokens
        stringstream ss(line);

        // Process fields
        uColumn = 0;
        while (getline(ss, field, ','))
        {
            // Process field based on column
            switch (uColumn)
            {
                case 0:

                    stringstream(field) >> uTrack;
                    break;

                case 1:

                    stringstream(field) >> uTimestamp;
                    break;

                case 2:

                    tag = field;
                    break;

                case 3:

                    stringstream(field) >> uArg0;
                    break;

                case 4:

                    stringstream(field) >> uArg1;
                    break;

                case 5:

                    stringstream(field) >> uArg2;
                    break;

                case 6:

                    stringstream(field) >> uArg3;
                    break;

                default:

                    // Ignore field
                    break;
            }

            // Update column
            uColumn++;
        }

        // Check tag
        if (tag == name)
        {
            return true;
        }
    }

    return false;
}

bool Midicsv::noteOn(uint32_t &uTimestamp, uint32_t &uNote, uint32_t &uVelocity)
{
    uint32_t uDummy;

    return event(spcEventNoteOn, uDummy, uTimestamp, uDummy, uNote, uVelocity,
            uDummy);
}
