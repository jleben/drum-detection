//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// MIDI CSV Class Definition
//
#ifndef _MIDICSV_H
#define _MIDICSV_H

// C L A S S
class Midicsv : public File
{
public:  

    // Constant[s]
    static const char *spcEventHeader;
    static const char *spcEventTempo;
    static const char *spcEventNoteOn;
    static const char *spcEventStartTrack;
    static const char *spcEventEndTrack;
    static const char *spcEventEndOfFile;

    static const uint32_t scuSupportedFormat = 1;

    // Constructor[s]
    Midicsv(string name, File::teMode eMode);

    // Destructor
    ~Midicsv();

    // Method[s]
    bool valid();
    bool eventRead(float &fTimestamp, uint32_t &uType, float &fStrength);
    bool eventWrite(float fTimestamp, uint32_t uType, float fStrength);
    bool header(uint32_t &uFormat, uint32_t &uTracks, uint32_t &uDivision);
    bool headerWrite(uint32_t uDivision, uint32_t uTempo);
    bool footerWrite(float fTimestamp);
    bool tempo(uint32_t &uValue);

private:

    // Method[s]
    bool event(string name, uint32_t &uTrack, uint32_t &uTimestamp,
               uint32_t &uArg0, uint32_t &uArg1, uint32_t &uArg2, 
               uint32_t &uArg3);
    bool noteOn(uint32_t &uTimestamp, uint32_t &uNote, uint32_t &uVelocity);

    // Data
    bool     mbValid;
    uint32_t muDivision;
    uint32_t muTempo;
    float    mfTick;
};

#endif // _MIDICSV_H