//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// MIDI to CSV Translator Application Class Definition
//
#ifndef _MIDI2CSV_H
#define _MIDI2CSV_H

#include <cmath>

// C L A S S
class Midi2Csv: public App
{
public:

    // Constant[s]
    static char  const cOptionHelp    = 'h';
    static char  const cOptionMap     = 'm';
    static char  const cOptionVerbose = 'v';
	static char  const cOptionChannel = 'c';

    // Constructor[s]
    Midi2Csv(int argc, char *argv[]);

    // Destructor
    ~Midi2Csv();

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
        uint32_t original_type;
        float    fStrength;
    } trEvent;

    typedef struct
    {
        uint32_t uIn;
        uint32_t uOut;
        float strength_scale;

        float rms (float velocity) const
        {
            return velocity * velocity * strength_scale;
        }

        float velocity (float rms) const
        {
            return std::sqrt( rms / strength_scale );
        }
    } trMap;

    struct type_map
    {
        vector<trMap> mappings;

        int size() const { return (int) mappings.size(); }

        const trMap * find(size_t source_type) const
        {
            for (unsigned int i = 0; i < mappings.size(); ++i)
            {
                if (mappings[i].uIn == source_type)
                    return &mappings[i];
            }
            return 0;
        }

        const trMap * find_source(size_t target_type) const
        {
            for (unsigned int i = 0; i < mappings.size(); ++i)
            {
                if (mappings[i].uOut == target_type)
                    return &mappings[i];
            }
            return 0;
        }

        uint32_t type( uint32_t source_type ) const
        {
            const trMap *m = find(source_type);
            if (m)
                return m->uOut;
            else
                return source_type;
        }

        uint32_t source_type( uint32_t target_type ) const
        {
            const trMap *m = find_source(target_type);
            if (m)
                return m->uIn;
            else
                return target_type;
        }
    };

    // Method[s]
    bool acquireMap(string name, vector<trMap> &map);
    void acquireEvents(string name, vector<trEvent> &onset,
                       const type_map &, bool do_map);
	static bool sortEvent(const trEvent& event1, const trEvent& event2)
	{
		return (event1.fTimestamp < event2.fTimestamp);
	}


    // Data
    bool     mbVerbose;
    bool     mbMap;
	uint32_t muChannel;
    string   mMidiFile;
	string   mCsvFile;
    string   mMap;
};

#endif // _MIDI2CSV_H
