#include "HardwareFactory.h"

#include "mock_hardware/Shared.h"
#include "mock_hardware/NamedPipeResource.h"
#include "mock_hardware/NamedPipeI2C_Device.h"
#include "mock_hardware/ImageWritingFrameBuffer.h"

I2C_DevicePtr HardwareFactory::CreateMotorControllerI2cDevice()
{
    return I2C_DevicePtr(new NamedPipeI2C_Device(
            MOTOR_CONTROLLER_I2C_READ_PIPE, MOTOR_CONTROLLER_I2C_WRITE_PIPE));
}

I2C_DevicePtr HardwareFactory::CreateFrontPanelI2cDevice()
{
    return I2C_DevicePtr(new NamedPipeI2C_Device(
            FRONT_PANEL_I2C_READ_PIPE, FRONT_PANEL_I2C_WRITE_PIPE));
}

//I2C_DevicePtr HardwareFactory::CreateProjectorI2cDevice()
//{
//    return I2C_DevicePtr(new NamedPipeI2C_Device(
//            PROJECTOR_I2C_READ_PIPE, PROJECTOR_I2C_WRITE_PIPE));
//}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new NamedPipeResource(
            MOTOR_CONTROLLER_INTERRUPT_READ_PIPE, 1));
}

ResourcePtr HardwareFactory::CreateFrontPanelInterruptResource() 
{
    return ResourcePtr(new NamedPipeResource(
            FRONT_PANEL_INTERRUPT_READ_PIPE, 1));
}

FrameBufferPtr HardwareFactory::CreateFrameBuffer()
{
    return FrameBufferPtr(new ImageWritingFrameBuffer(1280, 800,
            FRAME_BUFFER_IMAGE));
}