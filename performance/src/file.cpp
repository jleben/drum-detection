//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// File Class Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <cassert>
#include <string>
#include <fstream>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "file.h"

// P U B L I C  M E T H O D S
File::File(string name, teMode eMode) : Object(name)
{
    // Open file
    switch (eMode)
    {
        case eModeRead:
            mFile.open(name, ios::in);
            break;

        case eModeWrite:
            mFile.open(name, ios::out);
            break;

        default:
            assert(false);
    }
}

File::~File()
{
    // Cleanup
    mFile.close();
}

bool File::valid()
{
    return mFile.good();
}

bool File::lineGet(string &line)
{
	getline(mFile, line);

    return (line.length() > 0) ? true : false;
}

void File::linePut(string line)
{
    mFile << line << endl;
}

void File::reset()
{
    mFile.clear();
    mFile.seekg(0, ios::beg);
}

ostream& File::operator<<(const bool bState)
{
    return mFile << bState;
}

ostream& File::operator<<(const char character)
{
    return mFile << character;
}

ostream& File::operator<<(const uint32_t uValue)
{
    return mFile << uValue;
}

ostream& File::operator<<(const float fValue)
{
    return mFile << fValue;
}

ostream& File::operator<<(const char *pString)
{
    return mFile << pString;
}
