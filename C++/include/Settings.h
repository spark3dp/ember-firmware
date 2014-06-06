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
    static int GetInt(const char* name);
    static const char* GetString(const char* name);
    static double GetDouble(const char* name);
private:

};

#endif	/* SETTINGS_H */

