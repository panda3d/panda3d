/**************************************************************************

Filename    :   OVR_File.cpp
Content     :   File wrapper class implementation (Win32)

Created     :   April 5, 1999
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define  GFILE_CXX

// Standard C library (Captain Obvious guarantees!)
#include <stdio.h>

#include "OVR_File.h"

namespace OVR {

// Buffered file adds buffering to an existing file
// FILEBUFFER_SIZE defines the size of internal buffer, while
// FILEBUFFER_TOLERANCE controls the amount of data we'll effectively try to buffer
#define FILEBUFFER_SIZE         (8192-8)
#define FILEBUFFER_TOLERANCE    4096

// ** Constructor/Destructor

// Hidden constructor
// Not supposed to be used
BufferedFile::BufferedFile() : DelegatedFile(0)
{
    pBuffer     = (UByte*)OVR_ALLOC(FILEBUFFER_SIZE);
    BufferMode  = NoBuffer;
    FilePos     = 0;
    Pos         = 0;
    DataSize    = 0;
}

// Takes another file as source
BufferedFile::BufferedFile(File *pfile) : DelegatedFile(pfile)
{
    pBuffer     = (UByte*)OVR_ALLOC(FILEBUFFER_SIZE);
    BufferMode  = NoBuffer;
    FilePos     = pfile->LTell();
    Pos         = 0;
    DataSize    = 0;
}


// Destructor
BufferedFile::~BufferedFile()
{
    // Flush in case there's data
    if (pFile)
        FlushBuffer();
    // Get rid of buffer
    if (pBuffer)
        OVR_FREE(pBuffer);
}

/*
bool    BufferedFile::VCopy(const Object &source)
{
    if (!DelegatedFile::VCopy(source))
        return 0;

    // Data members
    BufferedFile *psource = (BufferedFile*)&source;

    // Buffer & the mode it's in
    pBuffer         = psource->pBuffer;
    BufferMode      = psource->BufferMode;
    Pos             = psource->Pos;
    DataSize        = psource->DataSize;
    return 1;
}
*/

// Initializes buffering to a certain mode
bool    BufferedFile::SetBufferMode(BufferModeType mode)
{
    if (!pBuffer)
        return false;
    if (mode == BufferMode)
        return true;
     
    FlushBuffer();

    // Can't set write mode if we can't write
    if ((mode==WriteBuffer) && (!pFile || !pFile->IsWritable()) )
        return 0;

    // And SetMode
    BufferMode = mode;
    Pos        = 0;
    DataSize   = 0;
    return 1;
}

// Flushes buffer
void    BufferedFile::FlushBuffer()
{
    switch(BufferMode)
    {
        case WriteBuffer:
            // Write data in buffer
            FilePos += pFile->Write(pBuffer,Pos);
            Pos = 0;
            break;

        case ReadBuffer:
            // Seek back & reset buffer data
            if ((DataSize-Pos)>0)
                FilePos = pFile->LSeek(-(int)(DataSize-Pos), Seek_Cur);
            DataSize = 0;
            Pos      = 0;
            break;
        default:
            // not handled!
            break;
    }
}

// Reloads data for ReadBuffer
void    BufferedFile::LoadBuffer()
{
    if (BufferMode == ReadBuffer)
    {
        // We should only reload once all of pre-loaded buffer is consumed.
        OVR_ASSERT(Pos == DataSize);

        // WARNING: Right now LoadBuffer() assumes the buffer's empty
        int sz   = pFile->Read(pBuffer,FILEBUFFER_SIZE);
        DataSize = sz<0 ? 0 : (unsigned)sz;
        Pos      = 0;
        FilePos  += DataSize;
    }
}


// ** Overridden functions

// We override all the functions that can possibly
// require buffer mode switch, flush, or extra calculations

// Tell() requires buffer adjustment
int     BufferedFile::Tell()
{
    if (BufferMode == ReadBuffer)
        return int (FilePos - DataSize + Pos);

    int pos = pFile->Tell();
    // Adjust position based on buffer mode & data
    if (pos!=-1)
    {
        OVR_ASSERT(BufferMode != ReadBuffer);
        if (BufferMode == WriteBuffer)
            pos += Pos;
    }
    return pos;
}

SInt64  BufferedFile::LTell()
{
    if (BufferMode == ReadBuffer)
        return FilePos - DataSize + Pos;

    SInt64 pos = pFile->LTell();
    if (pos!=-1)
    {
        OVR_ASSERT(BufferMode != ReadBuffer);
        if (BufferMode == WriteBuffer)
            pos += Pos;
    }
    return pos;
}

int     BufferedFile::GetLength()
{
    int len = pFile->GetLength();
    // If writing through buffer, file length may actually be bigger
    if ((len!=-1) && (BufferMode==WriteBuffer))
    {
        int currPos = pFile->Tell() + Pos;
        if (currPos>len)
            len = currPos;
    }
    return len;
}
SInt64  BufferedFile::LGetLength()
{
    SInt64 len = pFile->LGetLength();
    // If writing through buffer, file length may actually be bigger
    if ((len!=-1) && (BufferMode==WriteBuffer))
    {
        SInt64 currPos = pFile->LTell() + Pos;
        if (currPos>len)
            len = currPos;
    }
    return len;
}

/*
bool    BufferedFile::Stat(FileStats *pfs)
{
    // Have to fix up length is stat
    if (pFile->Stat(pfs))
    {
        if (BufferMode==WriteBuffer)
        {
            SInt64 currPos = pFile->LTell() + Pos;
            if (currPos > pfs->Size)
            {
                pfs->Size   = currPos;
                // ??
                pfs->Blocks = (pfs->Size+511) >> 9;
            }
        }
        return 1;
    }
    return 0;
}
*/

int     BufferedFile::Write(const UByte *psourceBuffer, int numBytes)
{
    if ( (BufferMode==WriteBuffer) || SetBufferMode(WriteBuffer))
    {
        // If not data space in buffer, flush
        if ((FILEBUFFER_SIZE-(int)Pos)<numBytes)
        {
            FlushBuffer();
            // If bigger then tolerance, just write directly
            if (numBytes>FILEBUFFER_TOLERANCE)
            {
                int sz = pFile->Write(psourceBuffer,numBytes);
                if (sz > 0)
                    FilePos += sz;
                return sz;
            }
        }

        // Enough space in buffer.. so copy to it
        memcpy(pBuffer+Pos, psourceBuffer, numBytes);
        Pos += numBytes;
        return numBytes;
    }
    int sz = pFile->Write(psourceBuffer,numBytes);
    if (sz > 0)
        FilePos += sz;
    return sz;
}

int     BufferedFile::Read(UByte *pdestBuffer, int numBytes)
{
    if ( (BufferMode==ReadBuffer) || SetBufferMode(ReadBuffer))
    {
        // Data in buffer... copy it
        if ((int)(DataSize-Pos) >= numBytes)
        {
            memcpy(pdestBuffer, pBuffer+Pos, numBytes);
            Pos += numBytes;
            return numBytes;
        }

        // Not enough data in buffer, copy buffer
        int     readBytes = DataSize-Pos;
        memcpy(pdestBuffer, pBuffer+Pos, readBytes);
        numBytes    -= readBytes;
        pdestBuffer += readBytes;
        Pos = DataSize;

        // Don't reload buffer if more then tolerance
        // (No major advantage, and we don't want to write a loop)
        if (numBytes>FILEBUFFER_TOLERANCE)
        {
            numBytes = pFile->Read(pdestBuffer,numBytes);
            if (numBytes > 0)
            {
                FilePos += numBytes;
                Pos = DataSize = 0;
            }
            return readBytes + ((numBytes==-1) ? 0 : numBytes);
        }

        // Reload the buffer
        // WARNING: Right now LoadBuffer() assumes the buffer's empty
        LoadBuffer();
        if ((int)(DataSize-Pos) < numBytes)
            numBytes = (int)DataSize-Pos;

        memcpy(pdestBuffer, pBuffer+Pos, numBytes);
        Pos += numBytes;
        return numBytes + readBytes;
        
        /*
        // Alternative Read implementation. The one above is probably better
        // due to FILEBUFFER_TOLERANCE.
        int     total = 0;

        do {
            int     bufferBytes = (int)(DataSize-Pos);
            int     copyBytes = (bufferBytes > numBytes) ? numBytes : bufferBytes;

            memcpy(pdestBuffer, pBuffer+Pos, copyBytes);
            numBytes    -= copyBytes;
            pdestBuffer += copyBytes;
            Pos         += copyBytes;
            total       += copyBytes;

            if (numBytes == 0)
                break;
            LoadBuffer();

        } while (DataSize > 0);

        return total;
        */     
    }
    int sz = pFile->Read(pdestBuffer,numBytes);
    if (sz > 0)
        FilePos += sz;
    return sz;
}


int     BufferedFile::SkipBytes(int numBytes)
{
    int skippedBytes = 0;

    // Special case for skipping a little data in read buffer
    if (BufferMode==ReadBuffer)
    {
        skippedBytes = (((int)DataSize-(int)Pos) >= numBytes) ? numBytes : (DataSize-Pos);
        Pos          += skippedBytes;
        numBytes     -= skippedBytes;
    }

    if (numBytes)
    {
        numBytes = pFile->SkipBytes(numBytes);
        // Make sure we return the actual number skipped, or error
        if (numBytes!=-1)
        {
            skippedBytes += numBytes;
            FilePos += numBytes;
            Pos = DataSize = 0;
        }
        else if (skippedBytes <= 0)
            skippedBytes = -1;
    }
    return skippedBytes;
}

int     BufferedFile::BytesAvailable()
{
    int available = pFile->BytesAvailable();
    // Adjust available size based on buffers
    switch(BufferMode)
    {
        case ReadBuffer:
            available += DataSize-Pos;
            break;
        case WriteBuffer:
            available -= Pos;
            if (available<0)
                available= 0;
            break;
        default:
            break;
    }
    return available;
}

bool    BufferedFile::Flush()
{
    FlushBuffer();
    return pFile->Flush();
}

// Seeking could be optimized better..
int     BufferedFile::Seek(int offset, int origin)
{    
    if (BufferMode == ReadBuffer)
    {
        if (origin == Seek_Cur)
        {
            // Seek can fall either before or after Pos in the buffer,
            // but it must be within bounds.
            if (((unsigned(offset) + Pos)) <= DataSize)
            {
                Pos += offset;
                return int (FilePos - DataSize + Pos);
            }

            // Lightweight buffer "Flush". We do this to avoid an extra seek
            // back operation which would take place if we called FlushBuffer directly.
            origin = Seek_Set;
            OVR_ASSERT(((FilePos - DataSize + Pos) + (UInt64)offset) < ~(UInt64)0);
            offset = (int)(FilePos - DataSize + Pos) + offset;
            Pos = DataSize = 0;
        }
        else if (origin == Seek_Set)
        {
            if (((unsigned)offset - (FilePos-DataSize)) <= DataSize)
            {
                OVR_ASSERT((FilePos-DataSize) < ~(UInt64)0);
                Pos = (unsigned)offset - (unsigned)(FilePos-DataSize);
                return offset;
            }
            Pos = DataSize = 0;
        }
        else
        {
            FlushBuffer();
        }
    }
    else
    {
        FlushBuffer();
    }    

    /*
    // Old Seek Logic
    if (origin == Seek_Cur && offset + Pos < DataSize)
    {
        //OVR_ASSERT((FilePos - DataSize) >= (FilePos - DataSize + Pos + offset));
        Pos += offset;
        OVR_ASSERT(int (Pos) >= 0);
        return int (FilePos - DataSize + Pos);
    }
    else if (origin == Seek_Set && unsigned(offset) >= FilePos - DataSize && unsigned(offset) < FilePos)
    {
        Pos = unsigned(offset - FilePos + DataSize);
        OVR_ASSERT(int (Pos) >= 0);
        return int (FilePos - DataSize + Pos);
    }   
    
    FlushBuffer();
    */


    FilePos = pFile->Seek(offset,origin);
    return int (FilePos);
}

SInt64  BufferedFile::LSeek(SInt64 offset, int origin)
{
    if (BufferMode == ReadBuffer)
    {
        if (origin == Seek_Cur)
        {
            // Seek can fall either before or after Pos in the buffer,
            // but it must be within bounds.
            if (((unsigned(offset) + Pos)) <= DataSize)
            {
                Pos += (unsigned)offset;
                return SInt64(FilePos - DataSize + Pos);
            }

            // Lightweight buffer "Flush". We do this to avoid an extra seek
            // back operation which would take place if we called FlushBuffer directly.
            origin = Seek_Set;            
            offset = (SInt64)(FilePos - DataSize + Pos) + offset;
            Pos = DataSize = 0;
        }
        else if (origin == Seek_Set)
        {
            if (((UInt64)offset - (FilePos-DataSize)) <= DataSize)
            {                
                Pos = (unsigned)((UInt64)offset - (FilePos-DataSize));
                return offset;
            }
            Pos = DataSize = 0;
        }
        else
        {
            FlushBuffer();
        }
    }
    else
    {
        FlushBuffer();
    }

/*
    OVR_ASSERT(BufferMode != NoBuffer);

    if (origin == Seek_Cur && offset + Pos < DataSize)
    {
        Pos += int (offset);
        return FilePos - DataSize + Pos;
    }
    else if (origin == Seek_Set && offset >= SInt64(FilePos - DataSize) && offset < SInt64(FilePos))
    {
        Pos = unsigned(offset - FilePos + DataSize);
        return FilePos - DataSize + Pos;
    }

    FlushBuffer();
    */

    FilePos = pFile->LSeek(offset,origin);
    return FilePos;
}

int     BufferedFile::CopyFromStream(File *pstream, int byteSize)
{
    // We can't rely on overridden Write()
    // because delegation doesn't override virtual pointers
    // So, just re-implement
    UByte   buff[0x4000];
    int     count = 0;
    int     szRequest, szRead, szWritten;

    while(byteSize)
    {
        szRequest = (byteSize > int(sizeof(buff))) ? int(sizeof(buff)) : byteSize;

        szRead    = pstream->Read(buff,szRequest);
        szWritten = 0;
        if (szRead > 0)
            szWritten = Write(buff,szRead);

        count   +=szWritten;
        byteSize-=szWritten;
        if (szWritten < szRequest)
            break;
    }
    return count;
}

// Closing files
bool    BufferedFile::Close()
{
    switch(BufferMode)
    {
        case WriteBuffer:
            FlushBuffer();
            break;
        case ReadBuffer:
            // No need to seek back on close
            BufferMode = NoBuffer;
            break;
        default:
            break;
    }
    return pFile->Close();
}


// ***** Global path helpers

// Find trailing short filename in a path.
const char* OVR_CDECL GetShortFilename(const char* purl)
{    
    UPInt len = OVR_strlen(purl);
    for (UPInt i=len; i>0; i--) 
        if (purl[i]=='\\' || purl[i]=='/')
            return purl+i+1;
    return purl;
}

} // OVR

