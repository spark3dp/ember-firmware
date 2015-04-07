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
class EvRequestPause : public sc::event<EvRequestPause> {};
class EvRotatedForPause : public sc::event<EvRotatedForPause> {};
class EvAtPause : public sc::event<EvAtPause> {};
class EvRotatedForResume : public sc::event<EvRotatedForResume> {};
class EvAtResume : public sc::event<EvAtResume> {};
class EvResume : public sc::event<EvResume> {};
class EvAtHome : public sc::event<EvAtHome> {};
class EvStartPrint : public sc::event<EvStartPrint> {};
class EvGotSetting : public sc::event<EvGotSetting> {};
class EvAtStartPosition : public sc::event<EvAtStartPosition> {};
class EvDelayEnded : public sc::event<EvDelayEnded> {};
class EvExposed : public sc::event<EvExposed> {};
class EvSeparated : public sc::event<EvSeparated> {};
class EvShowVersion : public sc::event<EvShowVersion> {};
class EvConnected : public sc::event<EvConnected> {};
class EvRegistered : public sc::event<EvRegistered> {};
// front panel button events
class EvLeftButton : public sc::event<EvLeftButton> {};
class EvRightButton : public sc::event<EvRightButton> {};
class EvLeftAndRightButton : public sc::event<EvLeftAndRightButton> {};
class EvLeftButtonHold : public sc::event<EvLeftButtonHold> {};
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
    RotatedForPause,   
    AtPauseAndInspect, 
    RotatedForResume,
    AtResume,
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
    void SetMotorCommand(const char* commandFormatString, int value, 
                         PendingMotorEvent pending,
                         int timeoutSec = DEFAULT_MOTOR_TIMEOUT_SEC);
    PrintEngine* GetPrintEngine() { return _pPrintEngine; }
    void HandleFatalError();
    void process_event( const event_base_type & evt );
    bool IsMotorMoving();
    void CancelPrint();
    PrintEngineState AfterSeparation();
    UISubState _homingSubState;
    bool _atInspectionPosition;
    
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
        sc::custom_reaction< EvSeparated>,
        sc::custom_reaction< EvRotatedForPause>,
        sc::custom_reaction< EvAtPause>,
        sc::custom_reaction< EvRotatedForResume>,
        sc::custom_reaction< EvAtResume> > reactions;
    sc::result react(const EvDoorClosed&);    
    sc::result react(const EvAtStartPosition&);
    sc::result react(const EvSeparated&);
    sc::result react(const EvRotatedForPause&);
    sc::result react(const EvAtPause&);
    sc::result react(const EvRotatedForResume&);
    sc::result react(const EvAtResume&);

private:
    bool _atStartPosition;
    bool _separated;
    bool _rotatedForPause;
    bool _atPause;
    bool _rotatedForResume;
    bool _atResume;
};

class Homing : public sc::state<Homing, DoorClosed>
{
public:
    Homing(my_context ctx);
    ~Homing();
    typedef sc::custom_reaction< EvAtHome > reactions;
    sc::result react(const EvAtHome&);    
};

class Error : public sc::state<Error, PrinterOn>
{
public:
    Error(my_context ctx);
    ~Error();
    typedef mpl::list<
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvLeftButtonHold> > reactions;
    sc::result react(const EvRightButton&);  
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvLeftButtonHold&);  
};
    

class Calibrating : public sc::state<Calibrating, PrinterOn>
{
public:
    Calibrating(my_context ctx);
    ~Calibrating();
    typedef mpl::list<
        sc::custom_reaction<EvRightButton> > reactions;
    sc::result react(const EvRightButton&);   
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

class ConfirmCancel : public sc::state<ConfirmCancel, DoorClosed>
{
public:
    ConfirmCancel(my_context ctx);
    ~ConfirmCancel();
    typedef mpl::list<
        sc::custom_reaction<EvCancel>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvResume>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvSeparated> > reactions;
    sc::result react(const EvCancel&);
    sc::result react(const EvRightButton&);  
    sc::result react(const EvResume&);  
    sc::result react(const EvLeftButton&);  
    sc::result react(const EvSeparated&);  
  
    static bool _fromPaused;
    static bool _fromJammed;
    
private:
    static bool _separated;  
};
    

class Home : public sc::state<Home, DoorClosed>
{
public:
    Home(my_context ctx);
    ~Home();
        typedef mpl::list<
        sc::custom_reaction<EvStartPrint>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvLeftButton>,        
        sc::custom_reaction<EvLeftButtonHold>,
        sc::custom_reaction<EvConnected> > reactions;
    sc::result react(const EvStartPrint&); 
    sc::result react(const EvRightButton&); 
    sc::result react(const EvLeftButton&); 
    sc::result react(const EvLeftButtonHold&); 
    sc::result react(const EvConnected&); 
    
private:
    sc::result TryStartPrint();
};

class ShowingVersion : public sc::state<ShowingVersion, PrinterStateMachine >
{
public:
    ShowingVersion(my_context ctx);
    ~ShowingVersion();
    typedef mpl::list<
            sc::custom_reaction<EvRightButton>,
            sc::custom_reaction< EvReset > > reactions;
    sc::result react(const EvRightButton&); 
    sc::result react(const EvReset&); 
};

class PrintSetup : public sc::state<PrintSetup, DoorClosed>
{
public:
    PrintSetup(my_context ctx);
    ~PrintSetup();
    typedef mpl::list<
            sc::custom_reaction<EvRightButton>,
            sc::custom_reaction<EvGotSetting> > reactions;
    sc::result react(const EvRightButton&);    
    sc::result react(const EvGotSetting&);    
};

class RotatingForPause : public sc::state<RotatingForPause, DoorClosed>
{
public:
    RotatingForPause(my_context ctx);
    ~RotatingForPause();
    typedef mpl::list<
        sc::custom_reaction<EvRotatedForPause> > reactions;
    sc::result react(const EvRotatedForPause&);        
};

class MovingToPause : public sc::state<MovingToPause, DoorClosed>
{
public:
    MovingToPause(my_context ctx);
    ~MovingToPause();
    typedef mpl::list<
        sc::custom_reaction<EvAtPause> > reactions;
    sc::result react(const EvAtPause&);    
};

class Paused : public sc::state<Paused, DoorClosed>
{
public:
    Paused(my_context ctx);
    ~Paused();
    typedef mpl::list<
        sc::custom_reaction<EvResume>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvCancel> > reactions;
    sc::result react(const EvResume&);    
    sc::result react(const EvRightButton&);    
    sc::result react(const EvLeftButton&);   
    sc::result react(const EvCancel&);       
};

class RotatingForResume : public sc::state<RotatingForResume, DoorClosed>
{
public:
    RotatingForResume(my_context ctx);
    ~RotatingForResume();
    typedef mpl::list<
        sc::custom_reaction<EvRotatedForResume> > reactions;
    sc::result react(const EvRotatedForResume&);        
};

class MovingToResume : public sc::state<MovingToResume, DoorClosed>
{
public:
    MovingToResume(my_context ctx);
    ~MovingToResume();
    typedef mpl::list<
        sc::custom_reaction<EvAtResume> > reactions;
    sc::result react(const EvAtResume&);    
};

class Jammed : public sc::state<Jammed, DoorClosed>
{
public:
    Jammed(my_context ctx);
    ~Jammed();
    typedef mpl::list<
        sc::custom_reaction<EvResume>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvLeftButton>,
        sc::custom_reaction<EvCancel> > reactions;
    sc::result react(const EvResume&);    
    sc::result react(const EvRightButton&);    
    sc::result react(const EvLeftButton&);   
    sc::result react(const EvCancel&);       
};

class MovingToStartPosition : public sc::state<MovingToStartPosition, DoorClosed>
{
public:
    MovingToStartPosition(my_context ctx);
    ~MovingToStartPosition();
    typedef mpl::list<
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvAtStartPosition> > reactions;
    sc::result react(const EvAtStartPosition&);
    sc::result react(const EvRightButton&);    
};

class PreExposureDelay;
class PrintingLayer : public sc::state<PrintingLayer, DoorClosed, PreExposureDelay, sc::has_deep_history >
{
public:
    PrintingLayer(my_context ctx);
    ~PrintingLayer();  
    typedef mpl::list<
        sc::custom_reaction<EvRequestPause>,
        sc::custom_reaction<EvRightButton>,
        sc::custom_reaction<EvLeftButton> > reactions;
    sc::result react(const EvRequestPause&);    
    sc::result react(const EvRightButton&);    
    sc::result react(const EvLeftButton&);         
};

class PreExposureDelay : public sc::state<PreExposureDelay, PrintingLayer>
{
public:
    PreExposureDelay(my_context ctx);
    ~PreExposureDelay();
        typedef mpl::list<
        sc::custom_reaction<EvDelayEnded>,
        sc::custom_reaction<EvCancel> > reactions;
    sc::result react(const EvDelayEnded&);  
    sc::result react(const EvCancel&);  
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


#endif	/* PRINTERSTATEMACHINE_H */

