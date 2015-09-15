//  File:   Settings.cpp
//  Handles storage, retrieval, and reset of settings
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#include <string.h>
#include <libgen.h>
#include <exception>
#include <sstream>
#include <fstream>
#include <vector>

#define RAPIDJSON_ASSERT(x)                         \
  if (x);                                            \
  else throw std::exception();  

#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>

#include <Settings.h>
#include <Shared.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>

// The default values of all settings are defined here.
// Printer settings are common to all prints.
// Z_MICRONS_PER_REV == 2000 (2 mm lead screw pitch at 1:1  gear ratio)
// R_MILLIDEGREES_PER_REV == 180000 (2:1 gear ratio for rotation)
#define PRINTER_SETTINGS    \
"        \"" DOWNLOAD_DIR "\": \"" ROOT_DIR "/download\","      \
"        \"" STAGING_DIR "\": \"" ROOT_DIR "/staging\","        \
"        \"" PRINT_DATA_DIR "\": \"" ROOT_DIR "/print_data\","  \
"        \"" HARDWARE_REV "\": 1,"                              \
"        \"" LAYER_OVERHEAD "\": 0.660,"                        \
"        \"" MAX_TEMPERATURE "\": 80.0,"                        \
"        \"" INSPECTION_HEIGHT "\": 60000,"                     \
"        \"" MAX_Z_TRAVEL "\": 160000,"                         \
"        \"" DETECT_JAMS "\": 1,"                               \
"        \"" MAX_UNJAM_TRIES "\": 5,"                           \
"        \"" MOTOR_TIMEOUT_FACTOR "\": 1.1,"                    \
"        \"" MIN_MOTOR_TIMEOUT_SEC "\": 15.0,"                  \
"        \"" PROJECTOR_LED_CURRENT "\": -1,"                    \
                                                                \
"        \"" MICRO_STEPS_MODE "\": 6,"                          \
"        \"" Z_STEP_ANGLE "\": 1800,"                           \
"        \"" Z_MICRONS_PER_REV "\": 2000,"                      \
                                                                \
"        \"" R_STEP_ANGLE "\": 1800,"                           \
"        \"" R_MILLIDEGREES_PER_REV "\": 180000,"               \
                                                                \
"        \"" Z_HOMING_JERK "\": 500000,"                        \
"        \"" Z_HOMING_SPEED "\": 5000,"                         \
"        \"" R_HOMING_JERK "\": 100000,"                        \
"        \"" R_HOMING_SPEED "\": 5,"                            \
"        \"" R_HOMING_ANGLE "\": -60000,"                       \
                                                                \
"        \"" Z_START_PRINT_JERK "\": 100000,"                   \
"        \"" Z_START_PRINT_SPEED "\": 5000,"                    \
"        \"" Z_START_PRINT_POSITION "\": -165000,"              \
"        \"" R_START_PRINT_JERK "\": 100000,"                   \
"        \"" R_START_PRINT_SPEED "\": 5,"                       \
"        \"" R_START_PRINT_ANGLE "\": 60000,"                   \
"        \"" FRONT_PANEL_AWAKE_TIME "\": 30,"                   \
"        \"" IMAGE_SCALE_FACTOR "\": 1.0,"                      \
"        \"" USB_DRIVE_DATA_DIR "\": \"/EmberUSB\""  

// Print settings are specific to a print, rather than the printer as a whole
#define PRINT_SPECIFIC_SETTINGS \
"        \"" JOB_NAME_SETTING "\": \"\","           \
"        \"" JOB_ID_SETTING "\": \"\","             \
"        \"" PRINT_FILE_SETTING "\": \"\","         \
                                                    \
"        \"" LAYER_THICKNESS "\": 25,"              \
"        \"" FIRST_EXPOSURE "\": 5.0,"              \
"        \"" BURN_IN_LAYERS "\": 1,"                \
"        \"" BURN_IN_EXPOSURE "\": 4.0,"            \
"        \"" MODEL_EXPOSURE "\": 2.5,"              \
                                                    \
"        \"" HOME_ON_APPROACH "\": 0,"              \
                                                    \
"        \"" FL_SEPARATION_R_JERK "\": 100000,"     \
"        \"" FL_SEPARATION_R_SPEED "\": 6,"         \
"        \"" FL_APPROACH_R_JERK "\": 100000,"       \
"        \"" FL_APPROACH_R_SPEED "\": 6,"           \
"        \"" FL_Z_LIFT "\": 2000,"                  \
"        \"" FL_SEPARATION_Z_JERK "\": 100000,"     \
"        \"" FL_SEPARATION_Z_SPEED "\": 5000,"      \
"        \"" FL_APPROACH_Z_JERK "\": 100000,"       \
"        \"" FL_APPROACH_Z_SPEED "\": 5000,"        \
"        \"" FL_ROTATION "\": 60000,"               \
"        \"" FL_EXPOSURE_WAIT "\": 0,"              \
"        \"" FL_SEPARATION_WAIT "\": 0,"            \
"        \"" FL_APPROACH_WAIT "\": 0,"              \
"        \"" FL_PRESS "\": 0,"                      \
"        \"" FL_PRESS_SPEED "\": 5000,"             \
"        \"" FL_PRESS_WAIT "\": 0,"                 \
"        \"" FL_UNPRESS_SPEED "\": 5000,"           \
                                                    \
"        \"" BI_SEPARATION_R_JERK "\": 100000,"     \
"        \"" BI_SEPARATION_R_SPEED "\": 11,"        \
"        \"" BI_APPROACH_R_JERK "\": 100000,"       \
"        \"" BI_APPROACH_R_SPEED "\": 11,"          \
"        \"" BI_Z_LIFT "\": 2000,"                  \
"        \"" BI_SEPARATION_Z_JERK "\": 100000,"     \
"        \"" BI_SEPARATION_Z_SPEED "\": 5000,"      \
"        \"" BI_APPROACH_Z_JERK "\": 100000,"       \
"        \"" BI_APPROACH_Z_SPEED "\": 5000,"        \
"        \"" BI_ROTATION "\": 60000,"               \
"        \"" BI_EXPOSURE_WAIT "\": 0,"              \
"        \"" BI_SEPARATION_WAIT "\": 0,"            \
"        \"" BI_APPROACH_WAIT "\": 0,"              \
"        \"" BI_PRESS "\": 0,"                      \
"        \"" BI_PRESS_SPEED "\": 5000,"             \
"        \"" BI_PRESS_WAIT "\": 0,"                 \
"        \"" BI_UNPRESS_SPEED "\": 5000,"           \
                                                    \
"        \"" ML_SEPARATION_R_JERK "\": 100000,"     \
"        \"" ML_SEPARATION_R_SPEED "\": 12,"        \
"        \"" ML_APPROACH_R_JERK "\": 100000,"       \
"        \"" ML_APPROACH_R_SPEED "\": 12,"          \
"        \"" ML_Z_LIFT "\": 2000,"                  \
"        \"" ML_SEPARATION_Z_JERK "\": 100000,"     \
"        \"" ML_SEPARATION_Z_SPEED "\": 5000,"      \
"        \"" ML_APPROACH_Z_JERK "\": 100000,"       \
"        \"" ML_APPROACH_Z_SPEED "\": 5000,"        \
"        \"" ML_ROTATION "\": 60000,"               \
"        \"" ML_EXPOSURE_WAIT "\": 0,"              \
"        \"" ML_SEPARATION_WAIT "\": 0,"            \
"        \"" ML_APPROACH_WAIT "\": 0,"              \
"        \"" ML_PRESS "\": 0,"                      \
"        \"" ML_PRESS_SPEED "\": 5000,"             \
"        \"" ML_PRESS_WAIT "\": 0,"                 \
"        \"" ML_UNPRESS_SPEED "\": 5000"           

#define SETTINGS_JSON_PREFIX "{ \"" SETTINGS_ROOT_KEY "\": {"
#define SETTINGS_JSON_SUFFIX "}}"

// Constructor.
Settings::Settings(std::string path) :
_settingsPath(path),
_errorHandler(&LOGGER)
{  
    // define the default value for each of the settings
    _defaults = SETTINGS_JSON_PREFIX
                PRINTER_SETTINGS ","
                PRINT_SPECIFIC_SETTINGS                              
                SETTINGS_JSON_SUFFIX;  
    
    // create the set of valid setting names
    Document doc;
    doc.Parse(_defaults);
    const Value& root = doc[SETTINGS_ROOT_KEY];
    for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                    itr != root.MemberEnd(); ++itr)
    {
        _names.insert(itr->name.GetString()); 
    }    

    // Make sure the parent directory of the settings file exists
    EnsureSettingsDirectoryExists();

    if (!Load(path, true))
    {
        RestoreAll();
        // clear any print data, since it probably doesn't use default settings,
        // but don't call any code that uses Settings!
        PurgeDirectory(root[PRINT_DATA_DIR].GetString());
    }
}

// Destructor.
Settings::~Settings() 
{
}

#define LOAD_BUF_LEN (1024)
// Load all the Settings from a file.  If 'initializing' is true, then any 
// corrupted or missing settings are given their default values.  In that way,
// when new settings are added in new versions of the firmware, any values for 
// existing settings will not be lost.
bool Settings::Load(const std::string &filename, bool initializing)
{
    bool retVal = false;
    std::vector<std::string> missing;
    try
    {
        FILE* pFile = fopen(filename.c_str(), "r");
        char buf[LOAD_BUF_LEN];
        FileReadStream frs1(pFile, buf, LOAD_BUF_LEN);
        // first parse into a temporary doc, for validation
        Document doc;
        doc.ParseStream(frs1);

        // make sure the file is valid
        RAPIDJSON_ASSERT(doc.IsObject() && doc.HasMember(SETTINGS_ROOT_KEY))
                
        // create a default doc, to check that all the expected setting names 
        // are present and have the correct type
        // (we may not yet have a valid _settingsDoc)
        Document defaultDoc;
        defaultDoc.Parse(_defaults);                
                
        for (std::set<std::string>::iterator it = _names.begin(); 
                                             it != _names.end(); ++it)
        {
            if (doc[SETTINGS_ROOT_KEY].HasMember(it->c_str())) 
            {
                if (!AreSameType(defaultDoc[SETTINGS_ROOT_KEY][it->c_str()],
                                       doc[SETTINGS_ROOT_KEY][it->c_str()]))
                {
                    _errorHandler->HandleError(WrongTypeForSetting, true, 
                                                                   it->c_str());
                    return false;                
                }           
            }
            else
            {
                if (initializing) // record the missing member to be added
                    missing.push_back(*it);
                else
                    throw std::exception(); 
            }
        }
        
        // parse again, but now into _settingsDoc
        fseek(pFile, 0, SEEK_SET);
        FileReadStream frs2(pFile, buf, LOAD_BUF_LEN);
        _settingsDoc.ParseStream(frs2);
        fclose(pFile);  
        
        if (initializing && missing.size() > 0)
        {
            // add any missing settings, with their default values
            for (std::vector<std::string>::iterator it = missing.begin(); 
                                                    it != missing.end(); ++it)
            {
                _settingsDoc[SETTINGS_ROOT_KEY].AddMember(StringRef(it->c_str()), 
                        defaultDoc[SETTINGS_ROOT_KEY][StringRef(it->c_str())], 
                        _settingsDoc.GetAllocator());
            }
            Save();
        }              
        retVal = true;
    }
    catch(std::exception)
    {
        // if we're initializing, we'll handle this by simply regenerating
        // the settings file from scratch
        if (!initializing)
            _errorHandler->HandleError(CantLoadSettings, true, 
                                                            filename.c_str());
    } 
    return retVal;
}
        
// Parse specified string as JSON and set any settings contained in the string 
// to their specified values
bool Settings::SetFromJSONString(const std::string &str)
{
    bool retVal = false;
    StringStream ss(str.c_str());
    
    try
    { 
        Document doc;
        doc.ParseStream(ss);
        const Value& root = doc[SETTINGS_ROOT_KEY];
        
        // first validate the name & type of each setting from the given string
        for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                        itr != root.MemberEnd(); ++itr)
        {
            const char* name = itr->name.GetString(); 
            if (!IsValidSettingName(name))
            {
                _errorHandler->HandleError(UnknownSetting, true, name);
                return false;
            }
            
            if (!AreSameType(_settingsDoc[SETTINGS_ROOT_KEY][name],
                                     doc[SETTINGS_ROOT_KEY][name]))
            {

                _errorHandler->HandleError(WrongTypeForSetting, true, name);
                return false;                
            }
        }
        
        // then set each value into the settings document
        for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                        itr != root.MemberEnd(); ++itr)
        {
            const char* name = itr->name.GetString();              
            if (_settingsDoc[SETTINGS_ROOT_KEY][name].IsString())
            {
                // need to make a copy of the string to be stored
                const char* str = doc[SETTINGS_ROOT_KEY][name].GetString();
                Value s;
                s.SetString(str, strlen(str), _settingsDoc.GetAllocator());
                _settingsDoc[SETTINGS_ROOT_KEY][name] = s;
            }
            else
                _settingsDoc[SETTINGS_ROOT_KEY][name] = 
                                                doc[SETTINGS_ROOT_KEY][name];
        }
        Save();
        retVal = true;
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantReadSettingsString, true, str.c_str());
    }
    return retVal;
}

// Parse contents of specified file as JSON and set any settings contained in 
// the JSON to their specified values
bool Settings::SetFromFile(const std::string& filename)
{
    std::stringstream buffer;
    std::ifstream settingsFile(filename.c_str());
   
    // check if file exists
    if (!settingsFile.is_open()) 
        return false;    
    
    buffer << settingsFile.rdbuf();
    
    return SetFromJSONString(buffer.str());
}

// Save the current settings in the main settings file
void Settings::Save()
{
    Save(_settingsPath); 
}

// Save the current settings in the given file
void Settings::Save(const std::string &filename)
{
    try
    {
        FILE* pFile = fopen (filename.c_str(), "w");
        FileStream fs(pFile);
        PrettyWriter<FileStream> writer(fs); 
        _settingsDoc.Accept(writer);     
        // call fsync to ensure critical data is written to the storage device
        fsync(fileno(pFile));
        fclose(pFile);
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSaveSettings, true, filename.c_str());
    }
}

// Get all the settings as a single text string in JSON.
std::string Settings::GetAllSettingsAsJSONString()
{
    StringBuffer buffer; 
    try
    {
        Writer<StringBuffer> writer(buffer);
        _settingsDoc.Accept(writer);        
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantWriteSettingsString);
    }
    return buffer.GetString();
}

// Restore all Settings to their default values
void Settings::RestoreAll()
{
    try
    {
        _settingsDoc.Parse(_defaults); 

        Save();     
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantRestoreSettings, true,   
                                                         _settingsPath.c_str());
    }
}

// Restore a particular setting to its default value
void Settings::Restore(const std::string key)
{
    if (IsValidSettingName(key))
    {
        Document defaultsDoc;
        defaultsDoc.Parse(_defaults);
        
        _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] = 
                         defaultsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())];
        Save();
    }
    else
    {
        _errorHandler->HandleError(NoDefaultSetting, true, key.c_str());
    }
}


// Restore all the settings used for a print (as opposed to printer settings)
// to their default values.
bool Settings::RestoreAllPrintSettings()
{
    try
    {
        Document defaultsDoc;
        defaultsDoc.Parse(SETTINGS_JSON_PREFIX 
                          PRINT_SPECIFIC_SETTINGS 
                          SETTINGS_JSON_SUFFIX);
        
        // for each key in default print settings
        const Value& root = defaultsDoc[SETTINGS_ROOT_KEY];
        for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                        itr != root.MemberEnd(); ++itr)
        {
            const char* key = itr->name.GetString(); 

            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key)] = 
                         defaultsDoc[SETTINGS_ROOT_KEY][StringRef(key)];
        }
        Save();
        return true;
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantRestorePrintSettings, true,   
                                                         _settingsPath.c_str());
        return false;
    }
}

// Reload the settings from the settings file
void Settings::Refresh()
{
    Load(_settingsPath); 
}

// Set  a new value for a saving but don't persist the change
void Settings::Set(const std::string key, const std::string value)
{
    try
    {
        if (IsValidSettingName(key))
        {
            // need to make a copy of the string to be stored
            Value s;
            s.SetString(value.c_str(), value.length(), 
                                                _settingsDoc.GetAllocator());
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] =  s;           
        }
        else
            _errorHandler->HandleError(UnknownSetting, true, key.c_str());
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSetSetting, true, key.c_str());
    }  
}

void Settings::Set(const std::string key, int value)
{
    try
    {
        if (IsValidSettingName(key))
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] =  value;
        else
            _errorHandler->HandleError(UnknownSetting, true, key.c_str());
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSetSetting, true, key.c_str());
    }    
}

void Settings::Set(const std::string key, double value)
{
    try
    {
        if (IsValidSettingName(key))
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] =  value;
        else
            _errorHandler->HandleError(UnknownSetting, true, key.c_str());
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSetSetting, true, key.c_str());
    }    
}

// Return the value of an integer setting.
int Settings::GetInt(const std::string key)
{
    int retVal = 0;
    try
    {
        if (IsValidSettingName(key))
            retVal = 
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetInt();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    }  
    return retVal;
}

// Returns the value of a string setting.
std::string Settings::GetString(const std::string key)
{
    std::string retVal("");
    try
    {
        if (IsValidSettingName(key))
            retVal = 
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetString();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    }  
    return retVal;
}

// Returns the value of a double-precision floating point setting.
double Settings::GetDouble(const std::string key)
{
    double retVal = 0.0;
    try
    {
        if (IsValidSettingName(key))
            retVal = 
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetDouble();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    } 
    return retVal;
}

// Validates that a setting name is one for which we have a default value.
bool Settings::IsValidSettingName(const std::string key)
{
    std::set<std::string>::iterator it = _names.find(key);
    return it != _names.end();
}

// Ensure that the directory containing the file specified by _settingsPath 
// exists
void Settings::EnsureSettingsDirectoryExists()
{
    char *path = strdup(_settingsPath.c_str());
    MakePath(dirname(path));
    free(path);
}

// Test that a given setting is of the expected type.
bool Settings::AreSameType(Value& expected, Value& actual)
{
    if (expected.IsInt() && actual.IsInt())
        return true;
    
    // accept integers where we expect a double, to facilitate javascript 
    // clients that remove the decimal point from doubles of integral value
    if (expected.IsDouble() && (actual.IsDouble() || actual.IsInt()))
        return true;
    
    return(expected.IsString() && actual.IsString());
}

// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);
    return settings;
}