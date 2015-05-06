/* 
 * File:   Motor.h
 * Author: Richard Greene
 * 
 * Interfaces with a motor controller via I2C
 * 
 * Created on March 13, 2014, 5:51 PM
 */

#ifndef MOTOR_H
#define	MOTOR_H

#include <vector>

#include <I2C_Device.h>
#include <MotorCommand.h>
#include <PrinterStatus.h>

#define UNITS_PER_REVOLUTION (360 * 1000)   // milli-degrees in a full circle
// The motor speed settings are defined in units of RPM and microns/s.
// Multiplying by these conversion factors will convert 
// RPM to degrees/1000/minute and microns/s to microns/minute.
#define R_SPEED_FACTOR (UNITS_PER_REVOLUTION)
#define Z_SPEED_FACTOR (60)

/// Defines a motor as an I2C device 
class Motor: public I2C_Device
{
public:
    Motor(unsigned char slaveAddress);
    ~Motor();
    bool SendCommand(MotorCommand command);
    bool SendCommands(std::vector<MotorCommand> commands);
    bool Initialize();
    bool EnableMotors();
    bool DisableMotors();
    bool Pause();
    bool Resume();
    bool ClearPendingCommands();
    bool GoHome(bool withInterrupt = true);
    bool GoToStartPosition();
    bool GoToNextLayer(LayerType currentLayerType);
    bool PauseAndInspect(int rotation);
    bool ResumeFromInspect(int rotation);
    bool TryJamRecovery();
};

#endif	/* MOTOR_H */

