/*
 * Status.h
 * Author: Jason Lefley
 * Date  : 2015-05-29
 */

#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

#include "../../C++/include/MotorController.h"

#define RETURN_ON_ERROR(function)                       \
    do                                                  \
    {                                                   \
        Status status = function;                       \
        if (status != MC_STATUS_SUCCESS) return status; \
    }                                                   \
    while (0)

typedef uint8_t Status;

#endif /* STATUS_H */
