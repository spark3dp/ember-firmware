/* 
 * File:   TarGzFile.h
 * Author: Jason Lefley
 * 
 * Utility for extracting gzipped tar files
 *
 * Created on July 16, 2015, 3:44 PM
 */

#ifndef TARGZFILE_H
#define	TARGZFILE_H

namespace TarGzFile
{
    bool Extract(const std::string& archivePath, const std::string& rootPath);
}

#endif	/* TARGZFILE_H */

