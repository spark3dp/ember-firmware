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
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();
    void WriteStatus(PrinterStatus* pPrinterStatus);

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    PrinterStatusPipe(const PrinterStatusPipe&);
    PrinterStatusPipe& operator=(const PrinterStatusPipe&);

private:
    int _readFd;
    int _writeFd;
    size_t _printerStatusSize;
};

#endif	/* PRINTERSTATUSPIPE_H */

