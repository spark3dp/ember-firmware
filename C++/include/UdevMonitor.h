/* 
 * File:   UdevMonitor.h
 * Author: Jason Lefley
 * 
 * Exposes udev events for a particular subsystem and device type as a pollable
 * resource.
 *
 * Created on August 18, 2015, 2:18 PM
 */

#ifndef UDEVMONITOR_H
#define	UDEVMONITOR_H

#include "IResource.h"

struct udev;

class UdevMonitor : public IResource
{
public:
    UdevMonitor(const std::string& subsystem, const std::string& deviceType);
    ~UdevMonitor();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    bool QualifyEvents(uint32_t events) const;
    ResourceBufferVec Read();

private:
    // this class owns a file based resource
    // disable copy construction and copy assignment
    UdevMonitor(const UdevMonitor&);
    UdevMonitor& operator=(const UdevMonitor&);

private:
    udev* _udev;
};

#endif	/* UDEVMONITOR_H */

