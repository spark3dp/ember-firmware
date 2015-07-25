/*
**
** $id:$
**
** File: izstream.h -- a C++ iostream style interface to .ZIP files.
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
** $Log: izstream.h,v $
** Revision 1.2  2003/06/22 19:24:02  epajarre
** Integrated all changes by me, testing still to be done
**
** Revision 1.1.1.1  2003/06/21 19:09:37  epajarre
** Initial import. (0.2) version)
**
** Revision 1.2  1999/08/22 05:21:12  mcuddy
** Revision 1.1  1999/06/10 03:35:17  mcuddy
** Initial revision
** Revision 1.2  1999/06/10 03:30:44  mcuddy
** Alpha changes
** Revision 1.1  1999/05/25 05:38:38  mcuddy
** Initial revision
**
*/

#ifndef _H_IZSTREAM_
#define _H_IZSTREAM_

#ifndef __cplusplus
#error this is a C++ header file.
#endif /* __cplusplus */

#ifndef _H_ZPP_
#error must include zpp.h before __FILE__.
#endif /* !_H_ZPP_ */

class zppZipReader;
#ifdef _CRTIMP
#undef _CRTIMP // dll problems??
#endif
#define _CRTIMP

		// CLASS zppstreambuf
class zppstreambuf : public std::streambuf {
public:
	static int defaultBufferSize;

		// ctor: default constructor, no file specified.
	zppstreambuf(int _bufsize = -1, char *_buffer = 0)
		{ if (_Init(_bufsize, _buffer) == false)
			throw zppError("can't init buffer");
		}

		// ctor: open stream given a filename as a 'string'
	zppstreambuf(const std::string &_fn, bool _seekable = false, int _bufsize = -1, char *_buffer = 0)
		{ if (_Init(_bufsize, _buffer) == false)
			throw zppError("can't init buffer");
		  _Open(_fn, NULL, _seekable);
		}

		// ctor: open a stream from an explicit .zip file
	zppstreambuf(const std::string &_fn, zppZipArchive *_zip, bool _seekable = false, int _bufsize = -1, char *_buffer = 0)
		{ if (_Init(_bufsize, _buffer) == false)
			throw zppError("can't init buffer");
		  _Open(_fn, _zip, _seekable);
		}

		// ctor: open a stream from a c-string named file
	zppstreambuf(const char *_fn, bool _seekable =  false, int _bufsize = -1, char *_buffer = 0)
		{ if (_Init(_bufsize, _buffer) == false)
			throw zppError("can't init buffer");
	          _Open(std::string(_fn), NULL, _seekable);
		}

		// ctor: open a stream from a c-string named file in an explicit .zip file
	zppstreambuf(const char *_fn, zppZipArchive *_zip, bool _seekable = false, int _bufsize = -1, char *_buffer = 0)
		{ if (_Init(_bufsize, _buffer) == false)
			throw zppError("can't init buffer");
		  _Open(std::string(_fn), _zip, _seekable);
		}

	void setSeekable(bool _seekable) {
		if (zReader || zFile) throw zppError("can't set seekable on open buffer\n");
		// XXX -- unimplemented.
	}

	~zppstreambuf();

		// open a buffer from a named file
	bool open(const char *_fn, bool _seekable = false);
	bool open(const char *_fn, zppZipArchive *_zip, bool _seekable = false);
	bool open(std::string _fn, bool _seekable = false);
	bool open(std::string _fn, zppZipArchive *_zip, bool _seekable = false);

		// close the stream.
	bool close();

	bool is_open()const { return (zReader != NULL || zFile != NULL); };

protected:
		// virtual functions to be implemented to be a streambuf..
	_CRTIMP virtual int overflow(int = EOF);
	_CRTIMP virtual int underflow();
	_CRTIMP virtual std::streampos seekoff(std::streamoff, std::ios_base::seekdir,
		std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
	_CRTIMP virtual std::streampos seekpos(std::streampos,
		std::ios_base::openmode = std::ios_base::in | std::ios_base::out);

		// other funcs...
	_CRTIMP bool _Init(int _bufsize = -1, char *_buffer = NULL);
	_CRTIMP bool _Open(const std::string &_fn, zppZipArchive *_zip = NULL, bool _seekable = false);

	_CRTIMP void _Tidy();

private:
	zppZipReader *zReader;				// actual zip file reader object
	zppZipFileInfo *zFile;				// the file's info..
	bool ownZFile;						// if true, we own the zfile
	bool ownBuffer;						// if we own the buffer, we can delete it.
	char *buffer;						// our buffer
	int bufferSize;						// size of buffer
	int seekable;
};
		// CLASS izppstream

typedef std::istream std_istream;

class izppstream : public std_istream {

private:
	zppstreambuf _Sb;

public:
 explicit izppstream( ) : std_istream(&_Sb), _Sb() {};

	explicit izppstream(const char *_fn, bool _seekable = false, int _bufsize = -1, char *_buffer = 0)
	  : std_istream(&_Sb), _Sb(_bufsize,_buffer) {open(_fn,in,_seekable); };

	explicit izppstream(const std::string &_fn, bool _seekable = false, int _bufsize = -1, char *_buffer = 0)
	  : std_istream(&_Sb), _Sb(_bufsize,_buffer) {open(_fn,in,_seekable); };


	virtual ~izppstream() { };
	zppstreambuf *rdbuf() const
		{return ((zppstreambuf *)&_Sb); }

	bool is_open() const { return _Sb.is_open(); }

	void open(std::string _S, std::ios_base::openmode _M = in, bool _seekable = false)
		{if (_Sb.open(_S, _seekable) == false)
		 	setstate(failbit); }

	void open(std::string _S, zppZipArchive *_zip, std::ios_base::openmode _M = in, bool _seekable = false)
		{if (_Sb.open(_S, _zip, _seekable) == false)
		 	setstate(failbit); }

	void open(std::string _S, std::ios_base::open_mode _M, bool _seekable = false)
		{open(_S, _M, _seekable); }

	void open(const char *_S, std::ios_base::openmode _M = in, bool _seekable = false)
		{if (_Sb.open(std::string(_S), _seekable) == false)
		 	setstate(failbit); (void)_M;}

	void open(const char *_S, zppZipArchive *_zip, std::ios_base::openmode _M = in, bool _seekable = false)
		{if (_Sb.open(std::string(_S), _zip, _seekable) == false)
		 	setstate(failbit); (void)_M;}

	void open(const char *_S, std::ios_base::open_mode _M, bool _seekable = false)
		{open(std::string(_S), _M,_seekable); }

	void close()
		{if (_Sb.close() == 0)
		 	setstate(failbit); }

};
#undef _CRTIMP

#endif /* _H_IZSTREAM_ */
