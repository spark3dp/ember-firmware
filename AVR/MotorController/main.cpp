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

CommandBuffer commandBuffer;
volatile uint8_t limitSwitchHit;
// Instance of the global state struct, all members initialized to 0
MotorController_t mcState;

/*
 * this function will run when a master somewhere else on the bus
 * addresses us and wishes to write data to us
 */

void i2cSlaveReceiveService(uint8_t receiveDataLength, uint8_t* receiveData)
{
    for(uint8_t i = 0; i < receiveDataLength; i++)
        commandBuffer.AddByte(*receiveData++);
}

void CheckForLimitSwitchInterrupt()
{
    if (limitSwitchHit)
    {
        limitSwitchHit = 0;
        MotorController_State_Machine_Event(&mcState, NULL, AxisLimitReached);
    }
}

void CheckForIncomingCommand()
{
    Command command;

    if (commandBuffer.ReceivedCommandCount() > 0)
    {
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

        MotorController_State_Machine_Event(&mcState, &command, eventCode);
    }
}

void QueryMotionComplete()
{
    if (mcState.motionComplete)
    {
        mcState.motionComplete = false;
#ifdef DEBUG
        printf_P(PSTR("DEBUG: motionComplete flag set\n"));
#endif
        MotorController_State_Machine_Event(&mcState, NULL, MotionComplete);
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
    //i2cSetSlaveTransmitHandler(i2cSlaveTransmitService);

    // Initialize I/O and subsystems
    MotorController::Initialize(&mcState);

    // Enable interrupts
    sei();

#ifdef DEBUG
    printf_P(PSTR("INFO: Motor controller firmware initialized\n"));
#endif

    // Initialize the state machine to the Ready state
    MotorController_State_Machine_Init(&mcState, Ready);

    for(;;)
    {
        CheckForLimitSwitchInterrupt();
        mp_plan_hold_callback();
        QueryMotionComplete();
        CheckForIncomingCommand();
    }
}

ISR (PCINT2_vect)
{
    limitSwitchHit = 1;

    // Disable interrupts immediately to avoid spurious interrupts
    LIMIT_SW_PCMSK &= ~Z_AXIS_LIMIT_SW_PCINT_BM;
    LIMIT_SW_PCMSK &= ~R_AXIS_LIMIT_SW_PCINT_BM;
}
