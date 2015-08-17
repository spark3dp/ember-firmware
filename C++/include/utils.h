/* 
 * File:   utils.h
 * Author: Richard Greene
 * 
 * Some handy utilities
 *
 * Created on April 17, 2014, 3:16 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#define UUID_LEN    (36)  // characters in hex ASCII string for a UUID

long GetMillis();
void StartStopwatch();
long StopStopwatch();
std::string GetFirmwareVersion();
std::string GetBoardSerialNum();
std::string GetIPAddress();
bool PurgeDirectory(std::string path);
bool Copy(std::string sourcePath, std::string providedDestinationPath);
int MakePath(std::string path);
int MkdirCheck(std::string path);
void GetUUID(char* uuid);
bool IsInternetConnected();

#endif	/* UTILS_H */

