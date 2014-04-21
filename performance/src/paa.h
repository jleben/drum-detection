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

#include <cmath>

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

    struct statistics
    {
      float onset_accuracy;
      float onset_precision;
      float onset_recall;
      float type_accuracy;
      float dynamics_accuracy;
    };

    struct confusion_matrix
    {
        vector<int> types;
        vector< vector<int> > data;

        confusion_matrix( const type_map & );

        int row_count() const { return types.size() + 2; }
        int column_count() const { return types.size() + 2; }

        int typeIndex( int type )
        {
            for (int i = 0; i < types.size(); ++i)
            {
                if (type == types[i])
                    return i;
            }
            return -1;
        }

        void print( std::ostream & out );
    };

    // Method[s]
    bool acquireMap(string name, vector<trMap> &map);
    void acquireEvents(string name, vector<trEvent> &onset,
                       const type_map &, bool do_map);
    void range(float fValue, float fTolerance, float fUpperLimit,
               float fLowerLimit, float &fUpper, float &fLower);
    void computeStatistics(ostream &out, vector<trEvent> &reference,
                           vector<trEvent> &measure,
                           statistics & stats,
                           confusion_matrix & matrix);



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
