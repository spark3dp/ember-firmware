/* 
 * File:   main.cpp
 * Author: Richard Greene
 *
 * Created on March 26, 2014, 4:01 PM
 */

#include <iostream>
#include <string>
#include <utils.h>
#include <signal.h>

#include <PrintEngine.h>
#include <EventHandler.h>
#include <TerminalUI.h>
#include <Logger.h>
#include <NetworkInterface.h>
#include <CommandInterpreter.h>
#include <Settings.h>
#include <MessageStrings.h>
#include <Hardware.h>

using namespace std;

int main(int argc, char** argv) 
{
    // Set up signal handling
    struct sigaction exitSA, hangUpSA;

    // Call exit handler on SIGINT or SIGTERM
    sigemptyset(&exitSA.sa_mask);
    exitSA.sa_handler = &ExitHandler;
    exitSA.sa_flags = 0;
    sigaddset(&exitSA.sa_mask, SIGINT);
    sigaddset(&exitSA.sa_mask, SIGTERM);
    sigaction(SIGINT, &exitSA, NULL);
    sigaction(SIGTERM, &exitSA, NULL);
    
    // Ignore SIGHUP as it causes terminiation by default
    sigemptyset(&hangUpSA.sa_mask);
    hangUpSA.sa_handler = SIG_IGN;
    hangUpSA.sa_flags = 0;
    sigaction(SIGHUP, &hangUpSA, NULL);
    
    // enable fans
    system("echo am33xx_pwm > /sys/devices/bone_capemgr.9/slots");
    system("echo bone_pwm_P8_19 > /sys/devices/bone_capemgr.9/slots");
    system("echo bone_pwm_P9_16 > /sys/devices/bone_capemgr.9/slots");
    system("echo bone_pwm_P8_13 > /sys/devices/bone_capemgr.9/slots");

    cout << PRINTER_STARTUP_MSG << endl;
    // report the firmware version and board serial no.
    string fwVersion = string(FW_VERSION_MSG) + GetFirmwareVersion();
    LOGGER.LogMessage(LOG_INFO, fwVersion.c_str());
    string serNum = string(BOARD_SER_NUM_MSG) + GetBoardSerialNum();
    LOGGER.LogMessage(LOG_INFO, serNum.c_str());
    cout << fwVersion << serNum;
    
    // ensure directories exist
    // accessing SETTINGS for the first time regenerates the settings file if it doesn't exist
    MakePath(SETTINGS.GetString(PRINT_DATA_DIR));
    MakePath(SETTINGS.GetString(DOWNLOAD_DIR));
    MakePath(SETTINGS.GetString(STAGING_DIR));
     
    // create an event handler
    EventHandler eh;
    
    // create a print engine that communicates with actual hardware
    PrintEngine pe(true);

    // give it to the settings singleton as an error handler
    SETTINGS.SetErrorHandler(&pe);
    
    // create the front panel
    FrontPanel fp(UI_SLAVE_ADDRESS); 
 
    // set the I2C devices
    eh.SetI2CDevice(MotorInterrupt, pe.GetMotorBoard(), MOTOR_STATUS);
    eh.SetI2CDevice(ButtonInterrupt, &fp, BTN_STATUS);
    
    // subscribe logger singleton first, so that it will show 
    // its output in the logs ahead of any other subscribers that actually 
    // act on those events
    eh.Subscribe(PrinterStatusUpdate, &LOGGER);
    eh.Subscribe(MotorInterrupt, &LOGGER);
    eh.Subscribe(ButtonInterrupt, &LOGGER);
    eh.Subscribe(DoorInterrupt, &LOGGER);
    eh.Subscribe(Keyboard, &LOGGER);
    eh.Subscribe(UICommand, &LOGGER);
    
    // subscribe the print engine to interrupt events
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe); 
    eh.Subscribe(DoorInterrupt, &pe);
    
    // subscribe the print engine to timer events
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    CommandInterpreter peCmdInterpreter(&pe);
    // subscribe the command interpreter to command input events,
    // from UI and keyboard
    eh.Subscribe(UICommand, &peCmdInterpreter);    
    eh.Subscribe(Keyboard, &peCmdInterpreter);   
    
    // subscribe the front panel to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &fp);
    
    // also connect a network interface, subscribed to UI commands and 
    // printer status events
    NetworkInterface networkIF;
    eh.Subscribe(PrinterStatusUpdate, &networkIF);
    CommandInterpreter niCmdInterpreter(&networkIF);
    eh.Subscribe(UICommand, &niCmdInterpreter);
    
    // also connect a terminal UI, subscribed to printer status events
    TerminalUI terminal;
    eh.Subscribe(PrinterStatusUpdate, &terminal);
    
    // start the print engine's state machine
    pe.Begin();
    // begin handling events
    eh.Begin();
    
    return 0;
}

