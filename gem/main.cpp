/* 
 * File:   main.cpp
 * Author: Richard Greene
 *
 * Created on March 26, 2014, 4:01 PM
 */

#include <PrintEngine.h>
#include <EventHandler.h>
#include <TerminalUI.h>
#include <Logger.h>
#include <NetworkInterface.h>

using namespace std;

int main(int argc, char** argv) 
{
    // create an event handler
    EventHandler eh;
    
    // create a print engine that communicates with actual hardware
    PrintEngine pe(true);
    
    // create the front panel
    FrontPanel fp(UI_SLAVE_ADDRESS); 
 
    // set the I2C devices
    eh.SetI2CDevice(MotorInterrupt, pe.GetMotorBoard(), MOTOR_STATUS);
    eh.SetI2CDevice(ButtonInterrupt, &fp, UI_STATUS);
    
    // create a logger and do all its subscriptions first, so that it will show 
    // its output in the logs ahead of any other subscribers that actually 
    // act on those events
    Logger logger;
    eh.Subscribe(PrinterStatusUpdate, &logger);
    eh.Subscribe(MotorInterrupt, &logger);
    eh.Subscribe(ButtonInterrupt, &logger);
    eh.Subscribe(DoorInterrupt, &logger);
    eh.Subscribe(Keyboard, &logger);
    
    // subscribe the print engine to interrupt events
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    // subscribe the print engine to timer events
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetPulseTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    // subscribe the print engine to command input & keyboard events
    eh.Subscribe(UICommand, &pe);    
    eh.Subscribe(Keyboard, &pe);    
    
    // subscribe the front panel to status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &fp);
    
    // also connect a network interface, subscribed to printer status events
    NetworkInterface networkIF;
    eh.Subscribe(PrinterStatusUpdate, &networkIF);
    
    // also connect a terminal UI, subscribed to printer status events
    TerminalUI terminal;
    eh.Subscribe(PrinterStatusUpdate, &terminal);
    
    // start the print engine's state machine
    pe.Begin();
    // begin handling events
    eh.Begin();
    
    return 0;
}

