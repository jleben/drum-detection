//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Map Class Definition
//
#ifndef _MAP_H
#define _MAP_H

// C L A S S
class Map : public File
{
public:  

    // Constructor[s]
    Map(string name, File::teMode eMode);

    // Destructor
    ~Map();

    // Method[s]
    bool read(uint32_t &uIn, uint32_t &uOut);

    bool eventRead(float &fTimestamp, uint32_t &uType, float &fStrength);

private:
};

#endif // _MAP_H