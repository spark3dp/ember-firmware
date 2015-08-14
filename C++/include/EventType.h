/* 
 * File:   EventType.h
 * Author: Richard Greene and Jason Lefley
 *
 * Created on March 26, 2014, 4:53 PM
 */

#ifndef EVENTTYPE_H
#define	EVENTTYPE_H

/// The possible kinds of events handled by the EventHandler.
enum EventType
{
    // An undefined interrupt type, which should never be used.
    Undefined = 0,
    
    // Hardware interrupt from the front panel's UI board,
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
    // whenever it changes state, or when a UI component (Web or USB)
    // requests it.
    PrinterStatusUpdate,
    
    // Fired when a command has been received from a UI component.
    // Its payload indicates the specific command.
    UICommand,
    
    // Fired when a user enters a character via stdin
    Keyboard,
        
    // TBD, all of these potentially from Web or USB
    // PrintDataInput,
    // SettingsInput
    // DownloadFirmware
    
    // Guardrail for valid event types.
    MaxEventTypes,
};


#endif	/* EVENTTYPE_H */
