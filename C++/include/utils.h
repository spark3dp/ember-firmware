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

long GetMillis();
void StartStopwatch();
long StopStopwatch();
std::string GetFirmwareVersion();
std::string GetBoardSerialNum();
bool PurgeDirectory(std::string path);
bool Copy(std::string sourcePath, std::string providedDestinationPath);
int MakePath(std::string path);
int MkdirCheck(std::string path);

#endif	/* UTILS_H */

