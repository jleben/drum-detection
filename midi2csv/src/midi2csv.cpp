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
#include "midi2csv.h"
#include "file.h"
#include "midicsv.h"
#include "csv.h"
#include "midifile.h"
#include "map.h"

// P U B L I C  M E T H O D S
Midi2Csv::Midi2Csv(int argc, char *argv[]) : App(argc, argv)
{
    // Set defaults
    mbVerbose = false;
	muChannel = 9;
}

Midi2Csv::~Midi2Csv()
{
}

bool Midi2Csv::run(ostream &out)
{
    uint32_t         uIndex;
    vector<trEvent>  event;
    type_map         map;

    // Check for help request
    if (option(cOptionHelp))
    {
        usage(cerr);

        return true;
    }

    // Parse option[s]
    mbVerbose = option(cOptionVerbose);
	mbMap     = option(cOptionMap, mMap);
	option(cOptionChannel, muChannel);

    // Parse argument[s]
	if (!argument(2, mMidiFile) || !argument(1, mCsvFile))
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
        acquireEvents(mMidiFile, event, map, true);
    }
    catch (std::exception & e)
    {
        cerr << "Error: Can not read MIDI file: " << mMidiFile << endl;
        cerr << "Reason: " << e.what() << endl;
        return false;
    }

	// Check verbosity
	if (mbVerbose)
	{
		cout << "events: " << event.size() << endl;
	}

    // Check for events
	if (event.size() > 0)
	{
		// Open CSV file
		Csv csv(mCsvFile, File::eModeWrite);

		// Write CSV entries
		for (uIndex = 0; uIndex < event.size(); uIndex++)
		{
			csv << event.at(uIndex).fTimestamp << ",";
			csv << event.at(uIndex).uType << ",";
			csv << event.at(uIndex).fStrength << "\n";
		}
	}

    return true;
}

void Midi2Csv::usage(ostream &out)
{
    char buffer[80];

    sprintf(buffer, "usage: %s -[%c%c%c%c] <midifile> <csvfile>\n",
        name().c_str(), cOptionVerbose, cOptionMap, cOptionChannel, cOptionHelp);
    out << buffer;
    sprintf(buffer, "where;\n");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionVerbose, "", "verbose");
    out << buffer;
    sprintf(buffer, " %8c %-16s %-32s\n", cOptionMap, "<filename>", 
        "comma seperated type map input file");
    out << buffer;
	sprintf(buffer, " %8c %-16s %-32s\n", cOptionChannel, "<channel>",
		"MIDI channel");
	out << buffer;
	sprintf(buffer, " %8c %-16s %-32s\n", cOptionHelp, "",
        "program help");
    out << buffer;
	sprintf(buffer, " %8c %-16s %-32s\n", ' ', "midifile",
		"MIDI format input file");
	out << buffer;
	sprintf(buffer, " %8c %-16s %-32s\n", ' ', "csvfile",
		"CSV format output file");
	out << buffer;
}

void Midi2Csv::dump(ostream &out)
{
    out << "map file: " << ((mbMap) ? mMap: "none") << endl;
    out << "MIDI file: " << mMidiFile << endl;
    out << "CSV file: " << mCsvFile << endl;
}

// P R I V A T E  M E T H O D S
bool Midi2Csv::acquireMap(string name, vector<trMap> &map)
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

void Midi2Csv::acquireEvents(string name, vector<trEvent> &onset,
                        const type_map &map, bool do_map)
{
    File    *pFile;
    trEvent  rEvent;

    // Try to open file using supported format[s]
    pFile = new Midifile(name, File::eModeBinaryRead);
	if (pFile->valid())
	{
		((Midifile*)pFile)->channel(muChannel);
	}

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

	// Sort events
	sort(onset.begin(), onset.end(), sortEvent);

    // Cleanup
    delete pFile;
}

