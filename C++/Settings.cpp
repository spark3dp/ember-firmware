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
#include <Shared.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>
#include <PrintData.h>

/// Constructor.
Settings::Settings(std::string path) :
_settingsPath(path),
_errorHandler(&LOGGER)
{  
    // define the default value for each of the settings
    _defaults = 
"{" 
"    \"" SETTINGS_ROOT_KEY "\":"
"    {"
"        \"" JOB_NAME_SETTING "\": \"\","      
"        \"" JOB_ID_SETTING "\": \"\","      
"        \"" PRINT_FILE_SETTING "\": \"\","         
"        \"" LAYER_THICKNESS "\": 25,"  
"        \"" FIRST_EXPOSURE "\": 5.0," 
"        \"" BURN_IN_LAYERS "\": 1,"            
"        \"" BURN_IN_EXPOSURE "\": 4.0,"
"        \"" MODEL_EXPOSURE "\": 2.5,"
"        \"" DOWNLOAD_DIR "\": \"" ROOT_DIR "/download\","
"        \"" STAGING_DIR "\": \"" ROOT_DIR "/staging\","
"        \"" PRINT_DATA_DIR "\": \"" ROOT_DIR "/print_data\","  
"        \"" HARDWARE_REV "\": 1," 
                        
"        \"" FL_SEPARATION_R_JERK "\": 500000,"   
"        \"" FL_SEPARATION_R_SPEED "\": 6,"   
"        \"" FL_APPROACH_R_JERK "\": 500000," 
"        \"" FL_APPROACH_R_SPEED "\": 6," 
"        \"" FL_Z_LIFT "\": 2000,"   
"        \"" FL_SEPARATION_Z_JERK "\": 500000,"   
"        \"" FL_SEPARATION_Z_SPEED "\": 5000,"   
"        \"" FL_APPROACH_Z_JERK "\": 500000,"   
"        \"" FL_APPROACH_Z_SPEED "\": 5000,"   
"        \"" FL_ROTATION "\": 60000," 
"        \"" FL_EXPOSURE_WAIT "\": 0,"   
"        \"" FL_SEPARATION_WAIT "\": 0,"   
"        \"" FL_APPROACH_WAIT "\": 0,"   
            
"        \"" BI_SEPARATION_R_JERK "\": 500000,"   
"        \"" BI_SEPARATION_R_SPEED "\": 11,"   
"        \"" BI_APPROACH_R_JERK "\": 500000," 
"        \"" BI_APPROACH_R_SPEED "\": 11," 
"        \"" BI_Z_LIFT "\": 2000,"   
"        \"" BI_SEPARATION_Z_JERK "\": 500000,"   
"        \"" BI_SEPARATION_Z_SPEED "\": 5000,"   
"        \"" BI_APPROACH_Z_JERK "\": 500000,"   
"        \"" BI_APPROACH_Z_SPEED "\": 5000,"   
"        \"" BI_ROTATION "\": 60000," 
"        \"" BI_EXPOSURE_WAIT "\": 0,"   
"        \"" BI_SEPARATION_WAIT "\": 0,"   
"        \"" BI_APPROACH_WAIT "\": 0,"   
            
"        \"" ML_SEPARATION_R_JERK "\": 500000,"   
"        \"" ML_SEPARATION_R_SPEED "\": 12,"   
"        \"" ML_APPROACH_R_JERK "\": 500000," 
"        \"" ML_APPROACH_R_SPEED "\": 12," 
"        \"" ML_Z_LIFT "\": 2000,"   
"        \"" ML_SEPARATION_Z_JERK "\": 500000,"   
"        \"" ML_SEPARATION_Z_SPEED "\": 5000,"   
"        \"" ML_APPROACH_Z_JERK "\": 500000,"   
"        \"" ML_APPROACH_Z_SPEED "\": 5000,"   
"        \"" ML_ROTATION "\": 60000," 
"        \"" ML_EXPOSURE_WAIT "\": 0,"   
"        \"" ML_SEPARATION_WAIT "\": 0,"   
"        \"" ML_APPROACH_WAIT "\": 0," 
            
"        \"" LAYER_OVERHEAD "\": 0.410,"  
"        \"" MAX_TEMPERATURE "\": 80.0,"  
                        
"        \"" INSPECTION_HEIGHT "\": 60000,"  
"        \"" MAX_Z_TRAVEL "\": 160000," 
"        \"" DETECT_JAMS "\": 1," 
"        \"" MAX_UNJAM_TRIES "\": 5," 
            
"        \"" Z_STEP_ANGLE "\": 1800,"       
"        \"" Z_MICRONS_PER_REV "\": 2000," // 2 mm lead screw pitch at 1:1      
"        \"" Z_MICRO_STEP "\": 4,"  
            
"        \"" R_STEP_ANGLE "\": 1800,"       
"        \"" R_MILLIDEGREES_PER_REV "\": 180000,"   // 2:1 gear ratio    
"        \"" R_MICRO_STEP "\": 4,"       
            
"        \"" Z_HOMING_JERK "\": 1000000,"  
"        \"" Z_HOMING_SPEED "\": 5000,"    
"        \"" R_HOMING_JERK "\": 3000000,"     
"        \"" R_HOMING_SPEED "\": 20,"     
"        \"" R_HOMING_ANGLE "\": -60000,"     
            
"        \"" Z_START_PRINT_JERK "\": 1000000,"  
"        \"" Z_START_PRINT_SPEED "\": 5000,"    
"        \"" Z_START_PRINT_POSITION "\": -165000,"    
"        \"" R_START_PRINT_JERK "\": 3000000,"     
"        \"" R_START_PRINT_SPEED "\": 20,"  
"        \"" R_START_PRINT_ANGLE "\": 60000"     
                              
"    }"
"}";    
    
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

    if(!Load(path, true))
    {
        RestoreAll();
        // clear any print data, since it probably doesn't use default settings,
        // but don't call any code that uses Settings!
        PurgeDirectory(root[PRINT_DATA_DIR].GetString());
    }
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
        RAPIDJSON_ASSERT(doc.IsObject() && doc.HasMember(SETTINGS_ROOT_KEY))
                
        // create a default doc, to check that all the expected setting names 
        // are present and have the correct type
        // (we may not yet have a valid _settingsDoc)
        Document defaultDoc;
        defaultDoc.Parse(_defaults);                
                
        for (std::set<std::string>::iterator it = _names.begin(); 
                                             it != _names.end(); ++it)
        {
            RAPIDJSON_ASSERT(doc[SETTINGS_ROOT_KEY].HasMember(it->c_str()))
             
           if(!AreSameType(defaultDoc[SETTINGS_ROOT_KEY][it->c_str()],
                                  doc[SETTINGS_ROOT_KEY][it->c_str()]))
            {
                _errorHandler->HandleError(WrongTypeForSetting, true, 
                                                                  it->c_str());
                return false;                
            }           
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
        
/// Load settings (not necessarily all of them) from a string
bool Settings::LoadFromJSONString(const std::string &str)
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
            if(!IsValidSettingName(name))
            {
                _errorHandler->HandleError(UnknownSetting, true, name);
                return false;
            }
            
            if(!AreSameType(_settingsDoc[SETTINGS_ROOT_KEY][name],
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
            if(_settingsDoc[SETTINGS_ROOT_KEY][name].IsString())
            {
                // need to make a copy of the string to be stored
                const char* str = doc[SETTINGS_ROOT_KEY][name].GetString();
                Value s;
                s.SetString(str, strlen(str), _settingsDoc.GetAllocator());
                _settingsDoc[SETTINGS_ROOT_KEY][name] = s;
            }
            else
                _settingsDoc[SETTINGS_ROOT_KEY][name] = doc[SETTINGS_ROOT_KEY][name];
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
        // call fsync to ensure critical data is written to the storage device
        fsync(fileno(pFile));
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
        
        _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] = 
                                    defaultsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())];
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
        if(IsValidSettingName(key))
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
        if(IsValidSettingName(key))
            _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())] =  value;
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
            retVal = _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetInt();
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
            retVal = _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetString();
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
            retVal = _settingsDoc[SETTINGS_ROOT_KEY][StringRef(key.c_str())].GetDouble();
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

bool Settings::AreSameType(Value& a, Value& b)
{
    if(a.IsInt() && b.IsInt())
        return true;
    else if(a.IsDouble() && b.IsDouble())
        return true;
    else 
        return(a.IsString() && b.IsString());
}

/// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);
    return settings;
}