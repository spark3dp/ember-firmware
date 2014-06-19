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
    
    std::string GetJobName() { return _jobName; }
    int GetLayerThicknessMicrons() { return _layerThicknessMicrons;}
    double GetModelExposureTimeSec()  { return _modelExposureTimeSec; }
    bool GetIsRegistered() { return _isRegistered;}
    
    static int GetInt(const char* name);
    static const char* GetString(const char* name);
    static double GetDouble(const char* name);
    void Save();
    
private:
    // don't allow construction without supplying file name
    Settings() {}
    std::string _jobName;
    int _layerThicknessMicrons;
    double _modelExposureTimeSec;
    bool _isRegistered;
    boost::property_tree::ptree _settingsTree;
    std::string _settingsPath;
};

#endif	/* SETTINGS_H */

