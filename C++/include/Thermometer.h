/* 
 * File:   Thermometer.h
 * Author: Richard Greene
 * 
 * Measures and reports temperature.
 *
 * Created on October 22, 2014, 12:58 PM
 */

#include <string>

#ifndef THERMOMETER_H
#define	THERMOMETER_H

class Thermometer {
public:
    Thermometer(bool haveHardware);
    double GetTemperature();
    
private:
    Thermometer();  // need to specify if we have hardware in c'tor
    std::string _temperatureFile;
    double _temperature;
    static void* ThreadHelper(void *context);
    pthread_t _getTemperatureThread;
    void AwaitThreadComplete();
};

#endif	/* THERMOMETER_H */

