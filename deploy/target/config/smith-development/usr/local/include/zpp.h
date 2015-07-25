/*
**
** $id:$
**
** File: zpp.h -- main header file for ZPP library.
**
** uses zlib to do actual decompression (and eventually compression)
** Copyright (C) 1999 Michael Cuddy, Fen's Ende Software. All Rights Reserved
** modifications Copyright (C) 2000-2003 Eero Pajarre
**
** check http://zpp-library.sourceforge.net for latest version
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** Change Log
** ----------
** $Log: Zpp.h,v $
** Revision 1.3  2003/06/23 06:46:30  epajarre
** couple of option related bugs fixed
**
** Revision 1.2  2003/06/22 19:24:02  epajarre
** Integrated all changes by me, testing still to be done
**
** Revision 1.1.1.1  2003/06/21 19:09:38  epajarre
** Initial import. (0.2) version)
**
** Revision 1.2  1999/08/22 05:21:12  mcuddy
** Revision 1.1  1999/06/10 03:35:08  mcuddy
** Initial revision
** Revision 1.2  1999/06/10 03:30:41  mcuddy
** Alpha changes
** Revision 1.1  1999/05/25 05:38:23  mcuddy
** Initial revision
**
*/

#ifndef _H_ZPP_
#define _H_ZPP_

#ifndef __cplusplus
#error this is a C++ header file.
#endif /* __cplusplus */

#include <iostream>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <fstream>

//
// configuration: change these defines to compile different options.
//
#undef ZPP_INCLUDE_OPENALL		// if defined, includes support for zppZipArchive::openAll()
#undef ZPP_INCLUDE_CRYPT		// if defined, zip encryption support is added
#undef ZPP_USE_STDIO                    // if defined then library itself uses C stdio for reading files
#undef ZPP_IGNORE_DRIVE_LETTER         // remove x:/ from the start of a path
#undef ZPP_IGNORE_PLAIN_FILE           // do not read plain files (only look at archives)

#include "zpplib.h"
#include "zreader.h"
#include "izstream.h"

#endif /* _H_ZPP_ */
