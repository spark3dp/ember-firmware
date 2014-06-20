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

#define SETTINGS (PrinterSettings::Instance())

/// The class that handles configuration and print options
class Settings 
{
public:
    Settings(std::string path);
    virtual ~Settings();
    void Load(const std::string &filename);
    void Save(const std::string &filename);
    void Save();
    void RestoreAll();
    void Refresh();
    void Set(const std::string key, const std::string value);
    int GetInt(const std::string key);
    std::string GetString(const char* key);
    double GetDouble(const std::string key);
    bool GetBool(const std::string key);
    
protected:
    // don't allow construction without supplying file name
    Settings();
    boost::property_tree::ptree _settingsTree;
    std::string _settingsPath;
};

/// Singleton for settings shared by all components
class PrinterSettings : public Settings
{
public:
    static Settings& Instance();
    
private:
    PrinterSettings();
    PrinterSettings(std::string path);
    PrinterSettings(PrinterSettings const&);
    PrinterSettings& operator=(PrinterSettings const&);
    ~PrinterSettings();
};
#endif	/* SETTINGS_H */

