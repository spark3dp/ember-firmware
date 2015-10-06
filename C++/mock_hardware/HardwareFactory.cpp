#include "HardwareFactory.h"

#include "mock_hardware/Shared.h"
#include "mock_hardware/NamedPipeStreamBuffer.h"

StreamBufferPtr HardwareFactory::CreateMotorStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(MOTOR_I2C_READ_PIPE,
            MOTOR_I2C_WRITE_PIPE));
}

StreamBufferPtr HardwareFactory::CreateFrontPanelStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(FRONT_PANEL_I2C_READ_PIPE,
            FRONT_PANEL_I2C_WRITE_PIPE));
}

StreamBufferPtr HardwareFactory::CreateProjectorStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(PROJECTOR_I2C_READ_PIPE,
            PROJECTOR_I2C_WRITE_PIPE));
}
