//  File:   TarGzFile.cpp
//  Utility for extracting gzipped tar files
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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

#include <libtar.h>
#include <zlib.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <cerrno>

#include <TarGzFile.h>

static int gzOpenFrontend(char* pathname, int oflags, int mode);

// Extracts the contents of the tar.gz file specified by archivePath into the 
// path specified by rootPath
bool TarGzFile::Extract(const std::string& archivePath, 
                        const std::string& rootPath)
{
    bool retVal = true;
    char archivePathBuf[archivePath.length()];
    char rootPathBuf[rootPath.length()];
    TAR* tar;
    tartype_t gzType = {
      (openfunc_t)  &gzOpenFrontend,
      (closefunc_t) gzclose,
      (readfunc_t)  gzread,
      (writefunc_t) gzwrite
    };
    
    std::strcpy(archivePathBuf, archivePath.c_str());
    std::strcpy(rootPathBuf, rootPath.c_str());
    
    if (tar_open(&tar, archivePathBuf, &gzType, O_RDONLY, 0, 0) == -1)
    {
        std::cerr << "could not get handle to archive" << std::endl;
        return false;
    }

    if (tar_extract_all(tar, rootPathBuf) != 0)
    {
        std::cerr << "could not extract archive" << std::endl;
        retVal = false;
    }
    
    if (tar_close(tar) != 0)
    {
        std::cerr << "could not close archive" << std::endl;
    }

    return retVal;
}

// Frontend for opening gzip files
// Taken from libtar.c (demo driver program for libtar)
int gzOpenFrontend(char* pathname, int oflags, int mode)
{
    char* gzoflags;
    gzFile gzf;
    int fd;
    
    switch (oflags & O_ACCMODE)
    {
    case O_WRONLY:
        gzoflags = const_cast<char*>("wb");
        break;
    case O_RDONLY:
        gzoflags = const_cast<char*>("rb");
        break;
    default:
    case O_RDWR:
        errno = EINVAL;
        return -1;
    }

    fd = open(pathname, oflags, mode);
    if (fd == -1)
    {
        std::cerr << "could not open archive with open" << std::endl;
        return -1;
    }

    if ((oflags & O_CREAT) && fchmod(fd, mode))
    {
        std::cerr << "could not do fchmod on archive fd"  << std::endl;
        return -1;
    }

    gzf = gzdopen(fd, gzoflags);
    if (!gzf)
    {
        std::cerr << "could not open archive with gzdopen" << std::endl;
        errno = ENOMEM;
        return -1;
    }

    return (intptr_t)gzf;
}
