/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_String.h
Content     :   String UTF8 string implementation with copy-on-write semantics
                (thread-safe for assignment but not modification).
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_String_h
#define OVR_String_h

#include "OVR_Types.h"
#include "OVR_Allocator.h"
#include "OVR_UTF8Util.h"
#include "OVR_Atomic.h"
#include "OVR_Std.h"
#include "OVR_Alg.h"

namespace OVR {

// ***** Classes

class String;
class StringBuffer;


//-----------------------------------------------------------------------------------
// ***** String Class 

// String is UTF8 based string class with copy-on-write implementation
// for assignment.

class String
{
protected:

    enum FlagConstants
    {
        //Flag_GetLength      = 0x7FFFFFFF,
        // This flag is set if GetLength() == GetSize() for a string.
        // Avoid extra scanning is Substring and indexing logic.
        Flag_LengthIsSizeShift   = (sizeof(UPInt)*8 - 1)
    };


    // Internal structure to hold string data
    struct DataDesc
    {
        // Number of bytes. Will be the same as the number of chars if the characters
        // are ascii, may not be equal to number of chars in case string data is UTF8.
        UPInt   Size;       
        volatile SInt32 RefCount;
        char    Data[1];

        void    AddRef()
        {
            AtomicOps<SInt32>::ExchangeAdd_NoSync(&RefCount, 1);
        }
        // Decrement ref count. This needs to be thread-safe, since
        // a different thread could have also decremented the ref count.
        // For example, if u start off with a ref count = 2. Now if u
        // decrement the ref count and check against 0 in different
        // statements, a different thread can also decrement the ref count
        // in between our decrement and checking against 0 and will find
        // the ref count = 0 and delete the object. This will lead to a crash
        // when context switches to our thread and we'll be trying to delete
        // an already deleted object. Hence decrementing the ref count and
        // checking against 0 needs to made an atomic operation.
        void    Release()
        {
            if ((AtomicOps<SInt32>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
                OVR_FREE(this);
        }

        static UPInt GetLengthFlagBit()     { return UPInt(1) << Flag_LengthIsSizeShift; }
        UPInt       GetSize() const         { return Size & ~GetLengthFlagBit() ; }
        UPInt       GetLengthFlag()  const  { return Size & GetLengthFlagBit(); }
        bool        LengthIsSize() const    { return GetLengthFlag() != 0; }
    };

    // Heap type of the string is encoded in the lower bits.
    enum HeapType
    {
        HT_Global   = 0,    // Heap is global.
        HT_Local    = 1,    // SF::String_loc: Heap is determined based on string's address.
        HT_Dynamic  = 2,    // SF::String_temp: Heap is stored as a part of the class.
        HT_Mask     = 3
    };

    union {
        DataDesc* pData;
        UPInt     HeapTypeBits;
    };
    typedef union {
        DataDesc* pData;
        UPInt     HeapTypeBits;
    } DataDescUnion;

    inline HeapType    GetHeapType() const { return (HeapType) (HeapTypeBits & HT_Mask); }

    inline DataDesc*   GetData() const
    {
        DataDescUnion u;
        u.pData    = pData;
        u.HeapTypeBits = (u.HeapTypeBits & ~(UPInt)HT_Mask);
        return u.pData;
    }
    
    inline void        SetData(DataDesc* pdesc)
    {
        HeapType ht = GetHeapType();
        pData = pdesc;
        OVR_ASSERT((HeapTypeBits & HT_Mask) == 0);
        HeapTypeBits |= ht;        
    }

    
    DataDesc*   AllocData(UPInt size, UPInt lengthIsSize);
    DataDesc*   AllocDataCopy1(UPInt size, UPInt lengthIsSize,
                               const char* pdata, UPInt copySize);
    DataDesc*   AllocDataCopy2(UPInt size, UPInt lengthIsSize,
                               const char* pdata1, UPInt copySize1,
                               const char* pdata2, UPInt copySize2);

    // Special constructor to avoid data initalization when used in derived class.
    struct NoConstructor { };
    String(const NoConstructor&) { }

public:

    // For initializing string with dynamic buffer
    struct InitStruct
    {
        virtual ~InitStruct() { }
        virtual void InitString(char* pbuffer, UPInt size) const = 0;
    };


    // Constructors / Destructors.
    String();
    String(const char* data);
    String(const char* data1, const char* pdata2, const char* pdata3 = 0);
    String(const char* data, UPInt buflen);
    String(const String& src);
    String(const StringBuffer& src);
    String(const InitStruct& src, UPInt size);
    explicit String(const wchar_t* data);      

    // Destructor (Captain Obvious guarantees!)
    ~String()
    {
        GetData()->Release();
    }

    // Declaration of NullString
    static DataDesc NullData;


    // *** General Functions

    void        Clear();

    // For casting to a pointer to char.
    operator const char*() const        { return GetData()->Data; }
    // Pointer to raw buffer.
    const char* ToCStr() const          { return GetData()->Data; }

    // Returns number of bytes
    UPInt       GetSize() const         { return GetData()->GetSize() ; }
    // Tells whether or not the string is empty
    bool        IsEmpty() const         { return GetSize() == 0; }

    // Returns  number of characters
    UPInt       GetLength() const;

    // Returns  character at the specified index
    UInt32      GetCharAt(UPInt index) const;
    UInt32      GetFirstCharAt(UPInt index, const char** offset) const;
    UInt32      GetNextChar(const char** offset) const;

    // Appends a character
    void        AppendChar(UInt32 ch);

    // Append a string
    void        AppendString(const wchar_t* pstr, SPInt len = -1);
    void        AppendString(const char* putf8str, SPInt utf8StrSz = -1);

    // Assigned a string with dynamic data (copied through initializer).
    void        AssignString(const InitStruct& src, UPInt size);
    // Assigns string with known size.
    void        AssignString(const char* putf8str, UPInt size);

    //  Resize the string to the new size
//  void        Resize(UPInt _size);

    // Removes the character at posAt
    void        Remove(UPInt posAt, SPInt len = 1);

    // Returns a String that's a substring of this.
    //  -start is the index of the first UTF8 character you want to include.
    //  -end is the index one past the last UTF8 character you want to include.
    String   Substring(UPInt start, UPInt end) const;

    // Case-conversion
    String   ToUpper() const;
    String   ToLower() const;

    // Inserts substr at posAt
    String&    Insert (const char* substr, UPInt posAt, SPInt len = -1);

    // Inserts character at posAt
    UPInt       InsertCharAt(UInt32 c, UPInt posAt);

    // Inserts substr at posAt, which is an index of a character (not byte).
    // Of size is specified, it is in bytes.
//  String&    Insert(const UInt32* substr, UPInt posAt, SPInt size = -1);

    // Get Byte index of the character at position = index
    UPInt       GetByteIndex(UPInt index) const { return (UPInt)UTF8Util::GetByteIndex(index, GetData()->Data); }

    // Utility: case-insensitive string compare.  stricmp() & strnicmp() are not
    // ANSI or POSIX, do not seem to appear in Linux.
    static int OVR_STDCALL   CompareNoCase(const char* a, const char* b);
    static int OVR_STDCALL   CompareNoCase(const char* a, const char* b, SPInt len);

    // Hash function, case-insensitive
    static UPInt OVR_STDCALL BernsteinHashFunctionCIS(const void* pdataIn, UPInt size, UPInt seed = 5381);

    // Hash function, case-sensitive
    static UPInt OVR_STDCALL BernsteinHashFunction(const void* pdataIn, UPInt size, UPInt seed = 5381);


    // ***** File path parsing helper functions.
    // Implemented in OVR_String_FilePath.cpp.

    // Absolute paths can star with:
    //  - protocols:        'file://', 'http://'
    //  - windows drive:    'c:\'
    //  - UNC share name:   '\\share'
    //  - unix root         '/'
    static bool HasAbsolutePath(const char* path);
    static bool HasExtension(const char* path);
    static bool HasProtocol(const char* path);

    bool    HasAbsolutePath() const { return HasAbsolutePath(ToCStr()); }
    bool    HasExtension() const    { return HasExtension(ToCStr()); }
    bool    HasProtocol() const     { return HasProtocol(ToCStr()); }

    String  GetProtocol() const;    // Returns protocol, if any, with trailing '://'.
    String  GetPath() const;        // Returns path with trailing '/'.
    String  GetFilename() const;    // Returns filename, including extension.
    String  GetExtension() const;   // Returns extension with a dot.

    void    StripProtocol();        // Strips front protocol, if any, from the string.
    void    StripExtension();       // Strips off trailing extension.
    

    // Operators
    // Assignment
    void        operator =  (const char* str);
    void        operator =  (const wchar_t* str);
    void        operator =  (const String& src);
    void        operator =  (const StringBuffer& src);

    // Addition
    void        operator += (const String& src);
    void        operator += (const char* psrc)       { AppendString(psrc); }
    void        operator += (const wchar_t* psrc)    { AppendString(psrc); }
    void        operator += (char  ch)               { AppendChar(ch); }
    String      operator +  (const char* str) const;
    String      operator +  (const String& src)  const;

    // Comparison
    bool        operator == (const String& str) const
    {
        return (OVR_strcmp(GetData()->Data, str.GetData()->Data)== 0);
    }

    bool        operator != (const String& str) const
    {
        return !operator == (str);
    }

    bool        operator == (const char* str) const
    {
        return OVR_strcmp(GetData()->Data, str) == 0;
    }

    bool        operator != (const char* str) const
    {
        return !operator == (str);
    }

    bool        operator <  (const char* pstr) const
    {
        return OVR_strcmp(GetData()->Data, pstr) < 0;
    }

    bool        operator <  (const String& str) const
    {
        return *this < str.GetData()->Data;
    }

    bool        operator >  (const char* pstr) const
    {
        return OVR_strcmp(GetData()->Data, pstr) > 0;
    }

    bool        operator >  (const String& str) const
    {
        return *this > str.GetData()->Data;
    }

    int CompareNoCase(const char* pstr) const
    {
        return CompareNoCase(GetData()->Data, pstr);
    }
    int CompareNoCase(const String& str) const
    {
        return CompareNoCase(GetData()->Data, str.ToCStr());
    }

    // Accesses raw bytes
    const char&     operator [] (int index) const
    {
        OVR_ASSERT(index >= 0 && (UPInt)index < GetSize());
        return GetData()->Data[index];
    }
    const char&     operator [] (UPInt index) const
    {
        OVR_ASSERT(index < GetSize());
        return GetData()->Data[index];
    }


    // Case insensitive keys are used to look up insensitive string in hash tables
    // for SWF files with version before SWF 7.
    struct NoCaseKey
    {   
        const String* pStr;
        NoCaseKey(const String &str) : pStr(&str){};
    };

    bool    operator == (const NoCaseKey& strKey) const
    {
        return (CompareNoCase(ToCStr(), strKey.pStr->ToCStr()) == 0);
    }
    bool    operator != (const NoCaseKey& strKey) const
    {
        return !(CompareNoCase(ToCStr(), strKey.pStr->ToCStr()) == 0);
    }

    // Hash functor used for strings.
    struct HashFunctor
    {    
        UPInt  operator()(const String& data) const
        {
            UPInt  size = data.GetSize();
            return String::BernsteinHashFunction((const char*)data, size);
        }        
    };
    // Case-insensitive hash functor used for strings. Supports additional
    // lookup based on NoCaseKey.
    struct NoCaseHashFunctor
    {    
        UPInt  operator()(const String& data) const
        {
            UPInt  size = data.GetSize();
            return String::BernsteinHashFunctionCIS((const char*)data, size);
        }
        UPInt  operator()(const NoCaseKey& data) const
        {       
            UPInt  size = data.pStr->GetSize();
            return String::BernsteinHashFunctionCIS((const char*)data.pStr->ToCStr(), size);
        }
    };

};


//-----------------------------------------------------------------------------------
// ***** String Buffer used for Building Strings

class StringBuffer
{
    char*           pData;
    UPInt           Size;
    UPInt           BufferSize;
    UPInt           GrowSize;    
    mutable bool    LengthIsSize;    

public:

    // Constructors / Destructor.    
    StringBuffer();
    explicit StringBuffer(UPInt growSize);
    StringBuffer(const char* data);
    StringBuffer(const char* data, UPInt buflen);
    StringBuffer(const String& src);
    StringBuffer(const StringBuffer& src);
    explicit StringBuffer(const wchar_t* data);
    ~StringBuffer();
    

    // Modify grow size used for growing/shrinking the buffer.
    UPInt       GetGrowSize() const         { return GrowSize; }
    void        SetGrowSize(UPInt growSize);
    

    // *** General Functions
    // Does not release memory, just sets Size to 0
    void        Clear();

    // For casting to a pointer to char.
    operator const char*() const        { return (pData) ? pData : ""; }
    // Pointer to raw buffer.
    const char* ToCStr() const          { return (pData) ? pData : ""; }

    // Returns number of bytes.
    UPInt       GetSize() const         { return Size ; }
    // Tells whether or not the string is empty.
    bool        IsEmpty() const         { return GetSize() == 0; }

    // Returns  number of characters
    UPInt       GetLength() const;

    // Returns  character at the specified index
    UInt32      GetCharAt(UPInt index) const;
    UInt32      GetFirstCharAt(UPInt index, const char** offset) const;
    UInt32      GetNextChar(const char** offset) const;


    //  Resize the string to the new size
    void        Resize(UPInt _size);
    void        Reserve(UPInt _size);

    // Appends a character
    void        AppendChar(UInt32 ch);

    // Append a string
    void        AppendString(const wchar_t* pstr, SPInt len = -1);
    void        AppendString(const char* putf8str, SPInt utf8StrSz = -1);
    void        AppendFormat(const char* format, ...);

    // Assigned a string with dynamic data (copied through initializer).
    //void        AssignString(const InitStruct& src, UPInt size);

    // Inserts substr at posAt
    void        Insert (const char* substr, UPInt posAt, SPInt len = -1);
    // Inserts character at posAt
    UPInt       InsertCharAt(UInt32 c, UPInt posAt);

    // Assignment
    void        operator =  (const char* str);
    void        operator =  (const wchar_t* str);
    void        operator =  (const String& src);

    // Addition
    void        operator += (const String& src)      { AppendString(src.ToCStr(),src.GetSize()); }
    void        operator += (const char* psrc)       { AppendString(psrc); }
    void        operator += (const wchar_t* psrc)    { AppendString(psrc); }
    void        operator += (char  ch)               { AppendChar(ch); }
    //String   operator +  (const char* str) const ;
    //String   operator +  (const String& src)  const ;

    // Accesses raw bytes
    char&       operator [] (int index)
    {
        OVR_ASSERT(((UPInt)index) < GetSize());
        return pData[index];
    }
    char&       operator [] (UPInt index)
    {
        OVR_ASSERT(index < GetSize());
        return pData[index];
    }

    const char&     operator [] (int index) const 
    {
        OVR_ASSERT(((UPInt)index) < GetSize());
        return pData[index];
    }
    const char&     operator [] (UPInt index) const
    {
        OVR_ASSERT(index < GetSize());
        return pData[index];
    }
};


//
// Wrapper for string data. The data must have a guaranteed 
// lifespan throughout the usage of the wrapper. Not intended for 
// cached usage. Not thread safe.
//
class StringDataPtr
{
public:
    StringDataPtr() : pStr(NULL), Size(0) {}
    StringDataPtr(const StringDataPtr& p)
        : pStr(p.pStr), Size(p.Size) {}
    StringDataPtr(const char* pstr, UPInt sz)
        : pStr(pstr), Size(sz) {}
    StringDataPtr(const char* pstr)
        : pStr(pstr), Size((pstr != NULL) ? OVR_strlen(pstr) : 0) {}
    explicit StringDataPtr(const String& str)
        : pStr(str.ToCStr()), Size(str.GetSize()) {}
    template <typename T, int N> 
    StringDataPtr(const T (&v)[N])
        : pStr(v), Size(N) {}

public:
    const char* ToCStr() const { return pStr; }
    UPInt       GetSize() const { return Size; }
    bool        IsEmpty() const { return GetSize() == 0; }

    // value is a prefix of this string
    // Character's values are not compared.
    bool        IsPrefix(const StringDataPtr& value) const
    {
        return ToCStr() == value.ToCStr() && GetSize() >= value.GetSize();
    }
    // value is a suffix of this string
    // Character's values are not compared.
    bool        IsSuffix(const StringDataPtr& value) const
    {
        return ToCStr() <= value.ToCStr() && (End()) == (value.End());
    }

    // Find first character.
    // init_ind - initial index.
    SPInt       FindChar(char c, UPInt init_ind = 0) const 
    {
        for (UPInt i = init_ind; i < GetSize(); ++i)
            if (pStr[i] == c)
                return static_cast<SPInt>(i);

        return -1; 
    }

    // Find last character.
    // init_ind - initial index.
    SPInt       FindLastChar(char c, UPInt init_ind = ~0) const 
    {
        if (init_ind == (UPInt)~0 || init_ind > GetSize())
            init_ind = GetSize();
        else
            ++init_ind;

        for (UPInt i = init_ind; i > 0; --i)
            if (pStr[i - 1] == c)
                return static_cast<SPInt>(i - 1);

        return -1; 
    }

    // Create new object and trim size bytes from the left.
    StringDataPtr  GetTrimLeft(UPInt size) const
    {
        // Limit trim size to the size of the string.
        size = Alg::PMin(GetSize(), size);

        return StringDataPtr(ToCStr() + size, GetSize() - size);
    }
    // Create new object and trim size bytes from the right.
    StringDataPtr  GetTrimRight(UPInt size) const
    {
        // Limit trim to the size of the string.
        size = Alg::PMin(GetSize(), size);

        return StringDataPtr(ToCStr(), GetSize() - size);
    }

    // Create new object, which contains next token.
    // Useful for parsing.
    StringDataPtr GetNextToken(char separator = ':') const
    {
        UPInt cur_pos = 0;
        const char* cur_str = ToCStr();

        for (; cur_pos < GetSize() && cur_str[cur_pos]; ++cur_pos)
        {
            if (cur_str[cur_pos] == separator)
            {
                break;
            }
        }

        return StringDataPtr(ToCStr(), cur_pos);
    }

    // Trim size bytes from the left.
    StringDataPtr& TrimLeft(UPInt size)
    {
        // Limit trim size to the size of the string.
        size = Alg::PMin(GetSize(), size);
        pStr += size;
        Size -= size;

        return *this;
    }
    // Trim size bytes from the right.
    StringDataPtr& TrimRight(UPInt size)
    {
        // Limit trim to the size of the string.
        size = Alg::PMin(GetSize(), size);
        Size -= size;

        return *this;
    }

    const char* Begin() const { return ToCStr(); }
    const char* End() const { return ToCStr() + GetSize(); }

    // Hash functor used string data pointers
    struct HashFunctor
    {    
        UPInt operator()(const StringDataPtr& data) const
        {
            return String::BernsteinHashFunction(data.ToCStr(), data.GetSize());
        }        
    };

    bool operator== (const StringDataPtr& data) const 
    {
        return (OVR_strncmp(pStr, data.pStr, data.Size) == 0);
    }

protected:
    const char* pStr;
    UPInt       Size;
};

} // OVR

#endif
