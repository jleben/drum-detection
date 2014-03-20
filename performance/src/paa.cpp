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
#include <algorithm>
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

    // Acquire events
    if (!acquireEvents(mReference, reference, map) || 
        !acquireEvents(mMeasure, measure, map))
    {
        cerr << "error: unable to acquire events" << endl;

        return false;
    }

	// Check verbosity
	if (mbVerbose)
	{
		cout << "reference events: " << reference.size() << endl;
		cout << "measure events: " << measure.size() << endl;
	}

    // Analyze event vectors
    for (uint32_t uEvent = 0; uEvent < reference.size(); uEvent++)
    {
        // Derive onset range
        range(reference.at(uEvent).fTimestamp, mfOnsetTolerance/1000.0f, 
            cfOnsetUpperLimit, cfOnsetLowerLimit, fOnsetUpper, fOnsetLower);

        // Iterate over measurements
        for (uIndex = 0; uIndex < measure.size(); uIndex++)
        {
            // Check for onset fit
            if ((measure.at(uIndex).fTimestamp >= fOnsetLower) && 
                (measure.at(uIndex).fTimestamp <= fOnsetUpper))
            {
				// Check for previous match
				if (!measure.at(uIndex).bMatch)
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
						reference.at(uEvent).fStrength*(mfDynamicTolerance/100.0f), 
						cfDynamicUpperLimit, cfDynamicLowerLimit, 
						fDynamicUpper, fDynamicLower);

					// Check for dynamic fit
					if ((measure.at(uIndex).fStrength >= fDynamicLower) &&
						(measure.at(uIndex).fStrength <= fDynamicUpper))
					{
						muDynamicMatch++;
					}
				} else
				{
					// Check for type match
					if (reference.at(uEvent).uType == measure.at(uIndex).uType)
					{
						// Update reference and measurement
						reference.at(uEvent).bMatch     = true;
						reference.at(uEvent).uReference = uIndex;
						measure.at(uIndex).bMatch       = true;
						measure.at(uIndex).uReference   = uEvent;

						// Update type counter
						muTypeMatch++;

						// TODO: Update dynamic range count
					}
				}
			}
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
        Midicsv ref(mReference, File::eModeRead);
        if (!ref.valid())
        {
            cerr << "error: unable to open reference" << endl;

            return false;
        }

        // Open file
        Midicsv resynthesis(mResynthesis, File::eModeWrite);

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
    fLower  = ((fValue-fTolerance) < fLowerLimit) ? fLowerLimit : fValue-fTolerance;
	fUpper = ((fValue + fTolerance) > fUpperLimit) ? fUpperLimit : fValue + fTolerance;
}

void Paa::confusionMatrix(ostream &out, vector<trEvent> &reference, vector<trEvent> &measure, vector<trMap> &map)
{
	char							buffer[80];
	typedef vector<uint32_t>		typeContainer;
	typedef typeContainer::iterator typeIterator;
	typeContainer					type;

	// Build type vector
	for (uint32_t uIndex = 0; uIndex < map.size(); uIndex++)
	{
		typeIterator i = find(type.begin(), type.end(), map.at(uIndex).uOut);
		if (i == type.end())
		{
			type.push_back(map.at(uIndex).uOut);
		}
	}

	// Display header
	cout << "confusion matrix" << endl;
	cout << "      ";
	for (uint32_t uIndex = 0; uIndex < type.size(); uIndex++)
	{
		sprintf(buffer, "%-6d", type.at(uIndex));
		cout << buffer;
	}

	// Iterate over rows
	for (uint32_t uRow = 0; uRow < type.size(); uRow++)
	{
		// Display row type
		sprintf(buffer, "%-6d", type.at(uRow));
		cout << endl << buffer;

		// Iterate over columns
		for (uint32_t uColumn = 0; uColumn < type.size(); uColumn++)
		{
			// Count the number of events per type
			uint32_t uCount = 0;
			for (uint32_t uEvent = 0; uEvent < reference.size(); uEvent++)
			{
				if (reference.at(uEvent).bMatch)
				{
					uint32_t uReference = reference.at(uEvent).uReference;
					if ((reference.at(uEvent).uType == type.at(uRow)) &&
						(measure.at(uReference).uType == type.at(uColumn)))
					{
						uCount++;
					}
				}
			}

			// Display row entry
			sprintf(buffer, "%-6d", uCount);
			cout << buffer;
		}
	}
	cout << endl;
}