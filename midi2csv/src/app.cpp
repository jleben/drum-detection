//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Application Class Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <cassert>
#include <iostream>
#include <string.h>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "app.h"

// P U B L I C  M E T H O D S
App::App(int argc, char *argv[]) : Object(argv[0])
{
    // Assign argument references
    mArgc   = argc;
    mppArgv = argv;
}

App::~App()
{
}

bool App::option(const char flag, string &arg)
{
    uint16_t uIndex;

    // Search for specified option
    for (uIndex = 0; uIndex < mArgc; uIndex++)
    {
        if (strlen(mppArgv[uIndex]) > 1)
        {
            if ((cOptionPrefix == mppArgv[uIndex][0]) && 
                (flag == mppArgv[uIndex][1]))
            {
                arg = (++uIndex < mArgc) ?mppArgv[uIndex] : "";

                return true;
            }
        }
    }

    return false;
}

bool App::option(const char flag, uint32_t &uValue)
{
    string arg;

    // Get option with argument
    if (option(flag, arg))
    {
        // Convert argument to value
        uValue = atol(arg.c_str());

        return true;
    }

    return false;
}

bool App::option(const char flag, float &fValue)
{
    string arg;

    // Get option with argument
    if (option(flag, arg))
    {
        // Convert argument to value
        fValue = (float)atof(arg.c_str());

        return true;
    }

    return false;
}

bool App::option(const char flag)
{
    string dummy;

    return option(flag, dummy);
}

bool App::argument(uint16_t uIndex, string &arg)
{
    // Check index
    if (uIndex > mArgc)
    {
        return false;
    }

    // Assign argument
    arg = mppArgv[mArgc - uIndex];

    return true;
}

void App::dump(ostream &out)
{
    Object::dump(out);
}
