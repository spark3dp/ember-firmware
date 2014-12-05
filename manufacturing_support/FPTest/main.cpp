/* 
 * File:   main.cpp
 * 
 * For testing Ember front panel during manufacturing.
 * 
 * Author: Richard Greene
 *
 * Created on December 5, 2014, 9:39 AM
 */

#include <I2C_Device.h>

#include <cstdlib>
#include <stdio.h>
#include <string.h>


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    int port = I2C2_PORT;
#ifdef LANIUS    
    // enable second I2C port 
    system("echo BB-I2C1 > /sys/devices/bone_capemgr.9/slots");
    
    port = I2C1_PORT;
#endif    
 
    I2C_Device frontPanel(UI_SLAVE_ADDRESS, port);
    
    const unsigned char ledSequence[] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 8, CMD_END};
    frontPanel.Write(UI_COMMAND, ledSequence, strlen((const char*)ledSequence));
    
    return 0;
}

