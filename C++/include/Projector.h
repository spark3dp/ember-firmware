/* 
 * File:   Projector.h
 * Author: Richard Greene
 * 
 * Encapsulates the functionality of the printer's projector.
 *
 * Created on May 30, 2014, 3:45 PM
 */

#ifndef PROJECTOR_H
#define	PROJECTOR_H

#include <SDL/SDL.h>
#include <I2C_Device.h>

class Projector : public I2C_Device 
{
public:
    Projector(unsigned char slaveAddress, int port);
    virtual ~Projector();
    void SetImage(SDL_Surface* image);
    bool ShowImage();
    bool ShowBlack();
    void TearDown();
    void ShowTestPattern();
    void ShowCalibrationPattern();
    void ScaleImage(SDL_Surface* image, double scale);

private:
    bool _canControlViaI2C;
    SDL_Surface* _screen;
    SDL_Surface* _image ;
    void TurnLED(bool on);
};

#endif	/* PROJECTOR_H */

