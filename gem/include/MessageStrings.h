/* 
 * File:   MessageStrings.h
 * Author: greener
 *
 * Created on March 31, 2014, 4:57 PM
 */

#ifndef MESSAGESTRINGS_H
#define	MESSAGESTRINGS_H

#define I2C_FILE_OPEN_ERROR ("couldn't open file in I2C_Device constructor")
#define I2C_SLAVE_ADDRESS_ERROR ("couldn't set slave address in I2C_Device constructor")
#define I2C_WRITE_ERROR ("error in I2C_Device::Write")
#define I2C_LONG_STRING_ERROR ("string too long for I2C_Device::Write")
#define I2C_READ_WRITE_ERROR ("write error in I2C_Device::Read")
#define I2C_READ_READ_ERROR ("read error in I2C_Device::Read")

#define GPIO_EXPORT_ERROR ("Unable to export GPIO pin for %d")
#define GPIO_DIRECTION_ERROR ("Unable to open direction handle for %d")
#define GPIO_EDGE_ERROR ("Unable to open edge handle for %d")
#define GPIO_INTERRUPT_ERROR ("Unable to create interrupt %d")

#define FILE_DESCRIPTOR_IN_USE_ERROR ("File descriptor for %d already defined")
#define EPOLL_CREATE_ERROR ("couldn't create the epoll set")
#define EPOLL_SETUP_ERROR ("couldn't set up epoll for %d")
#define NEGATIVE_NUM_FDS_ERROR ("negative number of file descriptors %d")
#define UNEXPECTED_EVENT_ERROR ("unexpected event from event type %d")
#define UNEXPORT_ERROR ("unable to open GPIO pin for unexport")

#define INVALID_INTERRUPT_ERROR ("Invalid interrupt event type %d")

#endif	/* MESSAGESTRINGS_H */

