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
    
    void StartPauseOrResume();
    void SleepOrWake();
    void MotionCompleted(bool successfully);
    void SetMotorCommand(const char command, PendingMotorEvent pending, 
                         int timeoutSec = DEFAULT_MOTOR_TIMEOUT_SEC);
    PrintEngine* GetPrintEngine() { return _pPrintEngine; }
    void HandleFatalError();
    void process_event( const event_base_type & evt );
    bool IsPrinting();
    
private:
    // don't allow construction without a PrintEngine
    PrinterStateMachine();
    PrintEngine* _pPrintEngine;  // the print engine containing this state machine
    PendingMotorEvent _pendingMotorEvent;
    bool _isProcessing;
};

class DoorClosed;
class PrinterOn : public sc::state<PrinterOn, PrinterStateMachine, DoorClosed>
{
public:
    PrinterOn(my_context ctx);
    ~PrinterOn();
    typedef sc::custom_reaction< EvReset > reactions;
    sc::result react(const EvReset&); 
};

class Initializing;
class DoorClosed : public sc::state<DoorClosed, PrinterOn, Initializing, sc::has_deep_history >
{
public:
    DoorClosed(my_context ctx);
    ~DoorClosed();
    typedef mpl::list<
        sc::custom_reaction<EvDoorOpened>,
        sc::custom_reaction<EvCancel>, 
        sc::custom_reaction<EvError> > reactions;

    sc::result react(const EvDoorOpened&); 
    sc::result react(const EvCancel&); 
    sc::result react(const EvError&); 
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
    typedef sc::custom_reaction< EvDoorClosed > reactions;
    sc::result react(const EvDoorClosed&);    
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
    typedef sc::custom_reaction< EvStartPrint > reactions;
    sc::result react(const EvStartPrint&);    
};

class Home : public sc::state<Home, DoorClosed>
{
public:
    Home(my_context ctx);
    ~Home();
    typedef sc::custom_reaction< EvStartPrint > reactions;
    sc::result react(const EvStartPrint&);    
};

class PrintSetup : public sc::state<PrintSetup, DoorClosed>
{
public:
    PrintSetup(my_context ctx);
    ~PrintSetup();
    typedef sc::custom_reaction< EvGotSetting > reactions;
    sc::result react(const EvGotSetting&);    
};

class MovingToStartPosition : public sc::state<MovingToStartPosition, DoorClosed>
{
public:
    MovingToStartPosition(my_context ctx);
    ~MovingToStartPosition();
    typedef sc::custom_reaction< EvAtStartPosition > reactions;
    sc::result react(const EvAtStartPosition&);    
};

class Exposing;
class Printing : public sc::state<Printing, DoorClosed, Exposing, sc::has_deep_history >
{
public:
    Printing(my_context ctx);
    ~Printing();
    typedef sc::custom_reaction< EvPause> reactions;    
    
    sc::result react(const EvPause&);       
};

class Paused : public sc::state<Paused, DoorClosed>
{
public:
    Paused(my_context ctx);
    ~Paused();
    typedef sc::custom_reaction< EvResume > reactions;
    sc::result react(const EvResume&);    
};

class Exposing : public sc::state<Exposing, Printing >
{
public:
    Exposing(my_context ctx);
    ~Exposing();
    typedef sc::custom_reaction< EvExposed > reactions;
    sc::result react(const EvExposed&);  
    static void ClearPendingExposureInfo();
    
private:
    static double _remainingExposureTimeSec;
    static int _previousLayer;
};

class Separating : public sc::state<Separating, Printing >
{
public:
    Separating(my_context ctx);
    ~Separating();
    typedef sc::custom_reaction< EvSeparated > reactions;
    sc::result react(const EvSeparated&);    
};

class EndingPrint : public sc::state<EndingPrint, Printing >
{
public:
    EndingPrint(my_context ctx);
    ~EndingPrint();
    typedef sc::custom_reaction< EvPrintEnded > reactions;
    sc::result react(const EvPrintEnded&);    
};

#endif	/* PRINTERSTATEMACHINE_H */

