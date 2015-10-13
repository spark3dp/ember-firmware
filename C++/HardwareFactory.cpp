#include "HardwareFactory.h"

#include "Hardware.h"
#include "I2C_StreamBuffer.h"
#include "GPIO_Interrupt.h"
#include "Settings.h"

StreamBufferPtr HardwareFactory::CreateMotorStreamBuffer()
{
    return StreamBufferPtr(new I2C_StreamBuffer(MOTOR_SLAVE_ADDRESS, I2C2_PORT));
}

StreamBufferPtr HardwareFactory::CreateFrontPanelStreamBuffer()
{
    return StreamBufferPtr(new I2C_StreamBuffer(FP_SLAVE_ADDRESS,
            SETTINGS.GetInt(HARDWARE_REV) == 0 ? I2C2_PORT : I2C1_PORT));
}

StreamBufferPtr HardwareFactory::CreateProjectorStreamBuffer()
{
}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new GPIO_Interrupt(MOTOR_INTERRUPT_PIN,
            GPIO_INTERRUPT_EDGE_RISING));
}

ResourcePtr HardwareFactory::CreateFrontPanelInterruptResource()
{
    return ResourcePtr(new GPIO_Interrupt(FP_INTERRUPT_PIN,
            GPIO_INTERRUPT_EDGE_RISING));
}