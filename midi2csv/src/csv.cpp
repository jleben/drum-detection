//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// CSV Class Implementation
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

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "file.h"
#include "csv.h"

// P U B L I C  M E T H O D S
Csv::Csv(string name, File::teMode eMode) : File(name, eMode)
{
}

Csv::~Csv()
{
}

bool Csv::eventRead(float &fTimestamp, uint32_t &uType, float &fStrength)
{
    uint16_t uColumn;
    string   line;
    string   field;

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

                    stringstream(field) >> fTimestamp;
                    break;

                case 1:

                    stringstream(field) >> uType;
                    break;

                case 2:

                    stringstream(field) >> fStrength;                    
                    return true;


                default:

                    // Ignore field
                    break;
            }

            // Update column
            uColumn++;
        }
    }

    return false;
}

