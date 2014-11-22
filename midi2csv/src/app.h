//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Application Class Definition
//
#ifndef _APP_H
#define _APP_H

// C L A S S
class App : public Object
{
public:

    // Constant[s]
    static const char cOptionPrefix = '-';

    // Constructor[s]
    App(int argc, char *argv[]);

    // Destructor
    ~App();

    // Methods[s]
    bool         option(const char flag);
    bool         option(const char flag, string &arg);
    bool         option(const char flag, uint32_t &uValue);
    bool         option(const char flag, float &fValue);
    bool         argument(uint16_t uIndex, string &arg);

    // Virtual Method[s]
    virtual void dump(ostream &out);

    // Pure Virtual Method[s]
    virtual bool run(ostream &out)   = 0;
    virtual void usage(ostream &out) = 0;

private:

    // Data
    int    mArgc;
    char **mppArgv;
};

#endif // _APP_H