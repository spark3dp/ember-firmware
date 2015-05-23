/* 
 * File:   LayerSettings.h
 * Author: Richard Greene
 * 
 * Handles overrides to settings on a per-layer basis, as defined in a CSV file.
 * 
 * Created on May 22, 2015, 2:28 PM
 */

#ifndef LAYERSETTINGS_H
#define	LAYERSETTINGS_H

#include <vector>
#include <map>
#include <string>

class LayerSettings {
public:
    LayerSettings();
    LayerSettings(const LayerSettings& orig);
    virtual ~LayerSettings();
    bool Load(std::string filename);
    int GetInt(int layer, std::string name);
    double GetDouble(int layer, std::string name);
    
private:
    std::map<std::string, int> _columns;
    std::map<int, std::vector<double> > _rows;
    void Clear();
    std::string Trim(std::string);
    double GetRawValue(int layer, std::string name);

};

#endif	/* LAYERSETTINGS_H */

