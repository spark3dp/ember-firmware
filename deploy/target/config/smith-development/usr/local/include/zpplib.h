/*
**
** $id:$
**
** File: zpplib.h -- a C++ iostream style interface to .ZIP files.
** uses zlib to do actual decompression (and eventually compression)
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
** $Log: Zpplib.h,v $
** Revision 1.4  2003/06/22 19:51:21  epajarre
** couple of problems with ZPP_USE_STDIO fixed
**
** Revision 1.3  2003/06/22 19:30:03  epajarre
** fixed #if vs #ifdef issue
**
** Revision 1.2  2003/06/22 19:24:02  epajarre
** Integrated all changes by me, testing still to be done
**
** Revision 1.1.1.1  2003/06/21 19:09:40  epajarre
** Initial import. (0.2) version)
**
** Revision 1.1  1999/08/22 05:19:40  mcuddy
** Initial revision
**
*/

#ifndef _H_ZPPLIB_
#define _H_ZPPLIB_

#ifndef __cplusplus
#error this is a C++ header file.
#endif /* __cplusplus */

#ifdef ZPP_USE_STDIO
#include  <assert.h>
#endif
#ifdef _MSC_VER
// Must make sure that structures declared here are byte aligned.
#pragma pack(push, 1)
#endif /* _MSC_VER */

#define ZPP_LOCAL_FILE_MAGIC (0x04034b50)
//
// this structure, (which must be byte aligned!) represents the exact
// on-disk structure of the .ZIP file's local-file-header.
//
struct __attribute__ ((__packed__)) zppLocalFileHeader {
	unsigned long magicNumber;	// local file header sig: 0x04034b50
    unsigned short version;	// version needed to extract
    unsigned short bitFlag;	// general purpose bit flags
    unsigned short method;	// compression method
    unsigned short modTime;	// last mod file time
    unsigned short modDate;	// last mod file date
    unsigned long crc32;	// crc32 checksum
    unsigned long cmpSize;	// compressed size
    unsigned long realSize;	// uncompressed size
    unsigned short fnLength;	// length of filename
    unsigned short extraLength;	// length of extra data
};

//
// This descriptor exists only if bit 3 of the general
// purpose bit flag is set (see below).  It is byte aligned
// and immediately follows the last byte of compressed data.
// This descriptor is used only when it was not possible to
// seek in the output zip file, e.g., when the output zip file
// was standard output or a non seekable device.
//
#define ZPP_DATA_DESCRIPTOR_MAGIC (0x08074B50)
struct __attribute__ ((__packed__)) zppDataDescriptor {
    unsigned long magicNumber;	// data descriptor signature (0x08074b50) 
    unsigned long crc32;	// crc32 checksum
    unsigned long cmpSize;	// compressed size
    unsigned long realSize;	// real size
};

//
//
//  Central directory structure
//
#define ZPP_CENTRAL_FILE_HEADER_MAGIC (0x02014b50)
struct __attribute__ ((__packed__)) zppCentralDirFileHeader {
    unsigned long magicNumber;	// central file header signature (0x02014b50)
    unsigned short versMadeBy;	// version made by
    unsigned short version;	// version needed to extract
    unsigned short bitFlag;	// general purpose bit flag
    unsigned short method;	// compression method
    unsigned short modTime; 	// last mod file time
    unsigned short modDate; 	// last mod file date
    unsigned long crc32;	// crc-32 checksum
    unsigned long cmpSize;	// compressed size
    unsigned long realSize;	// uncompressed size
    unsigned short fnLength;	// filename length
    unsigned short extraLength;	// extra field length
    unsigned short commentLength; // file comment length
    unsigned short diskNum;	// disk number start
    unsigned short intAttrib;	// internal file attributes
    unsigned long extAttrib;	// external file attributes
    unsigned long lclHdrOffset;	// relative offset of local header
};

//
// end-of-central-directory record
//
#define ZPP_END_OF_CENTRAL_DIR_MAGIC (0x06054b50)
struct __attribute__ ((__packed__)) zppEndOfCentralDirHeader {
    unsigned long magicNumber;		// end of central dir signature    4 bytes  (0x06054b50)
    unsigned short diskNum;		// number of this disk             2 bytes
    unsigned short dirDiskNum;		// number of the disk with start of central dir
    unsigned short entryCntThisDisk;	// total number of entries in the central dir on this disk
    unsigned short entryCntTotal;	// total number of entries in the central dir
    unsigned long dirSize;		// size of the central directory
    unsigned long dirOffset;		// offset of start of central directory with respect to the starting disk number
    unsigned short commentLength;	// zipfile comment length
};

#ifdef _MSC_VER
// Must make sure that structures declared here are byte aligned.
#pragma pack(pop)
#endif /* _MSC_VER */


// these codes go in the upper byte of the versMadeBy field.
// it really determines the interpretation of the extAttrib field.
enum zppZipVersions {
    ZPP_FATFS=0,	// dos, os/2, NT
    ZPP_AMIGA=1,	// amiga (duh)
    ZPP_VMS=2,		// VMS (vax or Alpha AXP)
    ZPP_UNIX=3,
    ZPP_VMCMS=4,
    ZPP_ATARI=5,
    ZPP_HPFS=6,
    ZPP_MACINTOSH=7,
    ZPP_ZSYSTEM=8,
    ZPP_CPM=9,
    ZPP_TOPS20=10,
    ZPP_NTFS=11,
    ZPP_SMSQDOS=12,
    ZPP_ACORN=13,
    ZPP_VFAT=14,	// VFAT (Win95, NT)
    ZPP_MVS=15,
    ZPP_BEOS=16,
    ZPP_TANDEM=17,
    ZPP_FIRSTUNKNOWNVERSION=18,
};

enum zppComprMethod {
    ZPP_STORED=0,	// no compression
    ZPP_SHRUNK=1,
    ZPP_REDUCE1=2,	// reduced with compression factor 1
    ZPP_REDUCE2=3,	// reduced with compression factor 2
    ZPP_REDUCE3=4,	// reduced with compression factor 3
    ZPP_REDUCE4=5,	// reduced with compression factor 4
    ZPP_IMPLODE=6,
    ZPP_TOKENIZED=7,
    ZPP_DEFLATED=8,
    ZPP_ENHDEFLATED=9,
    ZPP_PKWARELIB=10,
    ZPP_FIRSTUNKNOWNCOMPRMETHOD=11,
};

//
// placeholder exception class; should be replaced with something more
// robust; this just stores a string which can be printed when the
// exception is caught.
//
class zppError {
public:
	std::string str;
	zppError(const char *s) : str(s) { };
	zppError(std::string &s) : str(s) { };
};

// foward declare main zip file class.
class zppZipArchive;

#ifdef ZPP_INCLUDE_CRYPT

class zppCryptState {
	unsigned long key0, key1, key2;
	unsigned long initKey0, initKey1, initKey2;

	void updateKeys(unsigned char x); 

	// returns a pseudo-random value, in the decryption
	// sequence.
	unsigned char decryptByte() {
		unsigned short temp;
		temp = key2 | 2;
		return (temp * (temp ^ 1)) >> 8;
	}

	void _setHeader(unsigned char *hdr)
	{
		decrypt(hdr,12);
	}

public:
	// passwd is the zip archive password.
	void initKeys(char *passwd) {
		key0 = 0x12345678;
		key1 = 0x23456789;
		key2 = 0x34567890;
		while (*passwd) {
			updateKeys(*passwd++);
		}
		initKey0 = key0; initKey1 = key1; initKey2 = key2;
	}

	// reset keys to start decryption of another file
	void resetKeys() {
		key0 = initKey0; key1 = initKey1; key2 = initKey2;
	}

	// resets the encryption keys, and primes the
	// decrption engine with the encryption header. (12 bytes)
	void setHeader(unsigned char *hdr)
	{
		resetKeys();
		_setHeader(hdr);
	}

	// decrypt one byte.
	unsigned char decrypt(unsigned char c) {
		unsigned char tmp;
		tmp = c ^ decryptByte();
		updateKeys(tmp);
		return tmp;
	}

	// decrypt a buffer, in place.
	void decrypt(unsigned char *c, int size) {
		while (size > 0) {
			*c = decrypt(*c);
			c++; size--;
		}
	}

	// create a crypt-state object
	explicit zppCryptState() {
		key0 = key1 = key2 = 0;
		initKey0 = initKey1 = initKey2 = 0;
	}
};
#endif /* ZPP_INCLUDE_CRYPT */

//
// each file in a .zip archive gets one of these stuffed into a vector
// inside the zppZipArchive structure.  this structure is deliberately kept
// small. we store just the information that we need to extract the file
// at a later date.
//
class zppZipFileInfo {
protected:
	long centralDirOff;		// off of this file's central dir entry
	long centralDirSize;	// size of this file's central dir entry.
	long lclHdrOff;			// offset to local file header
	long dataOffset;		// offset of this file's data stream
	long realSize, cmpSize;	// real and compressed data sizes
#ifdef ZPP_USE_STDIO
        FILE *fp;				// if opened as a file, underlying file stream
#else
        std::ifstream *fp;				// if opened as a file, underlying file stream
#endif
	zppZipArchive *parentZip;	// zip containing this file.
    std::string fileName;			// filename read from file

#ifdef ZPP_INCLUDE_CRYPT
	bool isEncrypted;		// TRUE == file is encrypted
#endif /* ZPP_INCLUDE_CRYPT */

	zppComprMethod method;

private:
	bool _InitFromFile(const std::string &_fileName);

public:
    	// ctor -- reads from an input stream
		// the "FILE" pointer is not saved, but information about offsets
		// into the file are.
		//
	zppZipFileInfo(long cdoff, zppZipArchive *parent);

		// info structure from an on-disk file
	explicit zppZipFileInfo(const char *file);

		// info structure from an on-disk file
	explicit zppZipFileInfo(const std::string &file);
	
		// default constructor
	zppZipFileInfo() :
		centralDirOff(-1),
		centralDirSize(-1),
		lclHdrOff(-1),
		dataOffset(-1),
		realSize(-1),
		cmpSize(-1),
		fp(0),
		parentZip(0)
#ifdef ZPP_INCLUDE_CRYPT
		,isEncrypted(false)
#endif /* ZPP_INCLUDE_CRYPT */
		{ };

		// dtor
	~zppZipFileInfo();

		// handy operators for storage in an STL container
		// NOTE: this does NOT take priority into account ... XXX: should it?
	bool operator == (zppZipFileInfo &z) {
		return z.fileName == fileName;
	}
	bool operator < (zppZipFileInfo &z) {
		return z.fileName < fileName;
	}

		// return compression method
	zppComprMethod getMethod() { return method; }

		// return file priority (from parent)
	inline int getPriority();

		// seek to a (compressed) offset of data in parent zip
		// and read some bytes from there.
	inline bool rawRead(long _P, char *_S, std::streamsize _N);

		// return parent zip archive -- may return null if this
		// file isn't in a ZIP archive.
	inline zppZipArchive *getParentZip() { return parentZip; }

		// return file name
	const std::string &getName() { return fileName; };

		// return offset of data inside zip archive; will return 0
		// if not a member of a zip archive.
	long getDataOffset() { return dataOffset; };
		// return uncompressed data size
	long getSize() { return realSize; }
		// return compressed data size
	long getCmpSize() { return cmpSize; }

#ifdef ZPP_INCLUDE_CRYPT
	int isCrypt() { return isEncrypted; }
#endif /* ZPP_INCLUDE_CRYPT */

protected:
		// seek to, and read local file header, return in 'hdr'.
		// throws a zppError on failure.
	void getLclHeader(zppLocalFileHeader *hdr);

		// seek to, and read central dir header.  return in 'hdr'.
		// throws a zppError on failure.
	void getCentralHeader(zppCentralDirFileHeader *hdr);

		// return offset of central directory entry for this file.
	int cdOffset() { return centralDirOff; };

	friend class zppZipArchive;
		// return central directory entry size
	int cdSize() { return centralDirSize; };

};

typedef std::map<std::string,zppZipFileInfo> zppFileMap;

typedef std::map<std::string,std::string> zppStrStrMap;

#ifdef ZPP_INCLUDE_OPENALL
// we use a list of ziparchive*'s to keep track of all zip's opened
// with zppZipArchive::openAll.
typedef std::list<zppZipArchive*> zppZipArchiveList;
#endif /* ZPP_INCLUDE_OPENALL */

//
// this class encapsulates all of the information in a .ZIP file.
// there is a singleton list which contains all of the .ZIP files that
// have been opened (with _makeGlobal == TRUE), that can be searched by
// the static function zppZipArchive::find();
//

class zppZipArchive {

public:

#ifdef ZPP_INCLUDE_OPENALL
		// these functions open all .ZIP files that they can find in
		// a specific directory.
	static int openAll(const std::string &_path);
	static int openAll(char *_path);

		// call this, if you've called openAll()
	static void closeAll();
#endif /* ZPP_INCLUDE_OPENALL */

		// ctor: input is filename to open.
		// only ios::in is supported currently.
		// if _makeGlobal == TRUE, then the ZipFile is added to the list
		// of global zip files.
	zppZipArchive(const std::string &_fn, std::ios_base::openmode _mode = std::ios_base::in, bool _makeGlobal = true);

		// ctor: input is iostream
		// (implication: can have nested .zip files if incoming stream is
		// a zip file itself; but must be seekable, which implies that
		// the compression method is "store" or that the whole file
		// fits in memory.
		//
		// ---------------------------------------------------------------
		// NOTE: by default, zip files constructed in this way are NOT
		// made global!
		// ---------------------------------------------------------------
		// NOTE: this interface is currently broken, since .ZIP archive
		// members are not seekable, even if they are "stored".
		// ---------------------------------------------------------------
		//
		//
#ifndef ZPP_USE_STDIO
	zppZipArchive(std::istream *istr, std::string &_name, bool _makeGlobal = false);
#endif
		// dtor.
	~zppZipArchive();


private:
	zppFileMap fileMap;			// filename map
	int priority;				// priority read from zip info
	bool isGlobal;				// true if this .ZIP has been added to global list.
	bool ourFile;				// true if we opened (and thus should close) (f)stream.
#ifdef ZPP_USE_STDIO
	FILE *str;
	long current_pos_v;
#else
        std::istream *str;
#endif				// the stream we read from.
    zppEndOfCentralDirHeader ecdHeader;
	long ecdOffset;				// offset to end of central dir
	std::string zipName;				// zip file name (canonical)
	zppStrStrMap attrMap;		// attribute map parsed from zip comment

#ifdef ZPP_INCLUDE_CRYPT
	char *passwd;				// initialized by setPasswd()
#endif /* ZPP_INCLUDE_CRYPT */

public:

#ifdef ZPP_INCLUDE_CRYPT
	// set the password for the zip archive; all files in the archive
	// are extracted with this password.
	char *getPasswd() { return passwd; }
	void setPasswd(char *_passwd) {
		if (passwd) delete[] passwd; passwd = NULL;
		if (_passwd) {
			passwd = new char[strlen(passwd)+1];
			if (passwd == NULL) throw zppError("Can't new[] passwd");
			strcpy(passwd,_passwd);
		}
	}
#endif /* ZPP_INCLUDE_CRYPT */

		// search THIS archive for a file. uses the filename
	zppZipFileInfo *findInArchive(const std::string filename);

		// search THIS archive for a file. uses the filename
	zppZipFileInfo *findInArchive(const char *filename) {
		return findInArchive(std::string(filename));
	}

		// find an attribute in this archive.
	const std::string &findAttr(std::string &_key) {
		static const std::string nullString("");
		zppStrStrMap::iterator i = attrMap.find(_key);

		return ((i == attrMap.end()) ? nullString : (*i).second);
	}

		// return TRUE if an attribute exists in this archive.
	bool attrExists(std::string &_key) {
		zppStrStrMap::iterator i = attrMap.find(_key);

		return ((i == attrMap.end()) ? false : true );
	}

		// return name of archive file we created this object from
	inline const std::string &getName() { return zipName; }

	// it's not strictly a file, but we have some convenience functions
	// to read/seek/tell in the stream.
	// these should probably be private, or at least friend.
	inline bool rawRead(char *_S, std::streamsize _N) {
#ifdef ZPP_USE_STDIO
	  bool res=(_N==(int)fread((void *)_S,1,_N,str));
	  if (res)
	    current_pos_v += _N;
	  else
	    current_pos_v=ftell(str);
	  return res;
#else
		return !(str->read(_S,_N).fail());
#endif
	}

	inline bool rawSeek(long _P) {
#ifdef ZPP_USE_STDIO
	  assert(current_pos_v==ftell(str));
	  if (current_pos_v == _P)
	    return 1;
	  current_pos_v = _P;
	  return !fseek(str,_P,SEEK_SET);
#else
		return !(str->seekg(_P).fail());
#endif
	}

	inline bool rawSeek(long _O, std::ios_base::seekdir _W) {
#ifdef ZPP_USE_STDIO
	  bool res= !fseek(str,_O,_W == std::ios_base::end ? SEEK_END : (abort(),0));
	  current_pos_v=ftell(str);
	  return res;
#else
		return !(str->seekg(_O,_W).fail());
#endif
	}

		// return length of zip archive comment.
	int getCommentLength() { return ecdHeader.commentLength; }

		// read zipComment from disk. assumes that pComment is large
		// enough to hold the comment. (See getCommentLength() above)
		// if 'size' is -1, then the entire comment is read.
		// returns the # of bytes actually read.
	int getComment(char *pComment, int offset = 0, int size = -1);

		// if zip file is not a global zip, then this function has no
		// real effect.
		//
		// HOWEVER, if zip file is global, this updates the priority of
		// all of the contained files in the .ZIP file.  further, if
		//  _sortNow == TRUE, then zppZipArchive::sortGlobal() is called
		// causing all files in the global list to be sorted.
		// this should be considered a VERY heavy weight operation
		// an if multipel zip files are going to be updated at once,
		// then _sortNow should be set to FALSE.
		// note that the most efficient way to have the files priority
		// sorted is to use the zip comment facility and put the file
		// priority in there. (lower priority == better)
	int setPriority(int _prio, bool _sortNow = true);

		// return the global priority of this zip file.
	int getPriority() { return priority; }


		// read local file header from inputstream at 'offset'.
		// throw a zppError on failure.
	void getLclHeader(long offset, zppLocalFileHeader *hdr);

		// read central dir file header from inputstream at 'offset'.
		// throw a zppError on failure.
	void getCentralHeader(long offset, zppCentralDirFileHeader *hdr);

		// return read only reference to file map
	const zppFileMap& getFileMap() const { return fileMap; }

		// if this function is called, we are given ownership to delete
		// the stream we were passed in the constructor when we are deleted.
		// otherwise, we will not delete it.
	inline void setStreamOwnership(bool _ourFile) { ourFile = _ourFile; }

		// class functions
	static void setParseAttrFlag(bool flag) { parseAttrFlag = flag; }
	static int getParseAttrFlag() { return parseAttrFlag; }
	static void setDefaultPriority(int prio) { defaultPriority = prio; }
	static int getDefaultPriority() { return defaultPriority; }
	static zppZipFileInfo *find(const std::string &filename);

	static zppZipFileInfo *find(const char *filename) {
		return find(std::string(filename));
	}

private:
		// parse a zip file; if fsize = 0, then we seekg() to the end
		// of the file to find out where it really ends, otherwise,
		// we seekg() fsize bytes from the beginning.
		// note that we really only deal with zip files < 2G
		// (because of signed-long issues)
		// this is called in the ctor() and will throw a zppError.
	void parseZipDirectory();

		// if parseAttrFlag is set, the zipfile has a comment, and
		// the first four characters of the first line of the zipfile comment 
		// are "%zpp% then the zipfile comment is read and parsed as 
		// a set of token/value pairs.
		// acceptable syntax:
		// token = "value"
		// token = 'value'
		// token = value
		// # comment
		// blank lines are also acceptable.  no backslash quoting of
		// characters is supported.  unless surrounded by quotes (" or '),
		// the value token ends at the first whitespace character.
	void parseAttrMap();

		// search an input stream for the central directory, return
		// the offset.
	long searchCentralDir(long fileSize);

		// default priority for .zip files without parseAttrFlag==TRUE,
		// or without a prio key in the zipcomment.
	static int defaultPriority;

		// if true, try to parse prioirty from zip comment
	static bool parseAttrFlag;

		// modify input parameter 'str' into a canonical path. (lower case
		// and '\\' to -> '/'
		// XXX -- should probably be "hostified"
	static void canonizePath(std::string &str);

		// this map contains ALL files (key is canonical filename,
		// data is iterator referencing the file in it's parent
		// zppZipArchive object.
	static zppFileMap globalMap;

#ifdef ZPP_INCLUDE_OPENALL
		// search for an archive by name.
		// returns a pointer to the archive, or NULL.
		// this executes in O(n) time, since the list of all
		// open archives is just that: a list.
	static zppZipArchive *findArchive(const std::string &name);

	static zppZipArchiveList filesWeOpened;
#endif /* ZPP_INCLUDE_OPENALL */

public:
	static void dumpGlobalMap();
	unsigned long crc32(unsigned long crc, const unsigned char *buf, unsigned int len);
};


#endif /* _H_ZPPLIB_ */
