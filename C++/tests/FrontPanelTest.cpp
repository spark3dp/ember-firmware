//  File:   FrontPanelTest.cpp
//  Used for manual testing of the front panel display 
//  May be commented out when running automated tests
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <iostream>
#include <stdlib.h>

#include <FrontPanel.h>
#include <Hardware.h>
#include <Settings.h>
#include "I2C_Device.h"


void test1() {
    std::cout << "FrontPanelTest test 1" << std::endl;
       
    I2C_Device i2cDevice(FP_SLAVE_ADDRESS, 
                         PrinterSettings::Instance().GetInt(HARDWARE_REV) == 0 ?
                                                        I2C2_PORT : I2C1_PORT);
    FrontPanel fp(i2cDevice);
    
    PrinterStatus ps;
    ps._numLayers = 100;
    ps._currentLayer  = 30;;
    ps._estimatedSecondsRemaining = 4380;
    ps._errorCode = NoImageForLayer;
    ps._change = NoChange;
    ps._UISubState = NoUISubState;
    
    PrintEngineState pes =  PrinterOnState;  
    int iter = 1;
    
    srand (time(NULL)); // seed the random number generator
#if 0
    // do forever
    for(;;)
    {
        ps._state = pes;  
        ps._errno = iter;
        
        ((ICallback*)&fp)->Callback(PrinterStatusUpdate, EventData(ps));
        
        // read the front panel buttons while screen is being drawn
        // wait 10 to 500 ms first
        int delayMs = rand() % 490 + 10;
        usleep(delayMs * 1000);
   
        unsigned char btns = i2cDevice.Read(BTN_STATUS);
        if (btns == 0xFF)
        {
            std::cout << "Error reading buttons when state =  " 
                      << PrinterStatus::GetStateName(pes) 
                      << " delay = " << delayMs << std::endl;
        }
        else
        {
            btns &= 0xF;
            if (btns != 0)
                std::cout << "Buttons pressed: " << (int)btns << std::endl;
        }
        
        
        pes = (PrintEngineState)(1 + (int) pes);
        if (pes >= MaxPrintEngineState)
        {
            pes = PrinterOnState;
            ++iter;
        }
    }
#else

    // show all possible front panel error messages
    ps._state = ErrorState;
    ps._errno = 666;
    for(ErrorCode errorCode = Success; errorCode < MaxErrorCode; errorCode = (ErrorCode)(((int)errorCode) + 1))
    {
        if(ErrorMessage::GetShortMessages(errorCode)[0] != "")
        {
            ps._errorCode = errorCode;
            // show the screen
            ((ICallback*)&fp)->Callback(PrinterStatusUpdate, EventData(ps));

            sleep(2); 
        }
    }
#endif    
}


int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% FrontPanelTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (FrontPanelTest)" << std::endl;
//    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (FrontPanelTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

