/* 
 * File:   PrinterStatusPipe.h
 * Author: Jason Lefley
 *
 * Created on August 12, 2015, 9:04 AM
 */

#ifndef PRINTERSTATUSPIPE_H
#define	PRINTERSTATUSPIPE_H

#include "IResource.h"

class PrinterStatus;

class PrinterStatusPipe : public IResource
{
public:
    PrinterStatusPipe();
    ~PrinterStatusPipe();
    uint32_t GetEventTypes();
    int GetFileDescriptor();
    ResourceBufferVec Read();
    void WriteStatus(PrinterStatus* pPrinterStatus);

private:
    int _readFd;
    int _writeFd;
    size_t _printerStatusSize;
};

#endif	/* PRINTERSTATUSPIPE_H */

