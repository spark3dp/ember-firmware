#include "HardwareFactory.h"

#include "Hardware.h"
#include "Settings.h"
#include "I2C_StreamBuffer.h"

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
    return StreamBufferPtr(new I2C_StreamBuffer(PROJECTOR_SLAVE_ADDRESS, I2C0_PORT));
}
