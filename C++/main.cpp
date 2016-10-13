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
#include <fstream> 
#include <string>
#include <utils.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdexcept>
#include <Magick++.h>

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
#include "GPIO.h"
#include "Signals.h"
#include "UdevMonitor.h"
#include "I2C_Device.h"
#include "Projector.h"
#include "HardwareFactory.h"

using namespace std;

// command line argument to suppress use of stdin & stdout
constexpr const char* NO_STDIO = "--nostdio";

// for setting DMA priority to avoid video flicker
constexpr unsigned long MAP_SIZE = 4096UL;
constexpr unsigned long MAP_MASK = (MAP_SIZE - 1);
constexpr off_t REG_PR_OLD_COUNT = 0x4c000054;
constexpr unsigned long PR_OLD_COUNT_VALUE = 0x00FFFFF10;

int main(int argc, char** argv) 
{
    try
    {
        // sets up signal handling
        Signals signals;
        
        Magick::InitializeMagick("");
        
        // see if we should support keyboard input and TerminalUI output
        bool useStdio = true;
        if (argc > 1) 
        {
            useStdio = strcmp(argv[1], NO_STDIO) != 0;
        }
        
        // report the firmware version, board serial number, and startup message
        Logger::LogMessage(LOG_INFO, PRINTER_STARTUP_MSG);
        string version = GetFirmwareVersion();
        string fwVersion = string(FW_VERSION_MSG) + version;
        Logger::LogMessage(LOG_INFO, fwVersion.c_str());
        string serNum = string(BOARD_SER_NUM_MSG) + GetBoardSerialNum();
        Logger::LogMessage(LOG_INFO, serNum.c_str());
       
        if (useStdio)
        {
            cout << PRINTER_STARTUP_MSG << endl;
            cout << fwVersion << std::endl << serNum << std::endl;
        }
          
        // turn on fans
        GPIO fan1GPIO(FAN_1_PIN);
        GPIO fan2GPIO(FAN_2_PIN);
        GPIO fan3GPIO(FAN_3_PIN);

        fan1GPIO.SetDirectionOut();
        fan2GPIO.SetDirectionOut();
        fan3GPIO.SetDirectionOut();

        fan1GPIO.SetOutputHigh();
        fan2GPIO.SetOutputHigh();
        fan3GPIO.SetOutputHigh();
        
        // prevent video flickering by tweaking the value of REG_PR_OLD_COUNT
        // see https://groups.google.com/forum/#!msg/beagleboard/GjxRGeLdmRw/dx-bOXBPBgAJ
        // and http://www.lartmaker.nl/lartware/port/devmem2.c 
        int fd = open(MEMORY_DEVICE, O_RDWR | O_SYNC);
        if(fd < 0)
        {
            Logger::LogError(LOG_ERR, errno, CantOpenMemoryDevice);
            return 1;
        }

        // map one page 
        void* mapBase = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                             fd, REG_PR_OLD_COUNT & ~MAP_MASK);
        if(mapBase == MAP_FAILED)
        {
            Logger::LogError(LOG_ERR, errno,CantMapPriorityRegister);
            return 1;
        }

        void* addr = ((char*)mapBase) + (REG_PR_OLD_COUNT & MAP_MASK);

        *((unsigned long *) addr) = PR_OLD_COUNT_VALUE;

        if(munmap(mapBase, MAP_SIZE) < 0)
        {
            Logger::LogError(LOG_ERR, errno, CantUnMapPriorityRegister);
            return 1;
        }

        close(fd);        
        
        Settings& settings = PrinterSettings::Instance();
        
        // If we're upgrading to higher version or downgrading to a lower one,
        // update selected printer settings with current default values.
        if (version != settings.GetString(FW_VERSION))
        {
            // update FirmwareVersion setting so that this version of the 
            // firmware won't run this code again
            settings.Set(FW_VERSION, version);
            // restore these settings to their new defaults
            cout << UPDATING_DEFAULTS_MSG << endl; 
            vector<const char*> intSettings = {Z_HOMING_SPEED, 
                                               Z_START_PRINT_SPEED};
            for(int i = 0; i < intSettings.size(); i++)
            {
                settings.Restore(intSettings[i]);
                cout << "\t" << intSettings[i] 
                     << "\t" << settings.GetInt(intSettings[i]) << endl; 
            }
            
            vector<const char*> doubleSettings = {LAYER_OVERHEAD};
            for(int i = 0; i < doubleSettings.size(); i++)
            {
                settings.Restore(doubleSettings[i]);
                cout << "\t" << doubleSettings[i] 
                     << "\t" << settings.GetDouble(doubleSettings[i]) << endl; 
            }
        }
             
        // ensure directories exist
        MakePath(settings.GetString(PRINT_DATA_DIR));
        MakePath(settings.GetString(DOWNLOAD_DIR));
        MakePath(settings.GetString(STAGING_DIR));

        // create the motor controller
        I2C_DevicePtr pMotorControllerI2cDevice =
                HardwareFactory::CreateMotorControllerI2cDevice();
        Motor motor(*pMotorControllerI2cDevice);
       
        // create the front panel
        I2C_DevicePtr pFrontPanelI2cDevice =
                HardwareFactory::CreateFrontPanelI2cDevice();
        FrontPanel frontPanel(*pFrontPanelI2cDevice); 

        // create the projector
        I2C_Device projectorI2cDevice(PROJECTOR_SLAVE_ADDRESS,
                I2C0_PORT);
        Projector projector(projectorI2cDevice);

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
        UdevMonitor usbDriveConnectionMonitor(UDEV_SUBSYSTEM_BLOCK,
                UDEV_DEVTYPE_PARTITION, UDEV_ACTION_ADD);
        UdevMonitor usbDriveDisconnectionMonitor(UDEV_SUBSYSTEM_BLOCK,
                UDEV_DEVTYPE_PARTITION, UDEV_ACTION_REMOVE);
        
        Timer motorTimeoutTimer;
        I2C_Resource motorControllerTimeout(motorTimeoutTimer,
                *pMotorControllerI2cDevice, MC_STATUS_REG);
        
        ResourcePtr pMotorControllerInterruptResource =
                HardwareFactory::CreateMotorControllerInterruptResource();
        I2C_Resource motorControllerInterrupt(*pMotorControllerInterruptResource, 
                *pMotorControllerI2cDevice, MC_STATUS_REG);
       
        ResourcePtr pFrontPanelInterruptResource =
                HardwareFactory::CreateFrontPanelInterruptResource();
        I2C_Resource buttonInterrupt(*pFrontPanelInterruptResource,
                *pFrontPanelI2cDevice, BTN_STATUS);

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
        PrintEngine pe(true, motor, projector, printerStatusQueue, exposureTimer,
                temperatureTimer, delayTimer, motorTimeoutTimer);
        
        // give it to the settings singleton as an error handler
        settings.SetErrorHandler(&pe);

        // set the screensaver time, or disable screen saver if demo mode is 
        // being requested via a button press at startup
        frontPanel.SetAwakeTime(pe.DemoModeRequested() ?
                0 : settings.GetInt(FRONT_PANEL_AWAKE_TIME));
    
        // subscribe logger first, so that it will show 
        // its output in the logs ahead of any other subscribers that actually 
        // act on those events
        Logger logger;
        eh.Subscribe(PrinterStatusUpdate, &logger);
        eh.Subscribe(MotorInterrupt, &logger);
        eh.Subscribe(ButtonInterrupt, &logger);
        eh.Subscribe(DoorInterrupt, &logger);
        if (useStdio)
            eh.Subscribe(Keyboard, &logger);
        eh.Subscribe(UICommand, &logger);
        
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
        eh.Subscribe(PrinterStatusUpdate, &frontPanel);
      
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

