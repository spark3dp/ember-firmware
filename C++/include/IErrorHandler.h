//  File:   IErrorHandler.h
//  Interface to class that handles errors.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#ifndef IERRORHANDLER_H
#define	IERRORHANDLER_H

#include <limits.h>

#include <ErrorMessage.h>

// ABC defining the interface to a class that handles errors.
class IErrorHandler
{
public:
    virtual bool HandleError(ErrorCode code, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX) = 0;
};

#endif    // IERRORHANDLER_H