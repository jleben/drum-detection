//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Performance Analysis Application Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <float.h>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "app.h"
#include "paa.h"
#include "file.h"
#include "midicsv.h"
#include "csv.h"
#include "map.h"

// I N T I A L I Z A T I O N 
float const Paa::cfOnsetTolerance    = 10.0f;
float const Paa::cfDynamicTolerance  = 10.0f;
float const Paa::cfOnsetLowerLimit   = 0.0f;
float const Paa::cfOnsetUpperLimit   = FLT_MAX;
float const Paa::cfDynamicLowerLimit = 0.0f;
float const Paa::cfDynamicUpperLimit = 1.0f;

// P U B L I C  M E T H O D S
Paa::Paa(int argc, char *argv[]) : App(argc, argv)
{
    // Set defaults
    mbVerbose           = false;
    mbConfusion         = false;
    mbMap               = false;
    mfOnsetTolerance    = cfOnsetTolerance;
    mfDynamicTolerance  = cfDynamicTolerance;
    mConfusion          = "";
    muOnsetMatch        = 0;
    muTypeMatch         = 0; 
    muDynamicMatch      = 0;
}

Paa::~Paa()
{
}

bool Paa::run(ostream &out)
{
    uint32_t         uIndex;
    float            fUpper;
    float            fLower;
    float            fOnsetAccuracy;
    float            fTypeAccuracy;
    float            fDynamicAccuracy;
    vector<trEvent>  reference;
    vector<trEvent>  measure;
    vector<trMap>    map;

    // Check for help request
    if (option(cOptionHelp))
    {
        usage(cerr);

        return true;
    }

    // Parse option[s]
    mbVerbose = option(cOptionVerbose);
    mbConfusion = option(cOptionConfusion, mConfusion);
    mbMap = option(cOptionMap, mMap);
    mbResynthesis = option(cOptionResynthesis, mResynthesis);
    option(cOptionOnsetTolerance, mfOnsetTolerance);
    option(cOptionDynamicTolerance, mfDynamicTolerance);

    // Parse argument[s]
    if (!argument(2, mReference) || !argument(1, mMeasure))
    {
        usage(cerr);

        return true;
    }

    // Check verbosity
    if (mbVerbose)
    {
        dump(cout);
    }

    // Acquire map
    if (!acquireMap(mMap, map))
    {
        cerr << "error: unable to acquire map" << endl;

        return false;
    }

    // Acquire events
    if (!acquireEvents(mReference, reference, map) || 
        !acquireEvents(mMeasure, measure, map))
    {
        cerr << "error: unable to acquire events" << endl;

        return false;
    }

    // Analyze event vectors
    for (uint32_t uEvent = 0; uEvent < reference.size(); uEvent++)
    {
        // Derive onset range
        range(reference.at(uEvent).fTimestamp, mfOnsetTolerance/1000.0f, 
            cfOnsetUpperLimit, cfOnsetLowerLimit, fUpper, fLower);

        // Iterate over measurements
        for (uIndex = 0; uIndex < measure.size(); uIndex++)
        {
            // Check for existing match
            if (!measure.at(uIndex).bMatch)
            {
                // Check for onset fit
                if ((measure.at(uIndex).fTimestamp >= fLower) && 
                    (measure.at(uIndex).fTimestamp <= fUpper))
                {
                    // Set reference and measurement
                    reference.at(uEvent).bMatch     = true;
                    reference.at(uEvent).uReference = uIndex;
                    measure.at(uIndex).bMatch       = true;
                    measure.at(uIndex).uReference   = uEvent;

                    // Update match counter
                    muOnsetMatch++;

                    // Check for type match
                    if (reference.at(uEvent).uType == measure.at(uIndex).uType)
                    {
                        muTypeMatch++;
                    }

                    // Derive dynamic range
                    range(reference.at(uEvent).fStrength, 
                        mfDynamicTolerance/100.0f, cfDynamicUpperLimit,
                        cfDynamicLowerLimit, fUpper, fLower);
                    
                    // Check for dynamic fit
                    if ((measure.at(uIndex).fStrength >= fLower) && 
                        (measure.at(uIndex).fStrength <= fUpper))
                    {
                        muDynamicMatch++;
                    }

                    break;
                }
            }
        }
    }

    // Compute statistics
    fOnsetAccuracy = (0 == reference.size()) ? 0 :
                        ((float)muOnsetMatch/(float)reference.size())*100.0f;
    fTypeAccuracy = (0 == muOnsetMatch) ? 0 :
                        ((float)muTypeMatch/(float)muOnsetMatch)*100.0f;
    fDynamicAccuracy = (0 == muOnsetMatch) ? 0 :
                        ((float)muDynamicMatch/(float)muOnsetMatch)*100.0f;

    // Display statistics
    out.precision(1);
    out.setf(ios::fixed, ios::floatfield);
    out << "onset accuracy: " << fOnsetAccuracy << "%" << endl;
    out << "type accuracy: " << fTypeAccuracy << "%" << endl;
    out << "dynamic accuracy: " << fDynamicAccuracy << "%" << endl;

    // Write to confusion matrix
    if (mbConfusion)
    {
        // Open file
        Csv confusion(mConfusion, File::eModeWrite);

        // Write confusion matrix entries
        for (uIndex = 0; uIndex < reference.size(); uIndex++)
        {
            // Write reference metrics
            confusion << reference.at(uIndex).bMatch << ",";
            confusion << reference.at(uIndex).uReference << ",";
            confusion << reference.at(uIndex).fTimestamp << ",";
            confusion << reference.at(uIndex).uType << ",";
            confusion << reference.at(uIndex).fStrength << ",";

            // Validate reference limit
            if (reference.at(uIndex).uReference < measure.size())
            {
                // Check for match
                if (reference.at(uIndex).bMatch)
                {
                    uint32_t uReference = reference.at(uIndex).uReference;

                    confusion << measure.at(uReference).fTimestamp << ",";
                    confusion << measure.at(uReference).uType << ",";
                    confusion << measure.at(uReference).fStrength << endl;
                } else
                {
                    confusion << "0,0,0" << endl;
                }
            } else
            {
                confusion << " " << endl;
            }
        }
    }

    // Resynthesize measurement
    if (mbResynthesis)
    {
        // Open file
        Midicsv resynthesis(mResynthesis, File::eModeWrite);

        // TEMP: Write header
        if (!resynthesis.headerWrite(960, 500000))
        {
            cerr << "error: unable to write resynthesis header" << endl;

            return false;
        }

        // Iterate over events
        for (uIndex = 0; uIndex < measure.size(); uIndex++)
        {
            // Copy event
            trEvent rEvent = measure.at(uIndex);

            // Inverse map the type
            for (uint32_t uEntry = 0; uEntry < map.size(); uEntry++)
            {
                // Check type
                if (rEvent.uType == map.at(uEntry).uOut)
                {
                    // Set type
                    rEvent.uType = map.at(uEntry).uIn;

                    break;
                }
            }

            // Write event
            if (!resynthesis.eventWrite(rEvent.fTimestamp, rEvent.uType,
                rEvent.fStrength))
            {
                cerr << "error: unable to write resynthesis event" << endl;

                return false;
            }
        }

        // Write footer
        if (!resynthesis.footerWrite(measure.at(measure.size()-1).fTimestamp))
        {
            cerr << "error: unable to write resynthesis footer" << endl;

            return false;        
        }
    }

    return true;
}

void Paa::usage(ostream &out)
{
    char buffer[80];

    sprintf(buffer, "usage: %s -[%c%c%c%c%c%c%c] <reference> <measure>\n",
        name().c_str(), cOptionVerbose, cOptionConfusion, cOptionMap,
        cOptionResynthesis, cOptionOnsetTolerance, cOptionDynamicTolerance, 
        cOptionHelp);
    out << buffer;
    sprintf(buffer, "where;\n");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionVerbose, "", "verbose");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionConfusion, "<filename>", 
        "comma seperated confusion output file");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionMap, "<filename>", 
        "comma seperated type map input file");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionResynthesis, "<filename>", 
        "comma seperated resynthesis output file");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionOnsetTolerance, "<ms>", 
        "onset tolerance");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionDynamicTolerance, "<%>", 
        "dynamics tolerance");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionHelp, "",
        "program help");
    out << buffer;
}

void Paa::dump(ostream &out)
{
    out << "onset tolerance: " << mfOnsetTolerance << "ms" << endl;
    out << "dynamics tolerance: " << mfDynamicTolerance << "%" << endl;
    out << "confusion: " << ((mbConfusion) ? mConfusion: "none") << endl;
    out << "map: " << ((mbMap) ? mMap: "none") << endl;
    out << "resynthesis: " << ((mbResynthesis) ? mResynthesis: "none") << endl;
    out << "reference: " << mReference << endl;
    out << "measure: " << mMeasure << endl;
}

// P R I V A T E  M E T H O D S
bool Paa::acquireMap(string name, vector<trMap> &map)
{
    trMap rMap;

    // Open file
    Map mapFile(name, File::eModeRead);
    if (!mapFile.valid())
    {
        return false;
    }

    // Read events
    while (mapFile.read(rMap.uIn, rMap.uOut))
    {
        map.push_back(rMap);
    }

    return true;
}

bool Paa::acquireEvents(string name, vector<trEvent> &onset, vector<trMap> map)
{
    uint32_t  uIndex;
    File     *pFile;
    trEvent   rEvent;

    // Try to open file using supported format[s]
    pFile = new Midicsv(name, File::eModeRead);
    if (!pFile->valid())
    {
        delete pFile;
        pFile = new Csv(name, File::eModeRead);
        if (!pFile->valid())
        {
            delete pFile;

            return false;
        }
    }

    // Read events
    while (pFile->eventRead(rEvent.fTimestamp, rEvent.uType, rEvent.fStrength))
    {
        // Check for non-zero signal strength
        if (rEvent.fStrength > 0.0f)
        {
            // Initialize attributes
            rEvent.bMatch     = false;
            rEvent.uReference = 0;

            // Map type
            for (uIndex = 0; uIndex < map.size(); uIndex++)
            {
                // Check type
                if (rEvent.uType == map.at(uIndex).uIn)
                {
                    // Set type
                    rEvent.uType = map.at(uIndex).uOut;

                    break;
                }
            }

            // Append event
            onset.push_back(rEvent);
        }
    }

    // Cleanup
    delete pFile;

    return true;
}

void Paa::range(float fValue, float fTolerance, float fUpperLimit,
               float fLowerLimit, float &fUpper, float &fLower)
{
    float fMargin;

    fMargin = fValue * fTolerance;
    fLower  = ((fValue-fMargin) < fLowerLimit) ? fLowerLimit : fValue-fMargin;
    fUpper  = ((fValue+fMargin) > fUpperLimit) ? fUpperLimit : fValue+fMargin;
}