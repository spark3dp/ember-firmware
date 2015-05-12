/*
 * CommandMap.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-02
 * Description: Provides mapping between commands and state machine event codes
 *              Maps are stored in flash to save space in RAM
 */


#include <avr/pgmspace.h>
#include <stdio.h>

#include "CommandMap.h"
#include "StateMachine.h"
#include "../../C++/include/MotorController.h"

const static uint8_t GeneralCommandMap[MC_GENERAL_HIGH_FENCEPOST] PROGMEM =
{
    MC_GENERAL_HIGH_FENCEPOST, // Map size
    InterruptRequested,        // MC_INTERRUPT
    ResetRequested,            // MC_RESET
    0, // MC_CLEAR
    0, // MC_PAUSE
    0, // MC_RESUME
};

const static uint8_t ZAxisActionCommandMap[MC_ACTION_HIGH_FENCEPOST] PROGMEM =
{
    MC_ACTION_HIGH_FENCEPOST,   // Map size
    MoveZAxisRequested,         // MC_MOVE
    HomeZAxisRequested,         // MC_HOME
    EnableZAxisMotorRequested,  // MC_ENABLE
    DisableZAxisMotorRequested, // MC_DISABLE
};

const static uint8_t RAxisActionCommandMap[MC_ACTION_HIGH_FENCEPOST] PROGMEM =
{
    MC_ACTION_HIGH_FENCEPOST,   // Map size
    MoveRAxisRequested,         // MC_MOVE
    HomeRAxisRequested,         // MC_HOME
    EnableRAxisMotorRequested,  // MC_ENABLE
    DisableRAxisMotorRequested, // MC_DISABLE
};

const static uint8_t* const RegisterMap[MC_REG_HIGH_FENCEPOST] PROGMEM =
{
    0,                       // Invalid
    GeneralCommandMap,       // MC_GENERAL_REG
    0, // MC_ROT_SETTINGS_REG
    RAxisActionCommandMap,   // MC_ROT_ACTION_REG
    0, // MC_Z_SETTINGS_REG
    ZAxisActionCommandMap    // MC_Z_ACTION_REG
};

uint8_t CommandMap::GetEventCode(uint8_t commandRegister, uint8_t commandAction)
{
    // Ensure that command register is within bounds
    if (commandRegister == 0 || commandRegister >= MC_REG_HIGH_FENCEPOST)
    {
#ifdef DEBUG
        printf_P(PSTR("ERROR: Command register %d outside of bounds\n"), commandRegister);
#endif
        return 0;
    }

    // Reading status does not involve a command action
    if (commandRegister == MC_STATUS)
        //TODO: change to request status event code
        return 0;

    if (commandRegister == MC_ROT_SETTINGS_REG)
        return SetRAxisSettingRequested;
    
    if (commandRegister == MC_Z_SETTINGS_REG)
        return SetZAxisSettingRequested;

    // Pointers are 16 bits (one word) long
    uint8_t* map = reinterpret_cast<uint8_t*>(pgm_read_word_near(&RegisterMap[commandRegister]));

    // Ensure that command action is below maximum for register
    if (commandAction == 0 || commandAction >= pgm_read_byte_near(&map[0]))
    {
#ifdef DEBUG
        printf_P(PSTR("ERROR: Command action %d (register %d) outside of bounds\n"), commandAction, commandRegister);
#endif
        return 0;
    }

    return pgm_read_byte_near(&map[commandAction]);
}

