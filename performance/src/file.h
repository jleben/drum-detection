//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// File Class Definition
//
#ifndef _FILE_H
#define _FILE_H

// C L A S S
class File : Object
{
public:

    // Enum[s]
    typedef enum
    {
        eModeRead,
        eModeWrite
    } teMode;

    // Constructor[s]
    File(string name, teMode eMode);

    // Destructor
    ~File();

    // Method[s]
    bool lineGet(string &line);
    void linePut(string line);
    void reset();

    // Virtual Method[s]
    virtual bool valid();
    virtual bool eventRead(float &fTimestamp, uint32_t &uType, 
                    float &fStrength) = 0;

    // Operator[s]
    ostream& operator<<(const bool bState);
    ostream& operator<<(const char character);
    ostream& operator<<(const uint32_t uValue);
    ostream& operator<<(const float fValue);
    ostream& operator<<(const char *pString);

private:

    // Data
    fstream mFile;
};

#endif // _FILE_H