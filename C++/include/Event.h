/* 
 * File:   Event.h
 * Author: Richard Greene
 *
 * Created on March 26, 2014, 4:53 PM
 */

#ifndef EVENT_H
#define	EVENT_H

#include <sys/epoll.h> 
#include <vector>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <I2C_Device.h>

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

// ABC defining the interface to a class that supports callbacks.
class ICallback
{
public:
    virtual void Callback(EventType eventType, void* data) = 0;
    
protected:
    void HandleImpossibleCase(EventType eventType);
};

/// Defines how an event type will be handled.
class Event
{
public:
    Event(EventType eventType);
    ~Event();
    
    // call back all the subscribers to this type of event
    void CallSubscribers(EventType type, void* data);
    
    /// the file descriptor associated with the event type
    int _fileDescriptor;
    
	/// the flags to be used when setting up epoll for this event type
	uint32_t _inFlags;	
    
    /// the flags to be used when filtering received events from epoll
    uint32_t _outFlags;	
    
    // the data to be read when this type of event occurs
    unsigned char* _data;
    
    // the amount of data to be read
    int _numBytes;   
    
    /// the set of subscriptions for each event type
    std::vector<ICallback*> _subscriptions;
    
    // indicates if special handling for hardware interrupts is needed
    bool _isHardwareInterrupt;
    
    // pointer to the I2C device from which further information must be read,
    // in order to determine the cause of a hardware interrupt
    // (or NULL for hardware such as the door switch that require no further
    // information)
    I2C_Device*  _pI2CDevice;
    
    // the address of the status register for I2C devices requiring further
    // information 
    unsigned char _statusRegister;
    
    // indicates if all available input should be sent to subscribers
    bool _handleAllAvailableInput;
    
protected:    
    /// don't allow construction without specifying arguments
    Event() {} 
};

#endif	/* EVENT_H */

