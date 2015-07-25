/*
**
** $id:$
**
** File: zreader.h -- a C++ iostream style interface to .ZIP files.
** uses zlib to do actual decompression (and eventually compression)
** (This header file is private, and shouldn't need to be included
** by client code)
**
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
** $Log: zreader.h,v $
** Revision 1.2  2003/06/22 19:24:02  epajarre
** Integrated all changes by me, testing still to be done
**
** Revision 1.1.1.1  2003/06/21 19:09:40  epajarre
** Initial import. (0.2) version)
**
** Revision 1.2  1999/08/22 05:21:22  mcuddy
** Revision 1.1  1999/06/10 03:36:14  mcuddy
** Initial revision
** Revision 1.2  1999/06/10 03:30:45  mcuddy
** Alpha changes
** Revision 1.1  1999/05/25 05:38:46  mcuddy
** Initial revision
**
*/

#ifndef _H_ZREADER_
#define _H_ZREADER_

#ifndef __cplusplus
#error this is a C++ header file.
#endif /* __cplusplus */

typedef struct z_stream_s z_stream;	// forward declare

//
// the zppZipReader object is used to manage reading of a file from
// a zip archive. the istream wrapper uses this object.
//
class zppZipReader {
	int readBufSize;		// size of read buffer
	char *readBuf;			// pointer to read buffer
	bool ourBuffer;			// if TRUE, we delete the buffer on destruction

	long curPos;			// current seek position in decompressed stream
	long uncmpLeft;			// uncompressed bytes left to retrieve
	long cmpLeft;			// compressed bytes left to retrieve
	bool streamInit;
	zppZipFileInfo *file;	// ptr to file structure we're decompressing.
	z_stream *stream;		// stream structure for zlib

#ifdef ZPP_INCLUDE_CRYPT
	bool isCrypt;			// if is encrypted..
	zppCryptState crypt;
#endif /* ZPP_INCLUDE_CRYPT */

		// by default, this is the size of the read buffer allocated for
		// the reader object.
	static int defaultReadBufSize;

	static void* zlibAlloc(void *opaque, unsigned int items, unsigned int size);
	static void zlibFree(void *opaque, void *address);

		// "close" stream
	void tidy();

public:
	void setDefaultReadBufSize(int b) { defaultReadBufSize = b; }
	long getDefaultReadBufSize() { return defaultReadBufSize; }

		// ctor: open a file contained in a zip archive for reading
		// if readBufSize == 0 && readBuffer == NULL, a default buffer size
		// will be allocated.
		// if readBuffer != NULL, then readBufSize specifies how much space is
		// available in the readbuffer.
		//
		// will create encryption state from parent, if needed.
		// if file is encrypted, and crypt state cannot be established
		// (i.e.: invalid password) this code will throw().
		//
	zppZipReader(zppZipFileInfo *info, int rbSize = 0, char *rdBuf = NULL);

		// dtor
	~zppZipReader();

		// resets stream to beginning of file.
	void resetStream();

		// main workhorse read function, return # of bytes
		// decompressed. (-1 on eof)
	long read(char *buf, long len);

};

#endif /* !_H_ZREADER_ */
