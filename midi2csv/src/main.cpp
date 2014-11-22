//
// CSC 575 - Music Information Retrieval
//
// Copyright (c) 2014, Robert Van Rooyen. All Rights Reserved.
//
// The contents of this software are proprietary and confidential. No part of 
// this program may be photocopied, reproduced, or translated into another
// programming language without prior written consent of the author.
//
// Application Entry Point Implementation
//

// N A M E S P A C E S
using namespace std;

// S Y S T E M  I N C L U D E S
#include <cstdint>
#include <iostream>
#include <vector>

// P R O J E C T  I N C L U D E S
#include "object.h"
#include "app.h"
#include "midi2csv.h"

// P U B L I C  F U N C T I O N S
int main(int argc, char *argv[])
{
    // Instantiate application
    Midi2Csv app(argc, argv);

    // Run application
    if (!app.run(cout))
    {
        // Display error
        cout << "failure!" << endl;

        return -1;
    }

	return 0;
}
