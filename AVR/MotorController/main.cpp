#include <avr/interrupt.h>

#include "i2c.h"
#include "planner.h"

#include "Hardware.h"
#include "CommandBuffer.h"
#include "MotorController.h"
#include "CommandMap.h"
#include "StateMachine.h"

#ifdef DEBUG
#include "Debug.h"
#include <avr/pgmspace.h>
#endif /* DEBUG */

#define I2C_ADDRESS 0x10

MotorController_t mcState; // Instance of the global state struct, all members initialized to 0
CommandBuffer commandBuffer;
volatile uint8_t limitSwitchHit;

/*
 * this function will run when a master somewhere else on the bus
 * addresses us and wishes to write data to us
 */

void i2cSlaveReceiveService(uint8_t receiveDataLength, uint8_t* receiveData)
{
    Status status;
    for(uint8_t i = 0; i < receiveDataLength; i++)
    {
        status = commandBuffer.AddByte(*receiveData++);
        if (status != MC_STATUS_SUCCESS)
        {
            mcState.status = status;
            mcState.error = true;
        }
    }
}

/*
 * this function will run when a master reads data
 */
uint8_t i2cSlaveTransmitService(uint8_t transmitDataLengthMax, uint8_t* transmitData)
{
    // The I2C bus last received the address of the register to read from
    // Remove it from the command buffer since it is not part of a command
    commandBuffer.RemoveLastByte();
    // Status code
    transmitData[0] = mcState.status;
    return 1;
}

/*
 * Check limit switch interrupt flag and raise limit reached event if set
 */

void QueryLimitSwitchInterrupt()
{
    if (limitSwitchHit)
    {
        limitSwitchHit = 0;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, AxisLimitReached);
    }
}

/*
 * Check for incoming I2C command and raise appropriate event if new command exists in buffer
 */

void QueryCommandBuffer()
{
    if (!commandBuffer.IsEmpty())
    {
        Command command;
        EventData eventData;

        commandBuffer.GetCommand(command);
        uint8_t eventCode = CommandMap::GetEventCode(command.Register(), command.Action());

#ifdef DEBUG
        printf_P(PSTR("INFO: Received i2c message: register %2x, command: %d, value: %ld, event code: %d\n"),
                command.Register(), command.Action(), command.Parameter(), eventCode);
#endif

        if (eventCode == 0)
        {
#ifdef DEBUG
            printf_P(PSTR("ERROR: Command does not have corresponding state machine event, not handling\n"));
#endif
            return;
        }

        eventData.command = command.Action();
        eventData.parameter = command.Parameter();
        MotorController_State_Machine_Event(&mcState, eventData, eventCode);
    }
}

/*
 * Check motion complete flag and raise motion complete event if set
 */

void QueryMotionComplete()
{
    if (mcState.motionComplete)
    {
        mcState.motionComplete = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, MotionComplete);
    }
}

/*
 * Check for dequeued events
 * Returns true if a dequeued event found
 * The return value allows the main loop to check for additional queued events before
 * checking for new commands
 */

bool QueryEventQueue()
{
    if (mcState.queuedEvent)
    {
        mcState.queuedEvent = false;
        MotorController_State_Machine_Event(&mcState, mcState.queuedEventData, mcState.queuedEventCode);
        return true;
    }
    return false;
}

/*
 * Check resume flag and raise resume event if set
 */

void QueryResume()
{
    if (mcState.resume)
    {
        mcState.resume = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, ResumeRequested);
    }
}

/*
 * Check for error status and raise error event if set
 */

void QueryError()
{
    if (mcState.error)
    {
        mcState.error = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, ErrorEncountered);
    }
}

int main()
{
#ifdef DEBUG
    // Turn on LED
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PB0);
#endif

    // Disable interrupts
    cli();

#ifdef DEBUG
    Debug::Initialize();
#endif

    // Initialize I2C bus
    i2cInit();

    // Configure I2C address
    i2cSetLocalDeviceAddr(I2C_ADDRESS, TRUE);

    // Set I2C send/receive handlers
    i2cSetSlaveReceiveHandler(i2cSlaveReceiveService);
    i2cSetSlaveTransmitHandler(i2cSlaveTransmitService);

    // Initialize I/O and subsystems
    MotorController::Initialize(&mcState);

    // Enable interrupts
    sei();

#ifdef DEBUG
    printf_P(PSTR("INFO: Motor controller firmware initialized\n"));
#endif

    // Initialize the state machine to the Disabled state
    MotorController_State_Machine_Init(&mcState, Disabled);

    for(;;)
    {
        QueryError();
        QueryLimitSwitchInterrupt();
        mp_plan_hold_callback();
        QueryMotionComplete();
        QueryResume();

        /*
         * If QueryEventQueue() returns true, at least one queued event exists
         * If the queue contains one queued event, it may contain additional events
         * Skip querying the command buffer until the event queue empties to ensure commands are handled
         * in the order the controller receives them
         */

        if(QueryEventQueue()) continue;
        QueryCommandBuffer();
    }
}

ISR (PCINT2_vect)
{
    limitSwitchHit = 1;

    // Disable interrupts immediately to avoid spurious interrupts
    LIMIT_SW_PCMSK &= ~Z_AXIS_LIMIT_SW_PCINT_BM;
    LIMIT_SW_PCMSK &= ~R_AXIS_LIMIT_SW_PCINT_BM;
}
