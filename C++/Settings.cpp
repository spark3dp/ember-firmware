/* 
 * File:   Settings.cpp
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of Settings.
 *  
 * Created on June 4, 2014, 12:34 PM
 */

#include <string.h>
#include <libgen.h>
#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if(x);                                            \
  else throw std::exception();  

#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>

#include <Settings.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>

#define ROOT "Settings"

/// Constructor.
Settings::Settings(std::string path) :
_settingsPath(path),
_errorHandler(&LOGGER)
{  
    // define the default value for each of the settings
    _defaults = 
"{" 
"    \"" ROOT "\":"
"    {"
"        \"" JOB_NAME_SETTING "\": \"slice\","
"        \"" LAYER_THICKNESS "\": 25,"  
"        \"" FIRST_EXPOSURE "\": 5.0," 
"        \"" BURN_IN_LAYERS "\": 1,"            
"        \"" BURN_IN_EXPOSURE "\": 4.0,"
"        \"" MODEL_EXPOSURE "\": 2.5,"
"        \"" SEPARATION_RPM "\": 0,"            
"        \"" DOWNLOAD_DIR "\": \"" ROOT_DIR "/download\","
"        \"" STAGING_DIR "\": \"" ROOT_DIR "/staging\","
"        \"" PRINT_DATA_DIR "\": \"" ROOT_DIR "/print_data\","
"        \"" IS_REGISTERED "\": false,"
"        \"" R_HOMING_ACCEL "\": 50,"  
"        \"" R_HOMING_SPEED "\": 20,"  
"        \"" Z_HOMING_ACCEL "\": 100,"  
"        \"" Z_HOMING_SPEED "\": 20"                       
"    }"
"}";    
    
    // create the set of valid setting names
    _names.insert(JOB_NAME_SETTING);
    _names.insert(LAYER_THICKNESS);
    _names.insert(BURN_IN_LAYERS);
    _names.insert(FIRST_EXPOSURE);
    _names.insert(BURN_IN_EXPOSURE);
    _names.insert(MODEL_EXPOSURE);
    _names.insert(SEPARATION_RPM);
    _names.insert(IS_REGISTERED);
    _names.insert(PRINT_DATA_DIR);
    _names.insert(DOWNLOAD_DIR);
    _names.insert(STAGING_DIR);
    _names.insert(R_HOMING_ACCEL);
    _names.insert(R_HOMING_SPEED);
    _names.insert(Z_HOMING_ACCEL);
    _names.insert(Z_HOMING_SPEED);

    // Make sure the parent directory of the settings file exists
    EnsureSettingsDirectoryExists();

    if(!Load(path, true))
        RestoreAll();
}

/// Destructor.
Settings::~Settings() 
{
}

#define LOAD_BUF_LEN (1024)
/// Load all the Settings from a file
bool Settings::Load(const std::string &filename, bool ignoreErrors)
{
    bool retVal = false;
    try
    {
        FILE* pFile = fopen(filename.c_str(), "r");
        char buf[LOAD_BUF_LEN];
        FileReadStream frs1(pFile, buf, LOAD_BUF_LEN);
        // first parse into a temporary doc, for validation
        Document doc;
        doc.ParseStream(frs1);

        // make sure the file is valid
        RAPIDJSON_ASSERT(doc.IsObject() && doc.HasMember(ROOT))
                
        for (std::set<std::string>::iterator it = _names.begin(); 
                                             it != _names.end(); ++it)
        {
            RAPIDJSON_ASSERT(doc[ROOT].HasMember(it->c_str()))
        }
        
        // parse again, but now into _settingsDoc
        fseek(pFile, 0, SEEK_SET);
        FileReadStream frs2(pFile, buf, LOAD_BUF_LEN);
        _settingsDoc.ParseStream(frs2);
                    
        fclose(pFile);    
        retVal = true;
    }
    catch(std::exception)
    {
        if(!ignoreErrors)
        {
             _errorHandler->HandleError(CantLoadSettings, true, 
                                                             filename.c_str());
        }
    } 
    return retVal;
}
        
/// Load all the Settings from a string
bool Settings::LoadFromJSONString(const std::string &str)
{
    bool retVal = false;
    StringStream ss(str.c_str());
    
    try
    { 
        Document doc;
        doc.ParseStream(ss);
        const Value& root = doc[ROOT];
        
        // first validate each setting name from the given string,
        for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                        itr != root.MemberEnd(); ++itr)
        {
            const char* name = itr->name.GetString(); 
            if(!IsValidSettingName(name))
            {
                _errorHandler->HandleError(UnknownSetting, true, name);
                return false;
            }
        }
        
        // then set each value into the settings document
        for (Value::ConstMemberIterator itr = root.MemberBegin(); 
                                        itr != root.MemberEnd(); ++itr)
        {
        
            const char* name = itr->name.GetString();              
//////////////////////////////////////////////////////////////////////                
// TODO: remove this code when the portal no longer puts quotes around
// numeric values! 
            if(_settingsDoc[ROOT][name].IsNumber() && 
               doc[ROOT][name].IsString())
            {
                std::string s = doc[ROOT][name].GetString();
                std::string::size_type found = s.find_first_of("\"");
                if(found != std::string::npos)
                {
                    s.replace(found, 1, " ");
                    found = s.find_last_of("\"");
                    if(found != std::string::npos)
                        s.replace(found, 1, " ");
                }
                if(_settingsDoc[ROOT][name].IsInt())
                    _settingsDoc[ROOT][name] = atoi(s.c_str());
                else 
                    _settingsDoc[ROOT][name] = atof(s.c_str());
            }
            else
////////////////////////////////////////////////////////////////////// 
            if(_settingsDoc[ROOT][name].IsString())
            {
                // need to make a copy of the string to be stored
                const char* str = doc[ROOT][name].GetString();
                Value s;
                s.SetString(str, strlen(str), _settingsDoc.GetAllocator());
                _settingsDoc[ROOT][name] = s;
            }
            else
                _settingsDoc[ROOT][name] = doc[ROOT][name];
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


/// Save the current settings in the main settings file
void Settings::Save()
{
    Save(_settingsPath); 
}

/// Save the current settings in the given file
void Settings::Save(const std::string &filename)
{
    try
    {
        FILE* pFile = fopen (filename.c_str(), "w");
        FileStream fs(pFile);
        PrettyWriter<FileStream> writer(fs); 
        _settingsDoc.Accept(writer);     
        fclose(pFile);  
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSaveSettings, true, filename.c_str());
    }
}

/// Get all the settings as a single text string in JSON.
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

/// Restore all Settings to their default values
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

/// Restore a particular setting to its default value
void Settings::Restore(const std::string key)
{
    if(IsValidSettingName(key))
    {
        Document defaultsDoc;
        defaultsDoc.Parse(_defaults);
        
        _settingsDoc[ROOT][StringRef(key.c_str())] = 
                                    defaultsDoc[ROOT][StringRef(key.c_str())];
        Save();
    }
    else
    {
        _errorHandler->HandleError(NoDefaultSetting, true, key.c_str());
    }
}

/// Reload the settings from the settings file
void Settings::Refresh()
{
    Load(_settingsPath); 
}

/// Set  a new value for a saving but don't persist the change
void Settings::Set(const std::string key, const std::string value)
{
    try
    {
        if(IsValidSettingName(key))
        {
            // need to make a copy of the string to be stored
            Value s;
            s.SetString(value.c_str(), value.length(), _settingsDoc.GetAllocator());
            _settingsDoc[ROOT][StringRef(key.c_str())] =  s;           
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
        if(IsValidSettingName(key))
            _settingsDoc[ROOT][StringRef(key.c_str())] =  value;
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
        if(IsValidSettingName(key))
            _settingsDoc[ROOT][StringRef(key.c_str())] =  value;
        else
            _errorHandler->HandleError(UnknownSetting, true, key.c_str());
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSetSetting, true, key.c_str());
    }    
}

void Settings::Set(const std::string key, bool value)
{
    try
    {
        if(IsValidSettingName(key))
            _settingsDoc[ROOT][StringRef(key.c_str())] =  value;
        else
            _errorHandler->HandleError(UnknownSetting, true, key.c_str());
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantSetSetting, true, key.c_str());
    }  
}

/// Return the value of an integer setting.
int Settings::GetInt(const std::string key)
{
    int retVal = 0;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsDoc[ROOT][StringRef(key.c_str())].GetInt();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    }  
    return retVal;
}

/// Returns the value of a string setting.
std::string Settings::GetString(const std::string key)
{
    std::string retVal("");
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsDoc[ROOT][StringRef(key.c_str())].GetString();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    }  
    return retVal;
}

/// Returns the value of a double-precision floating point setting.
double Settings::GetDouble(const std::string key)
{
    double retVal = 0.0;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsDoc[ROOT][StringRef(key.c_str())].GetDouble();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    } 
    return retVal;
}

/// Returns the value of a boolean setting.
bool Settings::GetBool(const std::string key)
{
    bool retVal = false;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsDoc[ROOT][StringRef(key.c_str())].GetBool();
        else
           _errorHandler->HandleError(UnknownSetting, true, key.c_str()); 
    }
    catch(std::exception)
    {
        _errorHandler->HandleError(CantGetSetting, true, key.c_str());
    }  
    return retVal;
}

/// Validates that a setting name is one for which we have a default value.
bool Settings::IsValidSettingName(const std::string key)
{
    std::set<std::string>::iterator it = _names.find(key);
    return it != _names.end();
}

/// Ensure that the directory containing the file specified by _settingsPath exists
void Settings::EnsureSettingsDirectoryExists()
{
    char *path = strdup(_settingsPath.c_str());
    MakePath(dirname(path));
    free(path);
}

/// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);
    return settings;
}