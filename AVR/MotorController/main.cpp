#include <avr/interrupt.h>

extern "C" {
#include "i2c.h"
}

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

/*
 * this function will run when a master somewhere else on the bus
 * addresses us and wishes to write data to us
 */

void i2cSlaveReceiveService(uint8_t receiveDataLength, uint8_t* receiveData)
{
    for(uint8_t i = 0; i < receiveDataLength; i++)
        commandBuffer.AddByte(*receiveData++);
}

int main()
{
    // Turn on LED
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PB0);

    // Disable interrupts
    cli();

#ifdef DEBUG
    DebugInitialize();
#endif

    // Initialize I2C bus
    i2cInit();

    // Configure I2C address
    i2cSetLocalDeviceAddr(I2C_ADDRESS, TRUE);

    // Set I2C send/receive handlers
    i2cSetSlaveReceiveHandler(i2cSlaveReceiveService);
    //i2cSetSlaveTransmitHandler(i2cSlaveTransmitService);

    // Initialize I/O and subsystems
    MotorController::Initialize();

    // Enable interrupts
    sei();

#ifdef DEBUG
    printf_P(PSTR("INFO: Motor controller firmware initialized\n"));
#endif


    // Configure pin change interrupts for limit switches
    // Create an instance of the global state struct
    MotorController_t mc;

    // Initialize the state machine to the Ready state
    MotorController_State_Machine_Init(&mc, Ready);

    Command command;

    for(;;)
    {
        if (limitSwitchHit)
        {
            limitSwitchHit = 0;
            MotorController_State_Machine_Event(&mc, &command, AxisLimitReached);
#ifdef DEBUG
            printf_P(PSTR("INFO: Current state: %d\n"), mc.sm_state);
#endif
        }

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
                continue;
            }

            MotorController_State_Machine_Event(&mc, &command, eventCode);

#ifdef DEBUG
            printf_P(PSTR("INFO: Current state: %d\n"), mc.sm_state);
#endif
        }
    }
}

ISR (PCINT2_vect)
{
    limitSwitchHit = 1;

    // Disable interrupts immediately to avoid spurious interrupts
    LIMIT_SW_PCMSK &= ~Z_AXIS_LIMIT_SW_PCINT_BM;
    LIMIT_SW_PCMSK &= ~R_AXIS_LIMIT_SW_PCINT_BM;
}
