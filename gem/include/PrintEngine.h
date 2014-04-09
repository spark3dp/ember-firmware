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


namespace sc = boost::statechart;

/// the print engine state machine classes for each event
class EvReset : public sc::event<EvReset> {};
class EvSleep : public sc::event<EvSleep> {};
class EvWake : public sc::event<EvWake> {};
class EvInitialized : public sc::event<EvInitialized> {}; // TODO: this may not really be a separate event

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
    typedef sc::custom_reaction< EvSleep > reactions;
    sc::result react(const EvSleep&); 
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



#endif	/* PRINTENGINE_H */

