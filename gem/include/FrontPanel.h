/* 
 * File:   FrontPanel.h
 * Author: Richard Greene
 * 
 * Defines the front panel UI device, with its buttons and displays
 * 
 * Created on April 15, 2014, 12:43 PM
 */

#ifndef FRONTPANEL_H
#define	FRONTPANEL_H

#include <I2C_Device.h>

/// Defines a front panel as an I2C device 
class FrontPanel: public I2C_Device
{
public:
    FrontPanel(unsigned char slaveAddress);
    ~FrontPanel();
    
protected:
    // don't allow construction without specifying a slave address
    FrontPanel() {} 
};

#endif	/* FRONTPANEL_H */

