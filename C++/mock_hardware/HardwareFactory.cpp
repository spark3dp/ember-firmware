#include "HardwareFactory.h"

#include "mock_hardware/Shared.h"
#include "mock_hardware/NamedPipeStreamBuffer.h"
#include "mock_hardware/NamedPipeResource.h"

StreamBufferPtr HardwareFactory::CreateMotorStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(
            MOTOR_CONTROLLER_I2C_READ_PIPE, MOTOR_CONTROLLER_I2C_WRITE_PIPE));
}

StreamBufferPtr HardwareFactory::CreateFrontPanelStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(
            FRONT_PANEL_I2C_READ_PIPE, FRONT_PANEL_I2C_WRITE_PIPE));
}

StreamBufferPtr HardwareFactory::CreateProjectorStreamBuffer()
{
    return StreamBufferPtr(new NamedPipeStreamBuffer(
            PROJECTOR_I2C_READ_PIPE, PROJECTOR_I2C_WRITE_PIPE));
}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new NamedPipeResource(
            MOTOR_CONTROLLER_INTERRUPT_READ_PIPE, 1));
}
