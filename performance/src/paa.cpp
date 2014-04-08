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
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <float.h>
#include <stdexcept>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "app.h"
#include "paa.h"
#include "file.h"
#include "midicsv.h"
#include "csv.h"
#include "midifile.h"
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
    mbListing           = false;
    mbMap               = false;
    mfOnsetTolerance    = cfOnsetTolerance;
    mfDynamicTolerance  = cfDynamicTolerance;
    mListing            = "";
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
    float            fOnsetUpper;
    float            fOnsetLower;
	float            fDynamicUpper;
	float            fDynamicLower;
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
    mbListing = option(cOptionListing, mListing);
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

	// Check verbosity
	if (mbVerbose)
	{
		cout << "map entries: " << map.size() << endl;
	}

    try {
        acquireEvents(mReference, reference, map);
    }
    catch (std::exception & e)
    {
        cerr << "Error: Can not read reference event file: " << mReference << endl;
        cerr << "Resone: " << e.what() << endl;
        return false;
    }

    try {
        acquireEvents(mMeasure, measure, map);
    }
    catch (std::exception & e)
    {
        cerr << "Error: Can not read detected event file: " << mMeasure << endl;
        cerr << "Resone: " << e.what() << endl;
        return false;
    }

	// Check verbosity
	if (mbVerbose)
	{
		cout << "reference events: " << reference.size() << endl;
		cout << "measure events: " << measure.size() << endl;
	}

    // Match events by time and type

    // Iterate over reference events
    for (int referenceIndex = 0; referenceIndex < reference.size(); referenceIndex++)
    {
        trEvent & referenceEvent = reference[referenceIndex];

        // Derive onset range
        range(referenceEvent.fTimestamp, mfOnsetTolerance/1000.0f,
              cfOnsetUpperLimit, cfOnsetLowerLimit, fOnsetUpper, fOnsetLower);

        // Iterate over detected events
        for (int detectedIndex = 0; detectedIndex < measure.size(); detectedIndex++)
        {
            trEvent & detectedEvent = measure[detectedIndex];

            // Compare time
            bool time_match =
                    (detectedEvent.fTimestamp >= fOnsetLower) &&
                    (detectedEvent.fTimestamp <= fOnsetUpper);
            if (!time_match)
                continue;

            // Compare type
            bool type_match = (referenceEvent.uType == detectedEvent.uType);

            referenceEvent.bMatch = true;
            referenceEvent.uReference = detectedIndex;

            if (type_match)
                break;
        }

        // Only update the best matched detected event
        if (referenceEvent.bMatch)
        {
            trEvent & matchingDetectedEvent = measure[referenceEvent.uReference];
            matchingDetectedEvent.bMatch = true;
            matchingDetectedEvent.uReference = referenceIndex;
        }
    }

    // Count matches.

    // Iterate over reference events:
    for (int referenceIndex = 0; referenceIndex < reference.size(); referenceIndex++)
    {
        trEvent & referenceEvent = reference[referenceIndex];

        // Check time match
        if (!referenceEvent.bMatch)
            continue;

        muOnsetMatch++;

        trEvent & detectedEvent = measure[referenceEvent.uReference];

        // Check type match
        if (referenceEvent.uType == detectedEvent.uType)
            muTypeMatch++;

        // Derive dynamic range
        range(referenceEvent.fStrength,
              referenceEvent.fStrength*(mfDynamicTolerance/100.0f),
              cfDynamicUpperLimit, cfDynamicLowerLimit,
              fDynamicUpper, fDynamicLower);

        // Check strength match
        if ( (detectedEvent.fStrength >= fDynamicLower) &&
             (detectedEvent.fStrength <= fDynamicUpper) )
        {
            muDynamicMatch++;
        }
    }

	// Check verbosity
	if (mbVerbose)
	{
		cout << "onset match: " << muOnsetMatch << endl;
		cout << "type match: " << muTypeMatch << endl;
		cout << "dynamic match: " << muDynamicMatch << endl;
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

	// Display confusion matrix
	confusionMatrix(cout, reference, measure, map);

    // Write to listing file
    if (mbListing)
    {
        // Open file
        Csv listing(mListing, File::eModeWrite);

        // Write listing entries
        for (uIndex = 0; uIndex < reference.size(); uIndex++)
        {
            // Write reference metrics
            listing << reference.at(uIndex).bMatch << ",";
            listing << reference.at(uIndex).uReference << ",";
            listing << reference.at(uIndex).fTimestamp << ",";
            listing << reference.at(uIndex).uType << ",";
            listing << reference.at(uIndex).fStrength << ",";

            // Validate reference limit
            if (reference.at(uIndex).uReference < measure.size())
            {
                // Check for match
                if (reference.at(uIndex).bMatch)
                {
                    uint32_t uReference = reference.at(uIndex).uReference;

                    listing << measure.at(uReference).fTimestamp << ",";
                    listing << measure.at(uReference).uType << ",";
                    listing << measure.at(uReference).fStrength << endl;
                } else
                {
                    listing << "0,0,0" << endl;
                }
            } else
            {
                listing << " " << endl;
            }
        }
    }

    // Resynthesize measurement
    if (mbResynthesis)
    {
        // Open reference file in order to retrive time division and tempo
		Midifile ref(mReference, File::eModeBinaryRead);
		if (!ref.valid())
		{
			cerr << "error: unable to open reference" << endl;

			return false;
		}

		// Open file
		Midifile resynthesis(mResynthesis, File::eModeBinaryWrite);
		if (!resynthesis.valid())
		{
			cerr << "error: unable to open resynthesis" << endl;

			return false;
		}

        // Write header
        if (!resynthesis.headerWrite(ref.division(), ref.tempo()))
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
        name().c_str(), cOptionVerbose, cOptionListing, cOptionMap,
        cOptionResynthesis, cOptionOnsetTolerance, cOptionDynamicTolerance, 
        cOptionHelp);
    out << buffer;
    sprintf(buffer, "where;\n");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionVerbose, "", "verbose");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionListing, "<filename>", 
        "comma seperated listing output file");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionMap, "<filename>", 
        "comma seperated type map input file");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionResynthesis, "<filename>", 
        "MIDI format resynthesis output file");
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
	sprintf(buffer, " %8c %-16s %-32s\n", ' ', "reference",
		"MIDI format reference input file");
	out << buffer;
	sprintf(buffer, " %8c %-16s %-32s\n", ' ', "measure",
		"CSV format detection input file");
	out << buffer;
}

void Paa::dump(ostream &out)
{
    out << "onset tolerance: " << mfOnsetTolerance << "ms" << endl;
    out << "dynamics tolerance: " << mfDynamicTolerance << "%" << endl;
    out << "listing: " << ((mbListing) ? mListing: "none") << endl;
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

void Paa::acquireEvents(string name, vector<trEvent> &onset, vector<trMap> map)
{
    uint32_t  uIndex;
    File     *pFile;
    trEvent   rEvent;

    // Try to open file using supported format[s]
    pFile = new Midifile(name, File::eModeBinaryRead);

    if (!pFile->valid())
    {
        delete pFile;
        pFile = new Midicsv(name, File::eModeRead);
    }

    if (!pFile->valid())
    {
        delete pFile;
        pFile = new Csv(name, File::eModeRead);
    }

    if (!pFile->valid())
    {
        throw std::runtime_error("Invalid file format or inexistent file.");
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
}

void Paa::range(float fValue, float fTolerance, float fUpperLimit,
               float fLowerLimit, float &fUpper, float &fLower)
{
    fLower  = ((fValue-fTolerance) < fLowerLimit) ? fLowerLimit : fValue-fTolerance;
	fUpper = ((fValue + fTolerance) > fUpperLimit) ? fUpperLimit : fValue + fTolerance;
}

int typeIndex( int type, vector<int> & types )
{
    for (int i = 0; i < types.size(); ++i)
    {
        if (type == types[i])
            return i;
    }
    return -1;
}

void Paa::confusionMatrix(ostream &out, vector<trEvent> &reference, vector<trEvent> &measure, vector<trMap> &map)
{
    vector<int> types;

    // Build type vector
    for (uint32_t uIndex = 0; uIndex < map.size(); uIndex++)
    {
        vector<int>::iterator i = find(types.begin(), types.end(), map.at(uIndex).uOut);
        if (i == types.end())
        {
            types.push_back(map.at(uIndex).uOut);
        }
    }

    int total_cols = types.size() + 1;
    int total_rows = types.size() + 2;

    int missed_col = total_cols - 1;
    int unmapped_row = total_rows - 2;
    int ghost_row = total_rows - 1;

    vector< vector<int> > matrix;

    for (int i = 0; i < total_rows; ++i)
    {
        matrix.emplace_back( total_cols, 0 );
    }

    for (int refIndex = 0; refIndex < reference.size(); ++refIndex)
    {
        int row = unmapped_row;
        int col = missed_col;

        trEvent & refEvent = reference[refIndex];
        row = typeIndex(refEvent.uType, types);

        if (refEvent.bMatch)
        {
            trEvent & detectedEvent = measure[refEvent.uReference];
            col = typeIndex(detectedEvent.uType, types);
            if (col == -1)
                col = missed_col;
        }

        matrix[row][col]++;
    }

    for (int detIndex = 0; detIndex < measure.size(); ++detIndex)
    {
        trEvent & detectedEvent = measure[detIndex];
        if (!detectedEvent.bMatch)
        {
            int col = typeIndex(detectedEvent.uType, types);
            if (col == -1)
                col = missed_col;

            matrix[ghost_row][col]++;
        }
    }

    cout << "confusion matrix:" << endl;

    cout << setw(8) << "ref/det";
    for (int i = 0; i < types.size(); ++i)
        cout << setw(8) << types[i];
    cout << setw(8) << "miss";
    cout << endl;

    for (int r = 0; r < types.size(); ++r)
    {
        cout << setw(8) << types[r];
        for (int c = 0; c < total_cols; ++c)
        {
            cout << setw(8) << matrix[r][c];
        }
        cout << endl;
    }

    cout << setw(8) << "unknown";
    for (int c = 0; c < total_cols; ++c)
    {
        cout << setw(8) << matrix[unmapped_row][c];
    }
    cout << endl;

    cout << setw(8) << "ghost";
    for (int c = 0; c < types.size(); ++c)
    {
        cout << setw(8) << matrix[ghost_row][c];
    }
    cout << endl;
}
