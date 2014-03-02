//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// object Class Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <string>
#include <iostream>

// P R O J E C T  I N C L U D E S
#include "object.h"

// P U B L I C  M E T H O D S
Object::Object(string name)
{
    // Initialize data
    mName = name;
}

Object::~Object()
{
}

string &Object::name()
{
    return mName;
}

void Object::dump(ostream &out)
{
    out << mName;
}