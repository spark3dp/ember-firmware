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
#define GPIO_INPUT_ERROR ("Unable to open input for %d")

#define FILE_DESCRIPTOR_IN_USE_ERROR ("File descriptor for %d already defined")
#define NO_FILE_DESCRIPTOR_ERROR ("No file descriptor defined for subscription to event type %d")
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
#define COMMAND_PIPE_CREATION_ERROR ("Error creating named pipe used for command input")
#define WEB_COMMAND_PIPE_CREATION_ERROR ("Error creating named pipe used for web command input")
#define STATUS_TO_WEB_PIPE_CREATION_ERROR ("Error creating named pipe for reporting status to web")
#define UNKNOWN_TEXT_COMMAND_ERROR ("Unknown command text: '%s'")
#define STATUS_JSON_OPEN_ERROR ("Can't open file for saving latest printer status in JSON")

#define MOTOR_TIMEOUT_ERROR ("Timeout waiting for motor response")
#define FRONT_PANEL_ERROR ("Front panel error")
#define MOTOR_ERROR ("Motor error")
#define UNEXPECTED_MOTION_END ("End of motion when none expected")
#define UNKNOWN_MOTOR_EVENT ("Unknown pending motor event: %d")
#define UNKNOWN_MOTOR_STATUS ("Unknown motor board status: %d")
#define UNKNOWN_FRONT_PANEL_STATUS ("Unknown front panel status: %d")
#define HARDWARE_NEEDED_ERROR ("Release build must have hardware!")
#define UNKNOWN_COMMAND_INPUT ("Unknown command input: %d")
#define REMAINING_EXPOSURE_ERROR ("Error reading remaining exposure time")
#define SDL_INIT_ERROR ("Could not initialize screen, SDL error: %s")
#define LOAD_IMAGE_ERROR ("Error loading image: %s")
#define NO_IMAGE_FOR_LAYER ("No image for layer %d")

#define PRINTER_STATUS_FORMAT (", layer %d of %d, seconds left: %d")
#define LOG_STATUS_FORMAT ("entering %s")
#define LOG_ERROR_FORMAT ("%s: %s\n")
#define STDERR_FORMAT (": ")
#define LOG_MOTOR_EVENT ("motor interrupt: %d")
#define LOG_BUTTON_EVENT ("button interrupt: %d")
#define LOG_DOOR_EVENT ("door interrupt: %c")
#define LOG_KEYBOARD_INPUT ("keyboard input: %s")
#define LOG_UI_COMMAND ("UI command: %d")
#define LOG_WEB_COMMAND ("web command: %s")


#endif	/* MESSAGESTRINGS_H */

