//  File:   Projector.h
//  Encapsulates the functionality of the printer's projector
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
    bool ShowWhite();
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

#endif    // PROJECTOR_H

