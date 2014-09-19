/* 
 * File:   Motor.h
 * Author: Richard Greene
 * Created on March 13, 2014, 5:51 PM
 */

#ifndef MOTOR_H
#define	MOTOR_H

#include <vector>

#include <I2C_Device.h>
#include <MotorCommands.h>

/// Defines a motor as an I2C device 
class Motor: public I2C_Device
{
public:
    Motor(unsigned char slaveAddress);
    ~Motor();
    bool SendCommand(MotorCommand command);
    bool SendCommands(std::vector<MotorCommand> commands);
    bool Reset();
    bool EnableMotors();
    bool DisableMotors();
    bool Pause();
    bool Resume();
    bool ClearCommandQueue();

    
protected:
    // don't allow construction without specifying a slave address
    Motor() {} 
};


#endif	/* MOTOR_H */

