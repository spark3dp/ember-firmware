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

using namespace std;

int main(int argc, char** argv) 
{
    // create an event handler
    EventHandler eh;
    
    // connect it to a print engine that uses real hardware
    PrintEngine pe(true);
      
    // set the I2C devices
    eh.SetI2CDevice(MotorInterrupt, pe.GetMotorBoard(), MOTOR_STATUS);
    // TODO: FrontPanel should own the UI board, not PrintEngine
    eh.SetI2CDevice(ButtonInterrupt, pe.GetUIBoard(), UI_STATUS);
    
    // subscribe to interrupt events
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    // subscribe to timer events
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetPulseTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    eh.Subscribe(Keyboard, &pe);    
    
    // also connect a terminal UI
    TerminalUI terminal;
    // subscribe to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &terminal);
    
    // also connect a logger
    Logger logger;
    eh.Subscribe(PrinterStatusUpdate, &logger);
    eh.Subscribe(MotorInterrupt, &logger);
    eh.Subscribe(ButtonInterrupt, &logger);
    eh.Subscribe(DoorInterrupt, &logger);
    eh.Subscribe(Keyboard, &logger);
    
    // start the print engine's state machine
    pe.Begin();
    // begin handling events
    eh.Begin();
    
    return 0;
}

