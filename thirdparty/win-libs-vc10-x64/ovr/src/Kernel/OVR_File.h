/************************************************************************************

PublicHeader:   Kernel
Filename    :   OVR_File.h
Content     :   Header for all internal file management - functions and structures
                to be inherited by OS specific subclasses.
Created     :   September 19, 2012
Notes       : 

Notes       :   errno may not be preserved across use of BaseFile member functions
            :   Directories cannot be deleted while files opened from them are in use
                (For the GetFullName function)

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_File_h
#define OVR_File_h

#include "OVR_RefCount.h"
#include "OVR_Std.h"
#include "OVR_Alg.h"

#include <stdio.h>
#include "OVR_String.h"

namespace OVR {

// ***** Declared classes
class   FileConstants;
class   File;
class   DelegatedFile;
class   BufferedFile;


// ***** Flags for File & Directory accesses

class FileConstants
{
public:

    // *** File open flags
    enum OpenFlags
    {
        Open_Read       = 1,
        Open_Write      = 2,
        Open_ReadWrite  = 3,

        // Opens file and truncates it to zero length
        // - file must have write permission
        // - when used with Create, it opens an existing 
        //   file and empties it or creates a new file
        Open_Truncate   = 4,

        // Creates and opens new file 
        // - does not erase contents if file already
        //   exists unless combined with Truncate
        Open_Create     = 8,

         // Returns an error value if the file already exists
        Open_CreateOnly = 24,

        // Open file with buffering
        Open_Buffered    = 32
    };

    // *** File Mode flags
    enum Modes
    {
        Mode_Read       = 0444,
        Mode_Write      = 0222,
        Mode_Execute    = 0111,

        Mode_ReadWrite  = 0666
    };

    // *** Seek operations
    enum SeekOps
    {
        Seek_Set        = 0,
        Seek_Cur        = 1,
        Seek_End        = 2
    };

    // *** Errors
    enum Errors
    {
        Error_FileNotFound  = 0x1001,
        Error_Access        = 0x1002,
        Error_IOError       = 0x1003,
        Error_DiskFull      = 0x1004
    };
};


//-----------------------------------------------------------------------------------
// ***** File Class

// The pure virtual base random-access file
// This is a base class to all files

class File : public RefCountBase<File>, public FileConstants
{   
public:
    File() { }
    // ** Location Information

    // Returns a file name path relative to the 'reference' directory
    // This is often a path that was used to create a file
    // (this is not a global path, global path can be obtained with help of directory)
    virtual const char* GetFilePath() = 0;
                                                                                        

    // ** File Information

    // Return 1 if file's usable (open)
    virtual bool        IsValid() = 0;
    // Return 1 if file's writable, otherwise 0                                         
    virtual bool        IsWritable() = 0;
                                                                                        
    // Return position
    virtual int         Tell() = 0;
    virtual SInt64      LTell() = 0;
    
    // File size                                                                        
    virtual int         GetLength() = 0;
    virtual SInt64      LGetLength() = 0;
                                                                                        
    // Returns file stats                                                               
    // 0 for failure                                                                    
    //virtual bool      Stat(FileStats *pfs) = 0;
                                                                                        
    // Return errno-based error code                                                    
    // Useful if any other function failed                                              
    virtual int         GetErrorCode() = 0;
                                                                                        
                                                                                        
    // ** Stream implementation & I/O

    // Blocking write, will write in the given number of bytes to the stream
    // Returns : -1 for error
    //           Otherwise number of bytes read 
    virtual int         Write(const UByte *pbufer, int numBytes) = 0;
    // Blocking read, will read in the given number of bytes or less from the stream
    // Returns : -1 for error
    //           Otherwise number of bytes read,
    //           if 0 or < numBytes, no more bytes available; end of file or the other side of stream is closed
    virtual int         Read(UByte *pbufer, int numBytes) = 0;

    // Skips (ignores) a given # of bytes
    // Same return values as Read
    virtual int         SkipBytes(int numBytes) = 0;
        
    // Returns the number of bytes available to read from a stream without blocking
    // For a file, this should generally be number of bytes to the end
    virtual int         BytesAvailable() = 0;

    // Causes any implementation's buffered data to be delivered to destination
    // Return 0 for error
    virtual bool        Flush() = 0;
                                                                                            

    // Need to provide a more optimized implementation that doe snot necessarily involve a lot of seeking
    inline bool         IsEOF() { return !BytesAvailable(); }
    

    // Seeking                                                                              
    // Returns new position, -1 for error                                                   
    virtual int         Seek(int offset, int origin=Seek_Set) = 0;
    virtual SInt64      LSeek(SInt64 offset, int origin=Seek_Set) = 0;
    // Seek simplification
    int                 SeekToBegin()           {return Seek(0); }
    int                 SeekToEnd()             {return Seek(0,Seek_End); }
    int                 Skip(int numBytes)     {return Seek(numBytes,Seek_Cur); }
                        

    // Appends other file data from a stream
    // Return -1 for error, else # of bytes written
    virtual int         CopyFromStream(File *pstream, int byteSize) = 0;

    // Closes the file
    // After close, file cannot be accessed 
    virtual bool        Close() = 0;


    // ***** Inlines for convenient primitive type serialization

    // Read/Write helpers
private:
    UInt64  PRead64()           { UInt64 v = 0; Read((UByte*)&v, 8); return v; }
    UInt32  PRead32()           { UInt32 v = 0; Read((UByte*)&v, 4); return v; }
    UInt16  PRead16()           { UInt16 v = 0; Read((UByte*)&v, 2); return v; }
    UByte   PRead8()            { UByte  v = 0; Read((UByte*)&v, 1); return v; }
    void    PWrite64(UInt64 v)  { Write((UByte*)&v, 8); }
    void    PWrite32(UInt32 v)  { Write((UByte*)&v, 4); }
    void    PWrite16(UInt16 v)  { Write((UByte*)&v, 2); }
    void    PWrite8(UByte v)    { Write((UByte*)&v, 1); }

public:

    // Writing primitive types - Little Endian
    inline void    WriteUByte(UByte v)         { PWrite8((UByte)Alg::ByteUtil::SystemToLE(v));     }
    inline void    WriteSByte(SByte v)         { PWrite8((UByte)Alg::ByteUtil::SystemToLE(v));     }
    inline void    WriteUInt8(UByte v)         { PWrite8((UByte)Alg::ByteUtil::SystemToLE(v));     }
    inline void    WriteSInt8(SByte v)         { PWrite8((UByte)Alg::ByteUtil::SystemToLE(v));     }
    inline void    WriteUInt16(UInt16 v)       { PWrite16((UInt16)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteSInt16(SInt16 v)       { PWrite16((UInt16)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteUInt32(UInt32 v)       { PWrite32((UInt32)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteSInt32(SInt32 v)       { PWrite32((UInt32)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteUInt64(UInt64 v)       { PWrite64((UInt64)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteSInt64(SInt64 v)       { PWrite64((UInt64)Alg::ByteUtil::SystemToLE(v));   }
    inline void    WriteFloat(float v)         { v = Alg::ByteUtil::SystemToLE(v); Write((UByte*)&v, 4); } 
    inline void    WriteDouble(double v)       { v = Alg::ByteUtil::SystemToLE(v); Write((UByte*)&v, 8); }
    // Writing primitive types - Big Endian
    inline void    WriteUByteBE(UByte v)       { PWrite8((UByte)Alg::ByteUtil::SystemToBE(v));     }
    inline void    WriteSByteBE(SByte v)       { PWrite8((UByte)Alg::ByteUtil::SystemToBE(v));     }
    inline void    WriteUInt8BE(UInt16 v)      { PWrite8((UByte)Alg::ByteUtil::SystemToBE(v));     }
    inline void    WriteSInt8BE(SInt16 v)      { PWrite8((UByte)Alg::ByteUtil::SystemToBE(v));     }
    inline void    WriteUInt16BE(UInt16 v)     { PWrite16((UInt16)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteSInt16BE(UInt16 v)     { PWrite16((UInt16)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteUInt32BE(UInt32 v)     { PWrite32((UInt32)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteSInt32BE(UInt32 v)     { PWrite32((UInt32)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteUInt64BE(UInt64 v)     { PWrite64((UInt64)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteSInt64BE(UInt64 v)     { PWrite64((UInt64)Alg::ByteUtil::SystemToBE(v));   }
    inline void    WriteFloatBE(float v)       { v = Alg::ByteUtil::SystemToBE(v); Write((UByte*)&v, 4); }
    inline void    WriteDoubleBE(double v)     { v = Alg::ByteUtil::SystemToBE(v); Write((UByte*)&v, 8); }
        
    // Reading primitive types - Little Endian
    inline UByte   ReadUByte()                 { return (UByte)Alg::ByteUtil::LEToSystem(PRead8());    }
    inline SByte   ReadSByte()                 { return (SByte)Alg::ByteUtil::LEToSystem(PRead8());    }
    inline UByte   ReadUInt8()                 { return (UByte)Alg::ByteUtil::LEToSystem(PRead8());    }
    inline SByte   ReadSInt8()                 { return (SByte)Alg::ByteUtil::LEToSystem(PRead8());    }
    inline UInt16  ReadUInt16()                { return (UInt16)Alg::ByteUtil::LEToSystem(PRead16());  }
    inline SInt16  ReadSInt16()                { return (SInt16)Alg::ByteUtil::LEToSystem(PRead16());  }
    inline UInt32  ReadUInt32()                { return (UInt32)Alg::ByteUtil::LEToSystem(PRead32());  }
    inline SInt32  ReadSInt32()                { return (SInt32)Alg::ByteUtil::LEToSystem(PRead32());  }
    inline UInt64  ReadUInt64()                { return (UInt64)Alg::ByteUtil::LEToSystem(PRead64());  }
    inline SInt64  ReadSInt64()                { return (SInt64)Alg::ByteUtil::LEToSystem(PRead64());  }
    inline float   ReadFloat()                 { float v = 0.0f; Read((UByte*)&v, 4); return Alg::ByteUtil::LEToSystem(v); }
    inline double  ReadDouble()                { double v = 0.0; Read((UByte*)&v, 8); return Alg::ByteUtil::LEToSystem(v); }
    // Reading primitive types - Big Endian
    inline UByte   ReadUByteBE()               { return (UByte)Alg::ByteUtil::BEToSystem(PRead8());    }
    inline SByte   ReadSByteBE()               { return (SByte)Alg::ByteUtil::BEToSystem(PRead8());    }
    inline UByte   ReadUInt8BE()               { return (UByte)Alg::ByteUtil::BEToSystem(PRead8());    }
    inline SByte   ReadSInt8BE()               { return (SByte)Alg::ByteUtil::BEToSystem(PRead8());    }
    inline UInt16  ReadUInt16BE()              { return (UInt16)Alg::ByteUtil::BEToSystem(PRead16());  }
    inline SInt16  ReadSInt16BE()              { return (SInt16)Alg::ByteUtil::BEToSystem(PRead16());  }
    inline UInt32  ReadUInt32BE()              { return (UInt32)Alg::ByteUtil::BEToSystem(PRead32());  }
    inline SInt32  ReadSInt32BE()              { return (SInt32)Alg::ByteUtil::BEToSystem(PRead32());  }
    inline UInt64  ReadUInt64BE()              { return (UInt64)Alg::ByteUtil::BEToSystem(PRead64());  }
    inline SInt64  ReadSInt64BE()              { return (SInt64)Alg::ByteUtil::BEToSystem(PRead64());  }
    inline float   ReadFloatBE()               { float v = 0.0f; Read((UByte*)&v, 4); return Alg::ByteUtil::BEToSystem(v); }
    inline double  ReadDoubleBE()              { double v = 0.0; Read((UByte*)&v, 8); return Alg::ByteUtil::BEToSystem(v); }
};


// *** Delegated File

class DelegatedFile : public File
{
protected:
    // Delegating file pointer
    Ptr<File>     pFile;

    // Hidden default constructor
    DelegatedFile() : pFile(0)                             { }
    DelegatedFile(const DelegatedFile &source) : File()    { OVR_UNUSED(source); }
public:
    // Constructors
    DelegatedFile(File *pfile) : pFile(pfile)     { }

    // ** Location Information  
    virtual const char* GetFilePath()                               { return pFile->GetFilePath(); }    

    // ** File Information                                                      
    virtual bool        IsValid()                                   { return pFile && pFile->IsValid(); }   
    virtual bool        IsWritable()                                { return pFile->IsWritable(); }
//  virtual bool        IsRecoverable()                             { return pFile->IsRecoverable(); }          
                                                                    
    virtual int         Tell()                                      { return pFile->Tell(); }
    virtual SInt64      LTell()                                     { return pFile->LTell(); }
    
    virtual int         GetLength()                                 { return pFile->GetLength(); }
    virtual SInt64      LGetLength()                                { return pFile->LGetLength(); }
    
    //virtual bool      Stat(FileStats *pfs)                        { return pFile->Stat(pfs); }
    
    virtual int         GetErrorCode()                              { return pFile->GetErrorCode(); }
    
    // ** Stream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes)   { return pFile->Write(pbuffer,numBytes); }  
    virtual int         Read(UByte *pbuffer, int numBytes)          { return pFile->Read(pbuffer,numBytes); }   
    
    virtual int         SkipBytes(int numBytes)                     { return pFile->SkipBytes(numBytes); }      
    
    virtual int         BytesAvailable()                            { return pFile->BytesAvailable(); } 
    
    virtual bool        Flush()                                     { return pFile->Flush(); }
                                                                    
    // Seeking                                                      
    virtual int         Seek(int offset, int origin=Seek_Set)       { return pFile->Seek(offset,origin); }
    virtual SInt64      LSeek(SInt64 offset, int origin=Seek_Set)   { return pFile->LSeek(offset,origin); }

    virtual int         CopyFromStream(File *pstream, int byteSize) { return pFile->CopyFromStream(pstream,byteSize); }
                        
    // Closing the file 
    virtual bool        Close()                                     { return pFile->Close(); }    
};


//-----------------------------------------------------------------------------------
// ***** Buffered File

// This file class adds buffering to an existing file
// Buffered file never fails by itself; if there's not
// enough memory for buffer, no buffer's used

class BufferedFile : public DelegatedFile
{   
protected:  
    enum BufferModeType
    {
        NoBuffer,
        ReadBuffer,
        WriteBuffer
    };

    // Buffer & the mode it's in
    UByte*          pBuffer;
    BufferModeType  BufferMode;
    // Position in buffer
    unsigned        Pos;
    // Data in buffer if reading
    unsigned        DataSize;
    // Underlying file position 
    UInt64          FilePos;

    // Initializes buffering to a certain mode
    bool    SetBufferMode(BufferModeType mode);
    // Flushes buffer
    // WriteBuffer - write data to disk, ReadBuffer - reset buffer & fix file position  
    void    FlushBuffer();
    // Loads data into ReadBuffer
    // WARNING: Right now LoadBuffer() assumes the buffer's empty
    void    LoadBuffer();

    // Hidden constructor
    BufferedFile();
    inline BufferedFile(const BufferedFile &source) : DelegatedFile() { OVR_UNUSED(source); }
public:

    // Constructor
    // - takes another file as source
    BufferedFile(File *pfile);
    ~BufferedFile();
    
    
    // ** Overridden functions

    // We override all the functions that can possibly
    // require buffer mode switch, flush, or extra calculations
    virtual int         Tell();
    virtual SInt64      LTell();

    virtual int         GetLength();
    virtual SInt64      LGetLength();

//  virtual bool        Stat(GFileStats *pfs);  

    virtual int         Write(const UByte *pbufer, int numBytes);
    virtual int         Read(UByte *pbufer, int numBytes);

    virtual int         SkipBytes(int numBytes);

    virtual int         BytesAvailable();

    virtual bool        Flush();

    virtual int         Seek(int offset, int origin=Seek_Set);
    virtual SInt64      LSeek(SInt64 offset, int origin=Seek_Set);

    virtual int         CopyFromStream(File *pstream, int byteSize);
    
    virtual bool        Close();    
};                          


//-----------------------------------------------------------------------------------
// ***** Memory File

class MemoryFile : public File
{
public:

    const char* GetFilePath()       { return FilePath.ToCStr(); }

    bool        IsValid()           { return Valid; }
    bool        IsWritable()        { return false; }

    bool        Flush()             { return true; }
    int         GetErrorCode()      { return 0; }

    int         Tell()              { return FileIndex; }
    SInt64      LTell()             { return (SInt64) FileIndex; }

    int         GetLength()         { return FileSize; }
    SInt64      LGetLength()        { return (SInt64) FileSize; }

    bool        Close()
    {
        Valid = false;
        return false;
    }

    int         CopyFromStream(File *pstream, int byteSize)
    {   OVR_UNUSED2(pstream, byteSize);
        return 0;
    }

    int         Write(const UByte *pbuffer, int numBytes)
    {   OVR_UNUSED2(pbuffer, numBytes);
        return 0;
    }

    int         Read(UByte *pbufer, int numBytes)
    {
        if (FileIndex + numBytes > FileSize)
        {
            numBytes = FileSize - FileIndex;
        }

        if (numBytes > 0)
        {
            ::memcpy (pbufer, &FileData [FileIndex], numBytes);

            FileIndex += numBytes;
        }

        return numBytes;
    }

    int         SkipBytes(int numBytes)
    {
        if (FileIndex + numBytes > FileSize)
        {
            numBytes = FileSize - FileIndex;
        }

        FileIndex += numBytes;

        return numBytes;
    }

    int         BytesAvailable()
    {
        return (FileSize - FileIndex);
    }

    int         Seek(int offset, int origin = Seek_Set)
    {
        switch (origin)
        {
        case Seek_Set : FileIndex  = offset;               break;
        case Seek_Cur : FileIndex += offset;               break;
        case Seek_End : FileIndex  = FileSize - offset;  break;
        }

        return FileIndex;
    }

    SInt64      LSeek(SInt64 offset, int origin = Seek_Set)
    {
        return (SInt64) Seek((int) offset, origin);
    }

public:

    MemoryFile (const String& fileName, const UByte *pBuffer, int buffSize)
        : FilePath(fileName)
    {
        FileData  = pBuffer;
        FileSize  = buffSize;
        FileIndex = 0;
        Valid     = (!fileName.IsEmpty() && pBuffer && buffSize > 0) ? true : false;
    }

    // pfileName should be encoded as UTF-8 to support international file names.
    MemoryFile (const char* pfileName, const UByte *pBuffer, int buffSize)
        : FilePath(pfileName)
    {
        FileData  = pBuffer;
        FileSize  = buffSize;
        FileIndex = 0;
        Valid     = (pfileName && pBuffer && buffSize > 0) ? true : false;
    }
private:

    String       FilePath;
    const UByte *FileData;
    int          FileSize;
    int          FileIndex;
    bool         Valid;
};


// ***** Global path helpers

// Find trailing short filename in a path.
const char* OVR_CDECL GetShortFilename(const char* purl);

} // OVR

#endif
