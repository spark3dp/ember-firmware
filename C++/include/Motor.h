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
    bool TryJamRecovery(LayerType currentLayerType);
};

#endif	/* MOTOR_H */

