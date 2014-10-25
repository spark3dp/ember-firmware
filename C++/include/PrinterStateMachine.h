/* 
 * File:   PrinterStateMachine.h
 * Author: Richard Greene
 *
 * Defines all classes used by the PrintEngine's state machine.
 * 
 * Created on April 14, 2014, 10:55 AM
 */

#ifndef PRINTERSTATEMACHINE_H
#define	PRINTERSTATEMACHINE_H

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deep_history.hpp>
#include <boost/mpl/list.hpp>

#include <PrintEngine.h>
#include <Projector.h>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

/// the print engine state machine classes for each event
class EvReset : public sc::event<EvReset> {};
class EvDoorClosed : public sc::event<EvDoorClosed> {};
class EvDoorOpened : public sc::event<EvDoorOpened> {};
class EvInitialized : public sc::event<EvInitialized> {}; 
class EvCancel : public sc::event<EvCancel> {};
class EvNoCancel : public sc::event<EvNoCancel> {};
class EvError : public sc::event<EvError> {};
class EvPause : public sc::event<EvPause> {};
class EvResume : public sc::event<EvResume> {};
class EvAtHome : public sc::event<EvAtHome> {};
class EvStartPrint : public sc::event<EvStartPrint> {};
class EvGotSetting : public sc::event<EvGotSetting> {};
class EvAtStartPosition : public sc::event<EvAtStartPosition> {};
class EvExposed : public sc::event<EvExposed> {};
class EvSeparated : public sc::event<EvSeparated> {};
class EvPrintEnded : public sc::event<EvPrintEnded> {};
class EvShowVersion : public sc::event<EvShowVersion> {};
class EvConnected : public sc::event<EvConnected> {};
class EvRegistered : public sc::event<EvRegistered> {};
class EvStartCalibration : public sc::event<EvStartCalibration> {};
// front panel button events
class EvLeftButton : public sc::event<EvLeftButton> {};
class EvRightButton : public sc::event<EvRightButton> {};
class EvLeftAndRightButton : public sc::event<EvLeftAndRightButton> {};
class EvRightButtonHold : public sc::event<EvRightButtonHold> {};

/// Indicator of the event to be fired when the most recent motor command is
// completed
enum PendingMotorEvent
{
    None = 0,
    
    Initialized,
    
    AtHome,
    
    AtStartPosition,
    
    Separated,
    
    PrintEnded
};

/// the print engine state machine classes for each state
class PrinterOn;
class PrinterStateMachine : public sc::state_machine< PrinterStateMachine, PrinterOn >
{
public:
    PrinterStateMachine(PrintEngine* pPrintEngine);
    ~PrinterStateMachine();
    
    void MotionCompleted(bool successfully);
    void SetMotorCommand(const char command, PendingMotorEvent pending, 
                         int timeoutSec = DEFAULT_MOTOR_TIMEOUT_SEC);
    PrintEngine* GetPrintEngine() { return _pPrintEngine; }
    void HandleFatalError();
    void process_event( const event_base_type & evt );
    bool IsMotorMoving();
    void CancelPrint();
    UISubState _homingSubState;
    
private:
    // don't allow construction without a PrintEngine
    PrinterStateMachine();
    PrintEngine* _pPrintEngine;  // the print engine containing this state machine
    PendingMotorEvent _pendingMotorEvent;
    bool _isProcessing;
};

class DoorClosed;
class PrinterOn : public sc::state<PrinterOn, PrinterStateMachine, DoorClosed, sc::has_deep_history >
{
public:
    PrinterOn(my_context ctx);
    ~PrinterOn();
    typedef mpl::list<
        sc::custom_reaction<EvReset>,
        sc::custom_reaction<EvShowVersion>, 
        sc::custom_reaction<EvError> > reactions;
    sc::result react(const EvReset&); 
    sc::result react(const EvShowVersion&);
    sc::result react(const EvError&); 
};


class Initializing;
class DoorClosed : public sc::state<DoorClosed, PrinterOn, Initializing, sc::has_deep_history >
{
public:
    DoorClosed(my_context ctx);
    ~DoorClosed();
    typedef mpl::list<
        sc::custom_reaction<EvDoorOpened> > reactions;

    sc::result react(const EvDoorOpened&); 
};

class Initializing :  public sc::state<Initializing, DoorClosed>  
{
public:
    Initializing(my_context ctx);
    ~Initializing();
    typedef sc::custom_reaction< EvInitialized > reactions;
    sc::result react(const EvInitialized&);    
};

class DoorOpen : public sc::state<DoorOpen, PrinterOn>
{
public:
    DoorOpen(my_context ctx);
    ~DoorOpen();
    typedef mpl::list<
        sc::custom_reaction< EvDoorClosed>,
        sc::custom_reaction< EvAtStartPosition>,
        sc::custom_reaction< EvSeparated> > reactions;
    sc::result react(const EvDoorClosed&);    
    sc::result react(const EvAtStartPosition&);
    sc::result react(const EvSeparated&);

private:
    bool _atStartPosition;
    bool _separated;
};

class Homing : public sc::state<Homing, DoorClosed>
{
public:
    Homing(my_context ctx);
    ~Homing();
    typedef sc::custom_reaction< EvAtHome > reactions;
    sc::result react(const EvAtHome&);    
};

class Idle : public sc::state<Idle, PrinterOn>
{
public:
    Idle(my_context ctx);
    ~Idle();
    typedef mpl::list<
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvRightButtonHold> > reactions;
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvRightButton&);  
    sc::result react(const EvRightButtonHold&);  
};
    
class Calibrate : public sc::state<Calibrate, PrinterOn>
{
public:
    Calibrate(my_context ctx);
    ~Calibrate();
    typedef mpl::list<
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRightButton> > reactions;
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvRightButton&);  
};

class MovingToCalibration : public sc::state<MovingToCalibration, PrinterOn>
{
public:
    MovingToCalibration(my_context ctx);
    ~MovingToCalibration();
    typedef mpl::list<
        sc::custom_reaction<EvAtStartPosition> > reactions;
    sc::result react(const EvAtStartPosition&);   
};

class Calibrating : public sc::state<Calibrating, PrinterOn>
{
public:
    Calibrating(my_context ctx);
    ~Calibrating();
    typedef mpl::list<
        sc::custom_reaction<EvLeftButton> > reactions;
    sc::result react(const EvLeftButton&);   
};
    
class Registering : public sc::state<Registering, PrinterOn>
{
public:
    Registering(my_context ctx);
    ~Registering();
    typedef mpl::list<
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRegistered> > reactions;
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvRegistered&);  
};

class Registered : public sc::state<Registered, PrinterOn>
{
public:
    Registered(my_context ctx);
    ~Registered();
    typedef mpl::list<
        sc::custom_reaction<EvRightButton> > reactions;
    sc::result react(const EvRightButton&);   
};

class ConfirmCancel : public sc::state<ConfirmCancel, DoorClosed>
{
public:
    ConfirmCancel(my_context ctx);
    ~ConfirmCancel();
    typedef mpl::list<
        sc::custom_reaction<EvCancel>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvNoCancel>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvSeparated> > reactions;
    sc::result react(const EvCancel&);
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvNoCancel&);  
    sc::result react(const EvRightButton&);  
    sc::result react(const EvSeparated&);  
    
private:
    bool _separated;        
};
    

class Home : public sc::state<Home, DoorClosed>
{
public:
    Home(my_context ctx);
    ~Home();
        typedef mpl::list<
        sc::custom_reaction<EvStartPrint>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRightButton>,        
        sc::custom_reaction<EvLeftAndRightButton>,
        sc::custom_reaction<EvStartCalibration>,
        sc::custom_reaction<EvRightButtonHold>,
        sc::custom_reaction<EvConnected> > reactions;
    sc::result react(const EvStartPrint&); 
    sc::result react(const EvLeftButton&); 
    sc::result react(const EvRightButton&); 
    sc::result react(const EvLeftAndRightButton&); 
    sc::result react(const EvRightButtonHold&);     
    sc::result react(const EvConnected&); 
    sc::result react(const EvStartCalibration&); 
    
private:
    sc::result TryStartPrint();
};

class ShowingVersion : public sc::state<ShowingVersion, PrinterStateMachine >
{
public:
    ShowingVersion(my_context ctx);
    ~ShowingVersion();
    typedef sc::custom_reaction< EvLeftButton > reactions;
    sc::result react(const EvLeftButton&); 
};

class PrintSetup : public sc::state<PrintSetup, DoorClosed>
{
public:
    PrintSetup(my_context ctx);
    ~PrintSetup();
    typedef sc::custom_reaction< EvGotSetting > reactions;
    sc::result react(const EvGotSetting&);    
};


class Paused : public sc::state<Paused, DoorClosed>
{
public:
    Paused(my_context ctx);
    ~Paused();
    typedef mpl::list<
        sc::custom_reaction<EvResume>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvSeparated> > reactions;
    sc::result react(const EvResume&);    
    sc::result react(const EvLeftButton&);    
    sc::result react(const EvRightButton&);   
    sc::result react(const EvSeparated&);   
    
private:
    bool _separated;    
};

class MovingToStartPosition : public sc::state<MovingToStartPosition, DoorClosed>
{
public:
    MovingToStartPosition(my_context ctx);
    ~MovingToStartPosition();
    typedef sc::custom_reaction<EvAtStartPosition> reactions;
    sc::result react(const EvAtStartPosition&);       
};

class Exposing;
class PrintingLayer : public sc::state<PrintingLayer, DoorClosed, Exposing, sc::has_deep_history >
{
public:
    PrintingLayer(my_context ctx);
    ~PrintingLayer();  
    typedef mpl::list<
        sc::custom_reaction<EvPause>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvRightButton> > reactions;
    sc::result react(const EvPause&);    
    sc::result react(const EvLeftButton&);    
    sc::result react(const EvRightButton&);         
};

class Exposing : public sc::state<Exposing, PrintingLayer>
{
public:
    Exposing(my_context ctx);
    ~Exposing();
        typedef mpl::list<
        sc::custom_reaction<EvExposed>,
        sc::custom_reaction<EvCancel> > reactions;
    sc::result react(const EvExposed&);  
    sc::result react(const EvCancel&);  
    static void ClearPendingExposureInfo();
    
private:
    static double _remainingExposureTimeSec;
    static int _previousLayer;
};

class Separating : public sc::state<Separating, PrintingLayer >
{
public:
    Separating(my_context ctx);
    ~Separating();
    typedef sc::custom_reaction< EvSeparated > reactions;
    sc::result react(const EvSeparated&);    
};

class EndingPrint : public sc::state<EndingPrint, DoorClosed >
{
public:
    EndingPrint(my_context ctx);
    ~EndingPrint();
    typedef sc::custom_reaction< EvPrintEnded > reactions;
    sc::result react(const EvPrintEnded&);    
};

#endif	/* PRINTERSTATEMACHINE_H */

