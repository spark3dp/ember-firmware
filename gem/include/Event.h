/* 
 * File:   Event.h
 * Author: greener
 *
 * Created on March 26, 2014, 4:53 PM
 */

#ifndef EVENT_H
#define	EVENT_H

#include <sys/epoll.h> 

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
    
    // Hardware interrupt from the motor board, positive going edge-triggered.  
    // Requires reading I2C register to determine the specific motor event 
    // that caused it.
    MotorInterrupt,
    
    // Expiration of the delay timer that the print engine sets to control its
    // state machine.  It's meaning depends on the pending print engine
    // operation.
    EndOfPrintEngineDelay,
    
    // Expiration of the timer the print engine uses to make sure that motor 
    // commands have completed within a reasonable time period.
    MotorTimeout,
    
    // Fired when the print engine changes state, and every second when 
    // a print is in progress.
    // [So this will need to be a timerfd_ event, and also one that can be 
    // triggered immediately somehow, e.g. by setting the timer interval
    // momentarily to 0? Perhaps 'twere better to have separate events 
    // (possibly both subscribed to by each client with the same callback),
    // e.g. PrintTimerStatus and PrinterStatus.]
    PrinterStatus,
    
    // Fired when the user opens the door to the print platform.
    // [This will presumably be a case of either ButtonInterrupt or MotorInterrupt]
    //DoorOpened,
    
    // Fired when a user sends a command via the web or USB applications.
    // Its payload indicates the specific command.
    UICommand,
    
    // Fired when a user requests information via the web or USB applications.
    // Its payload indicates the specific data requested.
    UIRequest,
    
    // Fired when a user inserts or removes a USB drive.
    USBDrive,
        
    // TBD, all of these potentially from Web or USB
    // PrintDataInput,
    // SettingsInput
    // DownloadFirmware
    
    // An error has occurred.  [Not yet clear how this will be encoded 
    // and delivered.  We'll need a way of indicating which errors require that
    // the print to be canceled, and/or the system to be reset]
    Error,

    // Guardrail for valid event types.
    Invalid,
};

/// Defines an event that may be subscribed to.
class Event
{
public:
    Event(EventType type, int fileDescriptor, 
          uint32_t inFlags, uint32_t outFlags);
    ~Event();
    
protected:    
    /// don't allow construction without specifying arguments
    Event() {} 
    
private:
    /// the type of this event
    EventType _type;
    
    /// the descriptor for the "file" whose change signals the event
    int _fileDescriptor;

	/// the flags to be used when defining the event
	uint32_t _inFlags;	
    
    /// the flags to be used when filtering received events
    uint32_t _outFlags;	
};


/// Defines a subscription to an event.
class Subscription
{
public:
    Subscription(EventType type, void (*callback)(void*));
    ~Subscription();
    
protected:    
    /// don't allow construction without specifying arguments
    Subscription() {} 
    
private:
    /// the type of this event
    EventType _type;
    
    /// the function to be called when the event fires
    void (*_callback)(void*);
};

#endif	/* EVENT_H */

