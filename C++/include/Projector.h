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

class I_I2C_Device;
class IFrameBuffer;
namespace Magick
{
class Image;
};

class Projector 
{
public:
    Projector(const I_I2C_Device& i2cDevice, IFrameBuffer& frameBuffer);
    virtual ~Projector();
    void SetImage(Magick::Image& image);
    void ShowCurrentImage();
    void ShowBlack();
    void ShowWhite();
    void ShowImageFromFile(const std::string& path);

private:
    void TurnLEDOn();
    void TurnLEDOff();
    
    bool _canControlViaI2C;
    const I_I2C_Device& _i2cDevice;
    IFrameBuffer& _frameBuffer;
};

#endif    // PROJECTOR_H

