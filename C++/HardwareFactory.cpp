#include "HardwareFactory.h"

#include "Hardware.h"
#include "I2C_StreamBuffer.h"
#include "GPIO_Interrupt.h"

StreamBufferPtr HardwareFactory::CreateMotorStreamBuffer()
{
    return StreamBufferPtr(new I2C_StreamBuffer(MOTOR_SLAVE_ADDRESS, I2C2_PORT));
}

StreamBufferPtr HardwareFactory::CreateFrontPanelStreamBuffer()
{
}

StreamBufferPtr HardwareFactory::CreateProjectorStreamBuffer()
{
}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new GPIO_Interrupt(MOTOR_INTERRUPT_PIN,
            GPIO_INTERRUPT_EDGE_RISING));
}