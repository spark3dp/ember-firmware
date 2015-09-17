//  File:   PrinterStatusQueue.h
//  Communicates printer status updates between different parts of program
//  Exposed as a pollable resource
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef PRINTERSTATUSQUEUE_H
#define PRINTERSTATUSQUEUE_H

#include <queue>

#include "IResource.h"
#include "PrinterStatus.h"

class PrinterStatusQueue : public IResource
{
public:
    PrinterStatusQueue();
    ~PrinterStatusQueue();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    void Push(const PrinterStatus& printerStatus);
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    PrinterStatusQueue(const PrinterStatusQueue&);
    PrinterStatusQueue& operator=(const PrinterStatusQueue&);

private:
    int _fd;
    std::queue<PrinterStatus> _queue;
};

#endif    // PRINTERSTATUSQUEUE_H

