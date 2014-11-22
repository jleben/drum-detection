//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// CSV Class Definition
//
#ifndef _CSV_H
#define _CSV_H

// C L A S S
class Csv : public File
{
public:  

    // Constructor[s]
    Csv(string name, File::teMode eMode);

    // Destructor
    ~Csv();

    // Method[s]
    bool eventRead(float &fTimestamp, uint32_t &uType, float &fStrength);

private:
};

#endif // _CSV_H