//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Object Class Definition
//
#ifndef _OBJECT_H
#define _OBJECT_H

// D E F I N E S
#ifdef WIN32
#define sprintf         sprintf_s
#endif // WIN32

// C L A S S
class Object
{
public:

    // Constructor[s]
    Object(string name);

    // Destructor
    ~Object();

    // Method[s]
    string &name();

    // Virtual Method[s]
    virtual void dump(ostream &out);

private:

    // Data
    string mName;
};

#endif // _OBJECT_H