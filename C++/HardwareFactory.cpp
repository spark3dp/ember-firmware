#include "HardwareFactory.h"

#include "Hardware.h"
#include "I2C_Device.h"
#include "GPIO_Interrupt.h"
#include "Settings.h"

I2C_DevicePtr HardwareFactory::CreateMotorControllerI2cDevice()
{
    return I2C_DevicePtr(new I2C_Device(MOTOR_SLAVE_ADDRESS, I2C2_PORT));
}

I2C_DevicePtr HardwareFactory::CreateFrontPanelI2cDevice()
{
    return I2C_DevicePtr(new I2C_Device(FP_SLAVE_ADDRESS,
            SETTINGS.GetInt(HARDWARE_REV) == 0 ? I2C2_PORT : I2C1_PORT));
}

//StreamBufferPtr HardwareFactory::CreateProjectorI2cDevice()
//{
//}

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