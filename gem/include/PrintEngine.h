/* 
 * File:   PrintEngine.h
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;


/// the print engine state machine classes for each event
class EvReset : public sc::event<EvReset> {};
class EvSleep : public sc::event<EvSleep> {};
class EvWake : public sc::event<EvWake> {};
class EvDoorClosed : public sc::event<EvDoorClosed> {};
class EvDoorOpened : public sc::event<EvDoorOpened> {};
// TODO: EvInitialized may not really be a separate event, 
// since Initializing just immediately goes to Homing on completion,
// but perhaps on completing initilization, the Initializing state will simply post an initialized event to itself?
// post_event( EvEvInitialized() ); ???
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

/// the print engine state machine classes for each state
class PrinterOn;
class PrinterStateMachine : public sc::state_machine< PrinterStateMachine, PrinterOn >
{
public:
    PrinterStateMachine();
    ~PrinterStateMachine();
};

class Active;
class PrinterOn : public sc::simple_state<PrinterOn, PrinterStateMachine, Active>
{
public:
    PrinterOn();
    ~PrinterOn();
    typedef sc::custom_reaction< EvReset > reactions;
    sc::result react(const EvReset&); 
};

class Initializing;
class Active : public sc::simple_state<Active, PrinterOn, Initializing>
{
public:
    Active();
    ~Active();
    
    typedef mpl::list<
        sc::custom_reaction<EvSleep>, 
        sc::custom_reaction<EvDoorOpened> > reactions;

    sc::result react(const EvSleep&); 

    sc::result react(const EvDoorOpened&); 
};

class Initializing :  public sc::simple_state<Initializing, Active>  
{
public:
    Initializing();
    ~Initializing();
    typedef sc::custom_reaction< EvInitialized > reactions;
    sc::result react(const EvInitialized&);    
};

class Sleeping : public sc::simple_state<Sleeping, PrinterOn>
{
public:
    Sleeping();
    ~Sleeping();
    typedef sc::custom_reaction< EvWake > reactions;
    sc::result react(const EvWake&);    
};

class DoorOpen : public sc::simple_state<DoorOpen, PrinterOn>
{
public:
    DoorOpen();
    ~DoorOpen();
    typedef sc::custom_reaction< EvDoorClosed > reactions;
    sc::result react(const EvDoorClosed&);    
};


class Homing;
class Idle;
class Home;
class MovingToStartPosition;
class Printing;
class Paused;
class SendingStatus;
class Exposing;
class Separating;
class MovingToLayer;

#endif	/* PRINTENGINE_H */

