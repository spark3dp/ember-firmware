/* 
 * File:   GPIO_Interrupt.h
 * Author: Jason Lefley
 *
 * Created on August 13, 2015, 10:59 AM
 */

#ifndef GPIO_INTERRUPT_H
#define	GPIO_INTERRUPT_H

#include "IResource.h"

class GPIO_Interrupt : public IResource
{
public:
    GPIO_Interrupt(int inputPin, const std::string& edge);
    ~GPIO_Interrupt();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();
    void UnExport() const;
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    GPIO_Interrupt(const GPIO_Interrupt&);
    GPIO_Interrupt& operator=(const GPIO_Interrupt&);

private:
    int _fd;
    int _pin;
    uint32_t _events;
};

#endif	/* GPIO_INTERRUPT_H */

