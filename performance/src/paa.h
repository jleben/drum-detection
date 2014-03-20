//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Performance Analysis Application Class Definition
//
#ifndef _PAA_H
#define _PAA_H

// C L A S S
class Paa : public App
{
public:

    // Constant[s]
    static char  const cOptionListing          = 'l';
    static char  const cOptionDynamicTolerance = 'd';
    static char  const cOptionHelp             = 'h';
    static char  const cOptionMap              = 'm';
    static char  const cOptionOnsetTolerance   = 'o';
    static char  const cOptionResynthesis      = 'r';
    static char  const cOptionVerbose          = 'v';
    static float const cfOnsetTolerance;
    static float const cfDynamicTolerance;
    static float const cfOnsetLowerLimit;
    static float const cfOnsetUpperLimit;
    static float const cfDynamicLowerLimit;
    static float const cfDynamicUpperLimit;

    // Constructor[s]
    Paa(int argc, char *argv[]);

    // Destructor
    ~Paa();

    // Virtual Method[s];
    virtual void dump(ostream &out);

    // Method[s]
    bool run(ostream &out);
    void usage(ostream &out);

private:

    // Structure[s]
    typedef struct
    {
        bool     bMatch;
        uint32_t uReference;
        float    fTimestamp;
        uint32_t uType;
        float    fStrength;
    } trEvent;
    typedef struct
    {
        uint32_t uIn;
        uint32_t uOut;
    } trMap;

    // Method[s]
    bool acquireMap(string name, vector<trMap> &map);
    bool acquireEvents(string name, vector<trEvent> &onset, vector<trMap> map);
    void range(float fValue, float fTolerance, float fUpperLimit,
               float fLowerLimit, float &fUpper, float &fLower);
	void confusionMatrix(ostream &out, vector<trEvent> &reference, vector<trEvent> &measure, vector<trMap> &map);

    // Data
    bool     mbVerbose;
    bool     mbListing;
    bool     mbMap;
    bool     mbResynthesis;
    float    mfOnsetTolerance;
    float    mfDynamicTolerance;
    uint32_t muOnsetMatch;
    uint32_t muDynamicMatch;
    uint32_t muTypeMatch;
    string   mListing;
    string   mReference;
    string   mMeasure;
    string   mMap;
    string   mResynthesis;
};

#endif // _PAA_H