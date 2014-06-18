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

/// The class that defines an individual setting
class Setting {
public:
    Setting();
    virtual ~Setting();
    std::string Get();
    void Set(char* newValue);
    void Restore() { _value = _defaultValue; }

private:   
    std::string _name;
    std::string _value;
    std::string _defaultValue;
    std::string _dataType;
};


/// The class that handles all settings
class Settings {
public:
    Settings();
    virtual ~Settings();
    void Load(const std::string &filename);
    void Save(const std::string &filename);
    
    std::string GetJobName() { return _jobName; }
    void SetJobName(std::string jobName) { _jobName = jobName; }
    int GetLayerThicknessMicrons() { return _layerThicknessMicrons;}
    void SetLayerThicknessMicrons(int layerThickness) 
                                         { _layerThicknessMicrons = layerThickness; }
    double GetModelExposureTimeSec()  { return _modelExposureTimeSec; }
    void SetModelExposureTimeSec(double modelExposureTimeSec)
                             { _modelExposureTimeSec = modelExposureTimeSec; }
    bool GetIsRegistered() { return _isRegistered;}
    void SetIsRegistered(bool isRegistered)    { _isRegistered = isRegistered; }    
    
    static int GetInt(const char* name);
    static const char* GetString(const char* name);
    static double GetDouble(const char* name);
private:
    std::string _jobName;
    int _layerThicknessMicrons;
    double _modelExposureTimeSec;
    bool _isRegistered;
};

#endif	/* SETTINGS_H */

