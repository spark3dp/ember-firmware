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
char* CmdToUpper(char* cmd);
void StartStopwatch();
long StopStopwatch();
std::string Replace(std::string str, const char* oldVal, const char* newVal);
const char* GetFirmwareVersion();
const char* GetBoardSerialNum();
bool PurgeDirectory(std::string path);
bool Copy(std::string sourcePath, std::string providedDestinationPath);
int MakePath(const char* path);
int MkdirCheck(const char *path);

#endif	/* UTILS_H */

