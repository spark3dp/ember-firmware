/* 
 * File:   Settings.h
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of settings.
 * 
 * Created on June 4, 2014, 12:34 PM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

class Settings {
public:
    Settings();
    virtual ~Settings();
    int GetIntSetting(char* name);
    char* GetStringSetting(char* name);
    double GetFloatSetting(char* name);
private:

};

#endif	/* SETTINGS_H */

