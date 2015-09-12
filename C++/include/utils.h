//  File:   utils.h
//  Defines some utility methods
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

#ifndef UTILS_H
#define	UTILS_H

#define UUID_LEN    (36)  // characters in hex ASCII string for a UUID

long GetMillis();
void StartStopwatch();
long StopStopwatch();
std::string GetFirmwareVersion();
std::string GetBoardSerialNum();
std::string GetIPAddress();
bool PurgeDirectory(const std::string& path);
bool Copy(const std::string& sourcePath, 
          const std::string& providedDestinationPath);
int MakePath(const std::string& path);
int MkdirCheck(const std::string& path);
void GetUUID(char* uuid);
bool IsInternetConnected();
bool Mount(const std::string& deviceNode, const std::string& mountPoint,
        const std::string& filesystemType, unsigned long mountFlags = 0,
        const std::string& data = "");

#endif    // UTILS_H

