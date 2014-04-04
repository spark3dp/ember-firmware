/* 
 * File:   Event.h
 * Author: greener
 *
 * Created on March 26, 2014, 4:53 PM
 */

#ifndef EVENT_H
#define	EVENT_H

#include <sys/epoll.h> 
#include <vector>

typedef void (*EventCallback)(void*);


/// The possible kinds of events handled by the EventHandler.
enum EventType
{
    // An undefined interrupt type, which should never be used.
    Undefined = 0,
    
    ///////////////////////////////////////////////////////////
    // Hardware interrupt event types should be defined together here, 
    // to ease handling.
    
    // Hardware interrupt from the front panel's UI board,
    // positive going edge-triggered.  Requires reading I2C
    // register to determine which button caused it, 
    // and whether it was pressed or held.
    ButtonInterrupt, 
    
    // Hardware interrupt from the motor board, positive going edge-triggered.  
    // Requires reading I2C register to determine the specific motor event 
    // that caused it.
    MotorInterrupt,
    
    // Fired when the user opens the door to the print platform.
    DoorInterrupt,
    
    // End of hardware interrupt event types
    ///////////////////////////////////////////////////////////
    
    // Expiration of the delay timer that the print engine sets to control its
    // state machine.  It's meaning depends on the pending print engine
    // operation.
    PrintEngineDelayEnd,
    
    // Expiration of the timer the print engine uses to make sure that motor 
    // commands have completed within a reasonable time period.
    MotorTimeout,
    
    // Expiration of the 1-second timer the print engine enables when a print is
    // in progress, in order to notify UI clients of that progress.
    PrintEnginePulse,
    
    // Fired when the print engine wants to broadcast its state.  It does this
    // whenever it changes state, or when a print is in progress and its 
    // 1-second timer fires, or when a transient UI component (Web or USB)
    // requests it.
    PrinterStatusUpdate,
    
    // Fired when a user sends a command via the web or USB applications.
    // Its payload indicates the specific command.
    UICommand,
    
    // Fired when a user requests information via the web or USB applications.
    // Its payload indicates the specific data requested (e.g. PE status, 
    // if the web app wasn't connected when the PE last broadcast status).
    UIRequest,
    
    // Fired when a user inserts or removes a USB drive.
    USBDrive,
        
    // TBD, all of these potentially from Web or USB
    // PrintDataInput,
    // SettingsInput
    // DownloadFirmware
    
    // An error has occurred.  [Not yet clear how this will be encoded 
    // and delivered.  We'll need a way of indicating which errors require that
    // the print to be canceled, and/or the system to be reset.  
    // Perhaps only the PE should inform UI blocks about any PE errors 
    // (within its status message).  Then if there are any UI errors that 
    // require stopping the print, they would simply send a stop command to 
    // the PE, rather than it needing to interpret errors?]
    Error,

    // Guardrail for valid event types.
    MaxEventTypes,
};

// Defines the interface to a class that supports callbacks.
class CallbackInterface
{
public:
    virtual void callback(EventType eventType, void*) = 0;
};

/// Defines a subscription to an event.
class Subscription
{
public:
    Subscription(CallbackInterface* pObject);
    ~Subscription()
    {
    }
    
    void Call(EventType type, void* data);
    
protected:    
    /// don't allow construction without specifying arguments
    Subscription() {} 
    
private: 
    /// the interface to be called when the event fires
    CallbackInterface* _pObject;
};

/// Defines how an event type will be handled.
class Event
{
public:
    Event(EventType eventType);
    ~Event();
    
    /// the file descriptor associated with the event type
    int _fileDescriptor;
    
	/// the flags to be used when defining the event
	uint32_t _inFlags;	
    
    /// the flags to be used when filtering received events
    uint32_t _outFlags;	
    
    // the data to be read when the event occurs
    unsigned char* _data;
    
    // the amount of data to be read
    int _numBytes;   
    
    /// the set of subscriptions for each event type
    std::vector<Subscription> _subscriptions;
    
protected:    
    /// don't allow construction without specifying arguments
    Event() {} 
};


#endif	/* EVENT_H */

