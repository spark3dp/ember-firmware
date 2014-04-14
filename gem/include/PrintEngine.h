/* 
 * File:   PrintEngine.h
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

#include <PrinterStatus.h>

/// We will always need one and only one PrintEngine, 
/// so it is defined as a singleton
class PrintEngine
{
public: 
    static PrintEngine& Instance()
    { 
        static PrintEngine _theOneAndOnly;
        return _theOneAndOnly;
    }

    void SendStatus(const char* stateName);
    void SetNumLayers(int numLayers);
    int NextLayer();
    bool NoMoreLayers();
    void EnablePulseTimer(bool enable);
    
private:
    PrinterStatus _status;
    int _pulseTimerFD;
    int _pulsePeriodSec;

    // Disallow construction, copying, or assignment 
    PrintEngine();
    PrintEngine(PrintEngine const&);
    PrintEngine& operator=(PrintEngine const&);
}; 

#endif	/* PRINTENGINE_H */

