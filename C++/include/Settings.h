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

#include <string>

#include <boost/property_tree/ptree.hpp>

/// The class that handles all settings
class Settings {
public:
    Settings(std::string path);
    virtual ~Settings();
    void Load(const std::string &filename);
    void Save(const std::string &filename);
    void RestoreAll();
    void Refresh();
    void Set(const std::string key, const std::string value);
    
    int GetInt2(const std::string key);
    std::string GetString2(const std::string key);
    double GetDouble2(const std::string key);
    bool GetBool2(const std::string key);
    
    static int GetInt(const char* name);
    static const char* GetString(const char* name);
    static double GetDouble(const char* name);
    void Save();
    
private:
    // don't allow construction without supplying file name
    Settings() {}
    boost::property_tree::ptree _settingsTree;
    std::string _settingsPath;
};

#endif	/* SETTINGS_H */

