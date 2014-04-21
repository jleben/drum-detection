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
#include <cmath>
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
    type_map         map;

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
    if (!acquireMap(mMap, map.mappings))
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
        acquireEvents(mReference, reference, map, true);
    }
    catch (std::exception & e)
    {
        cerr << "Error: Can not read reference event file: " << mReference << endl;
        cerr << "Resone: " << e.what() << endl;
        return false;
    }

    try {
        acquireEvents(mMeasure, measure, map, false);
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

    cout << setprecision(3) << fixed;

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

        //float strength = map.source_strength(detectedEvent.fStrength, referenceEvent.uType);
        float strength = detectedEvent.fStrength;

        // Check strength match
        bool strength_match = false;
        if ( (strength >= fDynamicLower) &&
             (strength <= fDynamicUpper) )
        {
            strength_match = true;
            muDynamicMatch++;
        }

        if (mbVerbose)
        {
            cout << "Comparing strength: " << strength
                 << " to " << referenceEvent.fStrength
                 << " [" << referenceEvent.original_type << "]"
                 << " (" << fDynamicLower << " - " << fDynamicUpper << ")"
                 << (strength_match ? " : OK" : " : WRONG")
                 << endl;
        }
    }

	// Check verbosity
	if (mbVerbose)
	{
		cout << "onset match: " << muOnsetMatch << endl;
		cout << "type match: " << muTypeMatch << endl;
		cout << "dynamic match: " << muDynamicMatch << endl;
	}
	

    // Output statistics

    out << setprecision(1) << fixed;

    statistics stats;
    confusion_matrix matrix(map);

    stats.dynamics_accuracy = muOnsetMatch ? (float) muDynamicMatch / muOnsetMatch : 0;
    computeStatistics(out, reference, measure, stats, matrix);

    out << "Onset:"
         << " A = " << stats.onset_accuracy * 100 << "%"
         << " | P = " << stats.onset_precision * 100 << "%"
         << " | R = " << stats.onset_recall * 100 << "%" << endl;

    out << "Type = " << stats.type_accuracy * 100 << "%" << endl;
    out << "Strength = " << stats.dynamics_accuracy * 100 << "%" << endl;

    out << "Confusion matrix:" << endl;
    matrix.print(out);

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

            // Inverse map type and strength
            const trMap *mapping = map.find_source(rEvent.uType);
            if (mapping)
            {
                rEvent.uType = mapping->uIn;
                rEvent.fStrength = mapping->velocity(rEvent.fStrength);
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
    while (mapFile.read(rMap.uIn, rMap.uOut, rMap.strength_scale))
    {
        map.push_back(rMap);
    }

    return true;
}

void Paa::acquireEvents(string name, vector<trEvent> &onset,
                        const type_map &map, bool do_map)
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
    while (pFile->eventRead(rEvent.fTimestamp, rEvent.original_type, rEvent.fStrength))
    {
        // Check for non-zero signal strength
        if (rEvent.fStrength > 0.0f)
        {
            // Initialize attributes
            rEvent.bMatch     = false;
            rEvent.uReference = 0;
            rEvent.uType = rEvent.original_type;

            // Map type and strength
            if (do_map)
            {
                const trMap *mapping = map.find(rEvent.original_type);
                if (mapping)
                {
                    rEvent.uType = mapping->uOut;
                    rEvent.fStrength = mapping->rms(rEvent.fStrength);
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
  // Disregard global limits.
  // Detected onsets may be (slightly) negative or beyond end of audio,
  // but still admissible.
  fLower = fValue - fTolerance;
  fUpper = fValue + fTolerance;
  //fLower  = ((fValue-fTolerance) < fLowerLimit) ? fLowerLimit : fValue-fTolerance;
  //fUpper = ((fValue + fTolerance) > fUpperLimit) ? fUpperLimit : fValue + fTolerance;
}

Paa::confusion_matrix::confusion_matrix( const type_map & map )
{
    // Build type vector

    for (uint32_t uIndex = 0; uIndex < map.mappings.size(); uIndex++)
    {
        vector<int>::const_iterator i = find(types.begin(), types.end(), map.mappings.at(uIndex).uOut);
        if (i == types.end())
        {
            types.push_back(map.mappings.at(uIndex).uOut);
        }
    }

    // Allocate data space

    for (int i = 0; i < row_count(); ++i)
    {
        data.emplace_back( column_count(), 0 );
    }
}

void Paa::confusion_matrix::print( std::ostream & out )
{
    int total_cols = column_count();
    int total_rows = row_count();

    int unmapped_col = total_cols - 2;
    int missed_col = total_cols - 1;
    int unmapped_row = total_rows - 2;
    int ghost_row = total_rows - 1;

    out << setw(8) << "ref/det";
    for (int i = 0; i < types.size(); ++i)
        out << setw(8) << types[i];
    out << setw(8) << "other";
    out << setw(8) << "missed";
    out << endl;

    for (int r = 0; r < types.size(); ++r)
    {
        out << setw(8) << types[r];
        for (int c = 0; c < total_cols; ++c)
        {
            out << setw(8) << data[r][c];
        }
        out << endl;
    }

    out << setw(8) << "other";
    for (int c = 0; c < total_cols; ++c)
    {
        out << setw(8) << data[unmapped_row][c];
    }
    out << endl;

    out << setw(8) << "ghost";
    for (int c = 0; c < (total_cols - 1); ++c)
    {
        out << setw(8) << data[ghost_row][c];
    }
    out << endl;
}

void Paa::computeStatistics(ostream &out, vector<trEvent> &reference,
                           vector<trEvent> &measure,
                           statistics & stats,
                           confusion_matrix & matrix)
{
    int total_cols = matrix.column_count();
    int total_rows = matrix.row_count();

    int unmapped_col = total_cols - 2;
    int missed_col = total_cols - 1;
    int unmapped_row = total_rows - 2;
    int ghost_row = total_rows - 1;

    unsigned int detected_count = 0;
    unsigned int misdetected_count = 0;
    unsigned int missed_count = 0;
    unsigned int ghost_count = 0;

    for (int refIndex = 0; refIndex < reference.size(); ++refIndex)
    {
        int row = unmapped_row;
        int col = missed_col;

        trEvent & refEvent = reference[refIndex];
        row = matrix.typeIndex(refEvent.uType);
        if (row == -1)
            row = unmapped_row;

        if (refEvent.bMatch)
        {
            trEvent & detectedEvent = measure[refEvent.uReference];
            col = matrix.typeIndex(detectedEvent.uType);
            if (col == -1)
                col = unmapped_col;

            if (detectedEvent.uType == refEvent.uType)
              detected_count++;
            else
              misdetected_count++;
        }
        else
        {
          missed_count++;
        }

        matrix.data[row][col]++;
    }

    for (int detIndex = 0; detIndex < measure.size(); ++detIndex)
    {
        trEvent & detectedEvent = measure[detIndex];
        if (!detectedEvent.bMatch)
        {
            int col = matrix.typeIndex(detectedEvent.uType);
            if (col == -1)
                col = unmapped_col;

            matrix.data[ghost_row][col]++;

            ghost_count++;
        }
    }

    int total_count = detected_count + misdetected_count + missed_count + ghost_count;

    stats.onset_accuracy =
        (float) (detected_count + misdetected_count) / total_count;
    stats.onset_precision =
        (float) (detected_count + misdetected_count) /
        (detected_count + misdetected_count + ghost_count);
    stats.onset_recall =
        (float) (detected_count  + misdetected_count) /
        (detected_count  + misdetected_count + missed_count);
    stats.type_accuracy =
        (float) detected_count / (detected_count + misdetected_count);
}
