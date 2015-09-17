//  File:   main.cpp
//  Performs initialization then starts the print engine and event handler
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
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

#include <iostream>
#include <string>
#include <utils.h>
#include <fcntl.h>
#include <stdexcept>

#include <PrintEngine.h>
#include <EventHandler.h>
#include <TerminalUI.h>
#include <Logger.h>
#include <NetworkInterface.h>
#include <CommandInterpreter.h>
#include <Settings.h>
#include <MessageStrings.h>
#include <Hardware.h>
#include <MotorController.h>
#include <Filenames.h>

#include "StandardIn.h"
#include "CommandPipe.h"
#include "PrinterStatusQueue.h"
#include "Timer.h"
#include "I2C_Resource.h"
#include "GPIO_Interrupt.h"
#include "Signals.h"
#include "UdevMonitor.h"

using namespace std;

// command line argument to suppress use of stdin & stdout
#define NO_STDIO    "--nostdio"

int main(int argc, char** argv) 
{
    try
    {
        // see if we should support keyboard input and TerminalUI output
        bool useStdio = true;
        if (argc > 1) 
        {
            useStdio = strcmp(argv[1], NO_STDIO) != 0;
        }
        
        // report the firmware version, board serial number, and startup message
        LOGGER.LogMessage(LOG_INFO, PRINTER_STARTUP_MSG);
        string fwVersion = string(FW_VERSION_MSG) + GetFirmwareVersion();
        LOGGER.LogMessage(LOG_INFO, fwVersion.c_str());
        string serNum = string(BOARD_SER_NUM_MSG) + GetBoardSerialNum();
        LOGGER.LogMessage(LOG_INFO, serNum.c_str());
       
        if (useStdio)
        {
            cout << PRINTER_STARTUP_MSG << endl;
            cout << fwVersion << serNum;
        }
           
        // use cape manager to enable non-default I/O
        int fd = open(CAPE_MANAGER_SLOTS_FILE, O_WRONLY); 
        if (fd < 0)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(CantOpenCapeManager), 
                                                    CAPE_MANAGER_SLOTS_FILE);
            return 1;
        }

        
        std::string s[] = {"am33xx_pwm",    // enable PWM outputs to fans
                           "bone_pwm_P8_19",   
                           "bone_pwm_P9_16", 
                           "bone_pwm_P8_13" };

        for(int i = 0; i < sizeof(s)/sizeof(std::string); i++)
            write(fd, s[i].c_str(), s[i].size());
        
        close(fd);
             
        // ensure directories exist
        MakePath(SETTINGS.GetString(PRINT_DATA_DIR));
        MakePath(SETTINGS.GetString(DOWNLOAD_DIR));
        MakePath(SETTINGS.GetString(STAGING_DIR));

        // create the I2C device for the motor controller
        // use 0xFF as slave address for testing without actual boards
        // note, this must be defined before starting the state machine!
        Motor motor(MOTOR_SLAVE_ADDRESS);
       
        // create the front panel
        FrontPanel fp(FP_SLAVE_ADDRESS, SETTINGS.GetInt(HARDWARE_REV) == 0 ?
            I2C2_PORT : I2C1_PORT); 
 
        EventHandler eh;

        StandardIn standardIn;
        CommandPipe commandPipe;
        PrinterStatusQueue printerStatusQueue;
        Timer exposureTimer;
        Timer temperatureTimer;
        Timer delayTimer;
        GPIO_Interrupt doorSensorGPIOInterrupt(DOOR_SENSOR_PIN,
                GPIO_INTERRUPT_EDGE_BOTH);
        GPIO_Interrupt rotationSensorGPIOInterrupt(ROTATION_SENSOR_PIN,
                GPIO_INTERRUPT_EDGE_FALLING);
        Signals signals;
        UdevMonitor usbDriveConnectionMonitor(UDEV_SUBSYSTEM_BLOCK,
                UDEV_DEVTYPE_PARTITION, UDEV_ACTION_ADD);
        UdevMonitor usbDriveDisconnectionMonitor(UDEV_SUBSYSTEM_BLOCK,
                UDEV_DEVTYPE_PARTITION, UDEV_ACTION_REMOVE);
        
        Timer motorTimeoutTimer;
        I2C_Resource motorControllerTimeout(motorTimeoutTimer, motor, 
                MC_STATUS_REG);
        
        GPIO_Interrupt motorControllerGPIOInterrupt(MOTOR_INTERRUPT_PIN,
                GPIO_INTERRUPT_EDGE_RISING);
        I2C_Resource motorControllerInterrupt(motorControllerGPIOInterrupt, 
                motor, MC_STATUS_REG);
        
        GPIO_Interrupt frontPanelGPIOInterrupt(FP_INTERRUPT_PIN,
                GPIO_INTERRUPT_EDGE_RISING);
        I2C_Resource buttonInterrupt(frontPanelGPIOInterrupt, fp, BTN_STATUS);

        eh.AddEvent(Keyboard, &standardIn);
        eh.AddEvent(UICommand, &commandPipe);
        eh.AddEvent(PrinterStatusUpdate, &printerStatusQueue);
        eh.AddEvent(ExposureEnd, &exposureTimer);
        eh.AddEvent(TemperatureTimer, &temperatureTimer);
        eh.AddEvent(DelayEnd, &delayTimer);
        eh.AddEvent(DoorInterrupt, &doorSensorGPIOInterrupt);
        eh.AddEvent(RotationInterrupt, &rotationSensorGPIOInterrupt);
        eh.AddEvent(Signal, &signals);
        eh.AddEvent(USBDriveConnected, &usbDriveConnectionMonitor);
        eh.AddEvent(USBDriveDisconnected, &usbDriveDisconnectionMonitor);
        eh.AddEvent(MotorTimeout, &motorControllerTimeout);
        eh.AddEvent(MotorInterrupt, &motorControllerInterrupt);
        eh.AddEvent(ButtonInterrupt, &buttonInterrupt);

        // create a print engine that communicates with actual hardware
        PrintEngine pe(true, motor, printerStatusQueue, exposureTimer,
                temperatureTimer, delayTimer, motorTimeoutTimer);

        // set the screensaver time, or disable screen saver if demo mode is 
        // being requested via a button press at startup
        fp.SetAwakeTime(pe.DemoModeRequested() ?
                0 : SETTINGS.GetInt(FRONT_PANEL_AWAKE_TIME));
 
        // give it to the settings singleton as an error handler
        SETTINGS.SetErrorHandler(&pe);
    
        // subscribe logger singleton first, so that it will show 
        // its output in the logs ahead of any other subscribers that actually 
        // act on those events
        eh.Subscribe(PrinterStatusUpdate, &LOGGER);
        eh.Subscribe(MotorInterrupt, &LOGGER);
        eh.Subscribe(ButtonInterrupt, &LOGGER);
        eh.Subscribe(DoorInterrupt, &LOGGER);
        if (useStdio)
            eh.Subscribe(Keyboard, &LOGGER);
        eh.Subscribe(UICommand, &LOGGER);
        
        // subscribe the print engine to interrupt events
        eh.Subscribe(MotorInterrupt, &pe);
        eh.Subscribe(ButtonInterrupt, &pe); 
        eh.Subscribe(DoorInterrupt, &pe);
        eh.Subscribe(RotationInterrupt, &pe);
        
        // subscribe the print engine to timer events
        eh.Subscribe(DelayEnd, &pe);
        eh.Subscribe(ExposureEnd, &pe);
        eh.Subscribe(TemperatureTimer, &pe);
        eh.Subscribe(MotorTimeout, &pe);

        // subscribe the print engine to the usb addition/removal events
        eh.Subscribe(USBDriveConnected, &pe);
        eh.Subscribe(USBDriveDisconnected, &pe);
        
        CommandInterpreter peCmdInterpreter(&pe);
        // subscribe the command interpreter to command input events,
        // from UI and possibly the keyboard
        eh.Subscribe(UICommand, &peCmdInterpreter); 
        if (useStdio)
            eh.Subscribe(Keyboard, &peCmdInterpreter);   
        
        // subscribe the front panel to printer status events
        eh.Subscribe(PrinterStatusUpdate, &fp);
      
        // connect the event handler to itself via another command interpreter
        // to allow the event handler to stop when it receives an exit command
        // Also subscribe to Signal event to handle TERM and INT signals
        CommandInterpreter ehCmdInterpreter(&eh);
        eh.Subscribe(UICommand, &ehCmdInterpreter);
        eh.Subscribe(Keyboard, &ehCmdInterpreter);
        eh.Subscribe(Signal, &eh);
        
        // also connect a network interface, subscribed to printer status events
        NetworkInterface networkIF;
        eh.Subscribe(PrinterStatusUpdate, &networkIF);
        
        if (useStdio)
        {
            // also connect a terminal UI, subscribed to printer status events
            TerminalUI terminal;
            eh.Subscribe(PrinterStatusUpdate, &terminal);
        }
        
        // start the print engine's state machine
        pe.Begin();
        // begin handling events
        eh.Begin();
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

