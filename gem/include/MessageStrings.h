/* 
 * File:   MessageStrings.h
 * Author: greener
 *
 * Created on March 31, 2014, 4:57 PM
 */

#ifndef MESSAGESTRINGS_H
#define	MESSAGESTRINGS_H

#define I2C_FILE_OPEN_ERROR ("Couldn't open file in I2C_Device constructor")
#define I2C_SLAVE_ADDRESS_ERROR ("Couldn't set slave address in I2C_Device constructor")
#define I2C_WRITE_ERROR ("Error in I2C_Device::Write")
#define I2C_LONG_STRING_ERROR ("String too long for I2C_Device::Write")
#define I2C_READ_WRITE_ERROR ("Write error in I2C_Device::Read")
#define I2C_READ_READ_ERROR ("Read error in I2C_Device::Read")

#define GPIO_EXPORT_ERROR ("Unable to export GPIO pin for %d")
#define GPIO_DIRECTION_ERROR ("Unable to open direction handle for %d")
#define GPIO_EDGE_ERROR ("Unable to open edge handle for %d")
#define GPIO_INTERRUPT_ERROR ("Unable to create interrupt %d")

#define FILE_DESCRIPTOR_IN_USE_ERROR ("File descriptor for %d already defined")
#define NO_FILE_DESCRIPTOR_ERROR ("No file descriptor defined for subscription")
#define EPOLL_CREATE_ERROR ("Couldn't create the epoll set")
#define EPOLL_SETUP_ERROR ("Couldn't set up epoll for %d")
#define NEGATIVE_NUM_FDS_ERROR ("Negative number of file descriptors %d")
#define UNEXPECTED_EVENT_ERROR ("Unexpected event from event type %d")
#define UNEXPORT_ERROR ("Unable to open GPIO pin for unexport")

#define INVALID_INTERRUPT_ERROR ("Invalid interrupt event type %d")
#define UNKNOWN_EVENT_TYPE_ERROR ("Unknown event type %d")
#define PULSE_TIMER_CREATE_ERROR ("Unable to create pulse timer")
#define ENABLE_PULSE_TIMER_ERROR ("Unable to enable pulse timer")
#define DISABLE_PULSE_TIMER_ERROR ("Unable to disable pulse timer")
#define EXPOSURE_TIMER_CREATE_ERROR ("Unable to create exposure timer")
#define MOTOR_TIMER_CREATE_ERROR ("Unable to create motor timeout timer")
#define EXPOSURE_TIMER_ERROR ("Unable to set exposure timer")
#define STATUS_PIPE_CREATION_ERROR ("Error creating named pipe used for printer status")

#define MOTOR_TIMEOUT_ERROR ("Timeout waiting for motor response")


#endif	/* MESSAGESTRINGS_H */

