/**************************************************************************

Filename    :   OVR_SysFile.cpp
Content     :   File wrapper class implementation (Win32)

Created     :   April 5, 1999
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define  GFILE_CXX

// Standard C library (Captain Obvious guarantees!)
#include <stdio.h>

#include "OVR_SysFile.h"

namespace OVR {

// This is - a dummy file that fails on all calls.

class UnopenedFile : public File
{
public:
    UnopenedFile()  { }
    ~UnopenedFile() { }

    virtual const char* GetFilePath()               { return 0; }

    // ** File Information
    virtual bool        IsValid()                   { return 0; }
    virtual bool        IsWritable()                { return 0; }

    // Return position / file size
    virtual int         Tell()                      { return 0; }
    virtual SInt64      LTell()                     { return 0; }
    virtual int         GetLength()                 { return 0; }
    virtual SInt64      LGetLength()                { return 0; }

//  virtual bool        Stat(FileStats *pfs)        { return 0; }
    virtual int         GetErrorCode()              { return Error_FileNotFound; }

    // ** Stream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes)     { return -1; OVR_UNUSED2(pbuffer, numBytes); }
    virtual int         Read(UByte *pbuffer, int numBytes)            { return -1; OVR_UNUSED2(pbuffer, numBytes); }
    virtual int         SkipBytes(int numBytes)                       { return 0;  OVR_UNUSED(numBytes); }
    virtual int         BytesAvailable()                              { return 0; }
    virtual bool        Flush()                                       { return 0; }
    virtual int         Seek(int offset, int origin)                  { return -1; OVR_UNUSED2(offset, origin); }
    virtual SInt64      LSeek(SInt64 offset, int origin)              { return -1; OVR_UNUSED2(offset, origin); }
    
    virtual int         CopyFromStream(File *pstream, int byteSize)   { return -1; OVR_UNUSED2(pstream, byteSize); }
    virtual bool        Close()                                       { return 0; }    
};



// ***** System File

// System file is created to access objects on file system directly
// This file can refer directly to path

// ** Constructor
SysFile::SysFile() : DelegatedFile(0)
{
    pFile = *new UnopenedFile;
}

File* FileFILEOpen(const String& path, int flags, int mode);

// Opens a file
SysFile::SysFile(const String& path, int flags, int mode) : DelegatedFile(0)
{
    Open(path, flags, mode);
}


// ** Open & management
// Will fail if file's already open
bool SysFile::Open(const String& path, int flags, int mode)
{
    pFile = *FileFILEOpen(path, flags, mode);
    if ((!pFile) || (!pFile->IsValid()))
    {
        pFile = *new UnopenedFile;
        return 0;
    }
    //pFile = *OVR_NEW DelegatedFile(pFile); // MA Testing
    if (flags & Open_Buffered)
        pFile = *new BufferedFile(pFile);
    return 1;
}


// ** Overrides

int SysFile::GetErrorCode()
{
    return pFile ? pFile->GetErrorCode() : Error_FileNotFound;
}


// Overrides to provide re-open support
bool SysFile::IsValid()
{
    return pFile && pFile->IsValid();
}
bool SysFile::Close()
{
    if (IsValid())
    {
        DelegatedFile::Close();
        pFile = *new UnopenedFile;
        return 1;
    }
    return 0;
}

} // OVR
