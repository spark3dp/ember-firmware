//  File:   EventType.h
//  Defines the types of events handled by the EventHandler
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

#ifndef EVENTTYPE_H
#define	EVENTTYPE_H

// The possible kinds of events handled by the EventHandler.
enum EventType
{
    // An undefined interrupt type, which should never be used.
    Undefined = 0,
    
    // Hardware interrupt from the front panel,
    // positive going edge-triggered.  Requires reading I2C
    // register to determine which button caused it, 
    // and whether it was pressed or held.
    ButtonInterrupt, 
    
    // Hardware interrupt from the motor controller, positive going 
    // edge-triggered.  Requires reading I2C register to determine the specific 
    // motor event that caused it.
    MotorInterrupt,
    
    // Fired when the user opens the door to the print platform.
    DoorInterrupt,
    
    // Fired when the resin tray rotates over the rotation sensor.
    RotationInterrupt,
    
    // Expiration of the timer used by the print engine for various delays.
    DelayEnd,
    
    // Expiration of the timer used by the print engine to control exposure of 
    // each layer.
    ExposureEnd,
    
    // Expiration of the timer the print engine uses to make sure that motor 
    // commands have completed within a reasonable time period.
    MotorTimeout,
    
    // Expiration of the timer that tells when we should measure temperature.
    TemperatureTimer,
       
    // Fired when the print engine wants to broadcast its state.  It does this
    // whenever it changes state, or UI substate.
    PrinterStatusUpdate,
    
    // Fired when a command has been received from a UI component.
    // Its payload indicates the specific command.
    UICommand,
    
    // Fired when a user enters a character via stdin
    Keyboard,

    // Fired when the operating system sends a signal
    Signal,

    // Fired when a user attaches a usb drive
    USBDriveConnected,

    // Fired when a user removes a usb drive
    USBDriveDisconnected,
    
    // Guardrail for valid event types.
    MaxEventTypes,
};


#endif    // EVENTTYPE_H
