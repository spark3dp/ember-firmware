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
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deep_history.hpp>
#include <boost/mpl/list.hpp>

#include <PrintEngine.h>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

/// the print engine state machine classes for each event
class EvReset : public sc::event<EvReset> {};
class EvSleep : public sc::event<EvSleep> {};
class EvWake : public sc::event<EvWake> {};
class EvDoorClosed : public sc::event<EvDoorClosed> {};
class EvDoorOpened : public sc::event<EvDoorOpened> {};
class EvInitialized : public sc::event<EvInitialized> {}; 
class EvCancel : public sc::event<EvCancel> {};
class EvError : public sc::event<EvError> {};
class EvPause : public sc::event<EvPause> {};
class EvResume : public sc::event<EvResume> {};
class EvPulse : public sc::event<EvPulse> {};
class EvAtHome : public sc::event<EvAtHome> {};
class EvStartPrint : public sc::event<EvStartPrint> {};
class EvAtStartPosition : public sc::event<EvAtStartPosition> {};
class EvExposed : public sc::event<EvExposed> {};
class EvSeparated : public sc::event<EvSeparated> {};
class EvAtLayer : public sc::event<EvAtLayer> {};

/// Indicator of the event to be fired when the most recent motor command is
// completed
enum PendingMotorEvent
{
    None = 0,
    
    AtHome,
    
    AtStartPosition,
    
    Separated,
    
    AtLayer
    
};

/// the print engine state machine classes for each state
class PrinterOn;
class PrinterStateMachine : public sc::state_machine< PrinterStateMachine, PrinterOn >
{
public:
    PrinterStateMachine(PrintEngine* pPrintEngine);
    ~PrinterStateMachine();
    
    void StartOrCancelPrint();
    void PauseOrResume();
    void SleepOrWake();
    void MotionCompleted(bool successfully);
    PrintEngine* _pPrintEngine;  // the print engine containing this state machine
    
private:
    // don't allow construction without a PrintEngine
    PrinterStateMachine();
    PendingMotorEvent _pendingMotorEvent;
};

class Active;
class PrinterOn : public sc::state<PrinterOn, PrinterStateMachine, Active>
{
public:
    PrinterOn(my_context ctx);
    ~PrinterOn();
    typedef sc::custom_reaction< EvReset > reactions;
    sc::result react(const EvReset&); 
    
    
};

class Initializing;
class Active : public sc::state<Active, PrinterOn, Initializing, sc::has_deep_history >
{
public:
    Active(my_context ctx);
    ~Active();
    
    typedef mpl::list<
        sc::custom_reaction<EvSleep>, 
        sc::custom_reaction<EvDoorOpened>,
        sc::custom_reaction<EvCancel>, 
        sc::custom_reaction<EvError> > reactions;

    sc::result react(const EvSleep&); 
    sc::result react(const EvDoorOpened&); 
    sc::result react(const EvCancel&); 
    sc::result react(const EvError&); 
};

class Initializing :  public sc::state<Initializing, Active>  
{
public:
    Initializing(my_context ctx);
    ~Initializing();
    typedef sc::custom_reaction< EvInitialized > reactions;
    sc::result react(const EvInitialized&);    
};

class Sleeping : public sc::state<Sleeping, PrinterOn>
{
public:
    Sleeping(my_context ctx);
    ~Sleeping();
    typedef sc::custom_reaction< EvWake > reactions;
    sc::result react(const EvWake&);    
};

class DoorOpen : public sc::state<DoorOpen, PrinterOn>
{
public:
    DoorOpen(my_context ctx);
    ~DoorOpen();
    typedef sc::custom_reaction< EvDoorClosed > reactions;
    sc::result react(const EvDoorClosed&);    
};

class Homing : public sc::state<Homing, Active>
{
public:
    Homing(my_context ctx);
    ~Homing();
    typedef sc::custom_reaction< EvAtHome > reactions;
    sc::result react(const EvAtHome&);    
};

class Idle : public sc::state<Idle, Active>
{
public:
    Idle(my_context ctx);
    ~Idle();
    typedef sc::custom_reaction< EvStartPrint > reactions;
    sc::result react(const EvStartPrint&);    
};

class Home : public sc::state<Home, Active>
{
public:
    Home(my_context ctx);
    ~Home();
    typedef sc::custom_reaction< EvStartPrint > reactions;
    sc::result react(const EvStartPrint&);    
};

class MovingToStartPosition : public sc::state<MovingToStartPosition, Active>
{
public:
    MovingToStartPosition(my_context ctx);
    ~MovingToStartPosition();
    typedef sc::custom_reaction< EvAtStartPosition > reactions;
    sc::result react(const EvAtStartPosition&);    
};

class Exposing;
class SendingStatus;
class Printing : public sc::state<Printing, Active, Exposing, sc::has_deep_history >
{
public:
    Printing(my_context ctx);
    ~Printing();
    typedef mpl::list<
        sc::custom_reaction< EvPause>,
        sc::custom_reaction< EvPulse> > reactions;    
    
    sc::result react(const EvPause&);    
    sc::result react(const EvPulse&);    
};

class Paused : public sc::state<Paused, PrinterOn>
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
};

class Separating : public sc::state<Separating, Printing >
{
public:
    Separating(my_context ctx);
    ~Separating();
    typedef sc::custom_reaction< EvSeparated > reactions;
    sc::result react(const EvSeparated&);    
};

class MovingToLayer : public sc::state<MovingToLayer, Printing >
{
public:
    MovingToLayer(my_context ctx);
    ~MovingToLayer();
    typedef sc::custom_reaction< EvAtLayer > reactions;
    sc::result react(const EvAtLayer&);    
};


#endif	/* PRINTERSTATEMACHINE_H */

