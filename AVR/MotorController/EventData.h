/*
 * EventData.h
 * Author     : Jason Lefley
 * Date       : 2015-05-09
 * Description: Data structure to hold event data
 */

#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <stdint.h>

struct EventData
{
    uint8_t command;   // The command associated with the event
    int32_t parameter; // The command parameter associated with the event
};

#endif /* EVENTDATA_H */
