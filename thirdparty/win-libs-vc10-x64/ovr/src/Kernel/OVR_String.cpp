/************************************************************************************

Filename    :   OVR_String.cpp
Content     :   String UTF8 string implementation with copy-on-write semantics
                (thread-safe for assignment but not modification).
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_String.h"

#include <stdlib.h>
#include <ctype.h>

#ifdef OVR_OS_QNX
# include <strings.h>
#endif

namespace OVR {

#define String_LengthIsSize (UPInt(1) << String::Flag_LengthIsSizeShift)

String::DataDesc String::NullData = {String_LengthIsSize, 1, {0} };


String::String()
{
    pData = &NullData;
    pData->AddRef();
};

String::String(const char* pdata)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size = pdata ? OVR_strlen(pdata) : 0; 
    pData = AllocDataCopy1(size, 0, pdata, size);
};

String::String(const char* pdata1, const char* pdata2, const char* pdata3)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size1 = pdata1 ? OVR_strlen(pdata1) : 0; 
    UPInt size2 = pdata2 ? OVR_strlen(pdata2) : 0; 
    UPInt size3 = pdata3 ? OVR_strlen(pdata3) : 0; 

    DataDesc *pdataDesc = AllocDataCopy2(size1 + size2 + size3, 0,
                                         pdata1, size1, pdata2, size2);
    memcpy(pdataDesc->Data + size1 + size2, pdata3, size3);   
    pData = pdataDesc;    
}

String::String(const char* pdata, UPInt size)
{
    OVR_ASSERT((size == 0) || (pdata != 0));
    pData = AllocDataCopy1(size, 0, pdata, size);
};


String::String(const InitStruct& src, UPInt size)
{
    pData = AllocData(size, 0);
    src.InitString(GetData()->Data, size);
}

String::String(const String& src)
{    
    pData = src.GetData();
    pData->AddRef();
}

String::String(const StringBuffer& src)
{
    pData = AllocDataCopy1(src.GetSize(), 0, src.ToCStr(), src.GetSize());
}

String::String(const wchar_t* data)
{
    pData = &NullData;
    pData->AddRef();
    // Simplified logic for wchar_t constructor.
    if (data)    
        *this = data;    
}


String::DataDesc* String::AllocData(UPInt size, UPInt lengthIsSize)
{
    String::DataDesc* pdesc;

    if (size == 0)
    {
        pdesc = &NullData;
        pdesc->AddRef();
        return pdesc;
    }

    pdesc = (DataDesc*)OVR_ALLOC(sizeof(DataDesc)+ size);
    pdesc->Data[size] = 0;
    pdesc->RefCount = 1;
    pdesc->Size     = size | lengthIsSize;  
    return pdesc;
}


String::DataDesc* String::AllocDataCopy1(UPInt size, UPInt lengthIsSize,
                                         const char* pdata, UPInt copySize)
{
    String::DataDesc* pdesc = AllocData(size, lengthIsSize);
    memcpy(pdesc->Data, pdata, copySize);
    return pdesc;
}

String::DataDesc* String::AllocDataCopy2(UPInt size, UPInt lengthIsSize,
                                         const char* pdata1, UPInt copySize1,
                                         const char* pdata2, UPInt copySize2)
{
    String::DataDesc* pdesc = AllocData(size, lengthIsSize);
    memcpy(pdesc->Data, pdata1, copySize1);
    memcpy(pdesc->Data + copySize1, pdata2, copySize2);
    return pdesc;
}


UPInt String::GetLength() const 
{
    // Optimize length accesses for non-UTF8 character strings. 
    DataDesc* pdata = GetData();
    UPInt     length, size = pdata->GetSize();
    
    if (pdata->LengthIsSize())
        return size;    
    
    length = (UPInt)UTF8Util::GetLength(pdata->Data, (UPInt)size);
    
    if (length == size)
        pdata->Size |= String_LengthIsSize;
    
    return length;
}


//static UInt32 String_CharSearch(const char* buf, )


UInt32 String::GetCharAt(UPInt index) const 
{  
    SPInt       i = (SPInt) index;
    DataDesc*   pdata = GetData();
    const char* buf = pdata->Data;
    UInt32      c;
    
    if (pdata->LengthIsSize())
    {
        OVR_ASSERT(index < pdata->GetSize());
        buf += i;
        return UTF8Util::DecodeNextChar_Advance0(&buf);
    }

    c = UTF8Util::GetCharAt(index, buf, pdata->GetSize());
    return c;
}

UInt32 String::GetFirstCharAt(UPInt index, const char** offset) const
{
    DataDesc*   pdata = GetData();
    SPInt       i = (SPInt) index;
    const char* buf = pdata->Data;
    const char* end = buf + pdata->GetSize();
    UInt32      c;

    do 
    {
        c = UTF8Util::DecodeNextChar_Advance0(&buf);
        i--;

        if (buf >= end)
        {
            // We've hit the end of the string; don't go further.
            OVR_ASSERT(i == 0);
            return c;
        }
    } while (i >= 0);

    *offset = buf;

    return c;
}

UInt32 String::GetNextChar(const char** offset) const
{
    return UTF8Util::DecodeNextChar(offset);
}



void String::AppendChar(UInt32 ch)
{
    DataDesc*   pdata = GetData();
    UPInt       size = pdata->GetSize();
    char        buff[8];
    SPInt       encodeSize = 0;

    // Converts ch into UTF8 string and fills it into buff.   
    UTF8Util::EncodeChar(buff, &encodeSize, ch);
    OVR_ASSERT(encodeSize >= 0);

    SetData(AllocDataCopy2(size + (UPInt)encodeSize, 0,
                           pdata->Data, size, buff, (UPInt)encodeSize));
    pdata->Release();
}


void String::AppendString(const wchar_t* pstr, SPInt len)
{
    if (!pstr)
        return;

    DataDesc*   pdata = GetData();
    UPInt       oldSize = pdata->GetSize();    
    UPInt       encodeSize = (UPInt)UTF8Util::GetEncodeStringSize(pstr, len);

    DataDesc*   pnewData = AllocDataCopy1(oldSize + (UPInt)encodeSize, 0,
                                          pdata->Data, oldSize);
    UTF8Util::EncodeString(pnewData->Data + oldSize,  pstr, len);

    SetData(pnewData);
    pdata->Release();
}


void String::AppendString(const char* putf8str, SPInt utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == -1)
        utf8StrSz = (SPInt)OVR_strlen(putf8str);

    DataDesc*   pdata = GetData();
    UPInt       oldSize = pdata->GetSize();

    SetData(AllocDataCopy2(oldSize + (UPInt)utf8StrSz, 0,
                           pdata->Data, oldSize, putf8str, (UPInt)utf8StrSz));
    pdata->Release();
}

void    String::AssignString(const InitStruct& src, UPInt size)
{
    DataDesc*   poldData = GetData();
    DataDesc*   pnewData = AllocData(size, 0);
    src.InitString(pnewData->Data, size);
    SetData(pnewData);
    poldData->Release();
}

void    String::AssignString(const char* putf8str, UPInt size)
{
    DataDesc* poldData = GetData();
    SetData(AllocDataCopy1(size, 0, putf8str, size));
    poldData->Release();
}

void    String::operator = (const char* pstr)
{
    AssignString(pstr, pstr ? OVR_strlen(pstr) : 0);
}

void    String::operator = (const wchar_t* pwstr)
{
    DataDesc*   poldData = GetData();
    UPInt       size = pwstr ? (UPInt)UTF8Util::GetEncodeStringSize(pwstr) : 0;

    DataDesc*   pnewData = AllocData(size, 0);
    UTF8Util::EncodeString(pnewData->Data, pwstr);
    SetData(pnewData);
    poldData->Release();
}


void    String::operator = (const String& src)
{     
    DataDesc*    psdata = src.GetData();
    DataDesc*    pdata = GetData();    

    SetData(psdata);
    psdata->AddRef();
    pdata->Release();
}


void    String::operator = (const StringBuffer& src)
{ 
    DataDesc* polddata = GetData();    
    SetData(AllocDataCopy1(src.GetSize(), 0, src.ToCStr(), src.GetSize()));
    polddata->Release();
}

void    String::operator += (const String& src)
{
    DataDesc   *pourData = GetData(),
               *psrcData = src.GetData();
    UPInt       ourSize  = pourData->GetSize(),
                srcSize  = psrcData->GetSize();
    UPInt       lflag    = pourData->GetLengthFlag() & psrcData->GetLengthFlag();

    SetData(AllocDataCopy2(ourSize + srcSize, lflag,
                           pourData->Data, ourSize, psrcData->Data, srcSize));
    pourData->Release();
}


String   String::operator + (const char* str) const
{   
    String tmp1(*this);
    tmp1 += (str ? str : "");
    return tmp1;
}

String   String::operator + (const String& src) const
{ 
    String tmp1(*this);
    tmp1 += src;
    return tmp1;
}

void    String::Remove(UPInt posAt, SPInt removeLength)
{
    DataDesc*   pdata = GetData();
    UPInt       oldSize = pdata->GetSize();    
    // Length indicates the number of characters to remove. 
    UPInt       length = GetLength();

    // If index is past the string, nothing to remove.
    if (posAt >= length)
        return;
    // Otherwise, cap removeLength to the length of the string.
    if ((posAt + removeLength) > length)
        removeLength = length - posAt;

    // Get the byte position of the UTF8 char at position posAt.
    SPInt bytePos    = UTF8Util::GetByteIndex(posAt, pdata->Data, oldSize);
    SPInt removeSize = UTF8Util::GetByteIndex(removeLength, pdata->Data + bytePos, oldSize-bytePos);

    SetData(AllocDataCopy2(oldSize - removeSize, pdata->GetLengthFlag(),
                           pdata->Data, bytePos,
                           pData->Data + bytePos + removeSize, (oldSize - bytePos - removeSize)));
    pdata->Release();
}


String   String::Substring(UPInt start, UPInt end) const
{
    UPInt length = GetLength();
    if ((start >= length) || (start >= end))
        return String();   

    DataDesc* pdata = GetData();
    
    // If size matches, we know the exact index range.
    if (pdata->LengthIsSize())
        return String(pdata->Data + start, end - start);
    
    // Get position of starting character.
    SPInt byteStart = UTF8Util::GetByteIndex(start, pdata->Data, pdata->GetSize());
    SPInt byteSize  = UTF8Util::GetByteIndex(end - start, pdata->Data + byteStart, pdata->GetSize()-byteStart);
    return String(pdata->Data + byteStart, (UPInt)byteSize);
}

void String::Clear()
{   
    NullData.AddRef();
    GetData()->Release();
    SetData(&NullData);
}


String   String::ToUpper() const 
{       
    UInt32      c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    SPInt       bufferOffset = 0;
    char        buffer[512];
    
    while(psource < pend)
    {
        do {            
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeChar(buffer, &bufferOffset, OVR_towupper(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < SPInt(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}

String   String::ToLower() const 
{
    UInt32      c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    SPInt       bufferOffset = 0;
    char        buffer[512];

    while(psource < pend)
    {
        do {
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeChar(buffer, &bufferOffset, OVR_towlower(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < SPInt(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}



String& String::Insert(const char* substr, UPInt posAt, SPInt strSize)
{
    DataDesc* poldData   = GetData();
    UPInt     oldSize    = poldData->GetSize();
    UPInt     insertSize = (strSize < 0) ? OVR_strlen(substr) : (UPInt)strSize;    
    UPInt     byteIndex  =  (poldData->LengthIsSize()) ?
                            posAt : (UPInt)UTF8Util::GetByteIndex(posAt, poldData->Data, oldSize);

    OVR_ASSERT(byteIndex <= oldSize);
    
    DataDesc* pnewData = AllocDataCopy2(oldSize + insertSize, 0,
                                        poldData->Data, byteIndex, substr, insertSize);
    memcpy(pnewData->Data + byteIndex + insertSize,
           poldData->Data + byteIndex, oldSize - byteIndex);
    SetData(pnewData);
    poldData->Release();
    return *this;
}

/*
String& String::Insert(const UInt32* substr, UPInt posAt, SPInt len)
{
    for (SPInt i = 0; i < len; ++i)
    {
        UPInt charw = InsertCharAt(substr[i], posAt);
        posAt += charw;
    }
    return *this;
}
*/

UPInt String::InsertCharAt(UInt32 c, UPInt posAt)
{
    char    buf[8];
    SPInt   index = 0;
    UTF8Util::EncodeChar(buf, &index, c);
    OVR_ASSERT(index >= 0);
    buf[(UPInt)index] = 0;

    Insert(buf, posAt, index);
    return (UPInt)index;
}


int String::CompareNoCase(const char* a, const char* b)
{
    return OVR_stricmp(a, b);
}

int String::CompareNoCase(const char* a, const char* b, SPInt len)
{
    if (len)
    {
        SPInt f,l;
        SPInt slen = len;
        const char *s = b;
        do {
            f = (SPInt)OVR_tolower((int)(*(a++)));
            l = (SPInt)OVR_tolower((int)(*(b++)));
        } while (--len && f && (f == l) && *b != 0);

        if (f == l && (len != 0 || *b != 0))
        {
            f = (SPInt)slen;
            l = (SPInt)OVR_strlen(s);
            return int(f - l);
        }

        return int(f - l);
    }
    else
        return (0-(int)OVR_strlen(b));
}

// ***** Implement hash static functions

// Hash function
UPInt String::BernsteinHashFunction(const void* pdataIn, UPInt size, UPInt seed)
{
    const UByte*    pdata   = (const UByte*) pdataIn;
    UPInt           h       = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ (unsigned) pdata[size];
    }

    return h;
}

// Hash function, case-insensitive
UPInt String::BernsteinHashFunctionCIS(const void* pdataIn, UPInt size, UPInt seed)
{
    const UByte*    pdata = (const UByte*) pdataIn;
    UPInt           h = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ OVR_tolower(pdata[size]);
    }

    // Alternative: "sdbm" hash function, suggested at same web page above.
    // h = 0;
    // for bytes { h = (h << 16) + (h << 6) - hash + *p; }
    return h;
}



// ***** String Buffer used for Building Strings


#define OVR_SBUFF_DEFAULT_GROW_SIZE 512
// Constructors / Destructor.
StringBuffer::StringBuffer()
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
}

StringBuffer::StringBuffer(UPInt growSize)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    SetGrowSize(growSize);
}

StringBuffer::StringBuffer(const char* data)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    *this = data;
}

StringBuffer::StringBuffer(const char* data, UPInt dataSize)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(data, dataSize);
}

StringBuffer::StringBuffer(const String& src)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const StringBuffer& src)
    : pData(NULL), Size(0), BufferSize(src.GetGrowSize()), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(src.ToCStr(), src.GetSize());
    LengthIsSize = src.LengthIsSize;
}

StringBuffer::StringBuffer(const wchar_t* data)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    *this = data;
}

StringBuffer::~StringBuffer()
{
    if (pData)
        OVR_FREE(pData);
}
void StringBuffer::SetGrowSize(UPInt growSize) 
{ 
    if (growSize <= 16)
        GrowSize = 16;
    else
    {
        UByte bits = Alg::UpperBit(UInt32(growSize-1));
        UPInt size = 1<<bits;
        GrowSize = size == growSize ? growSize : size;
    }
}

UPInt StringBuffer::GetLength() const
{
    UPInt length, size = GetSize();
    if (LengthIsSize)
        return size;

    length = (UPInt)UTF8Util::GetLength(pData, (UPInt)GetSize());

    if (length == GetSize())
        LengthIsSize = true;
    return length;
}

void    StringBuffer::Reserve(UPInt _size)
{
    if (_size >= BufferSize) // >= because of trailing zero! (!AB)
    {
        BufferSize = (_size + 1 + GrowSize - 1)& ~(GrowSize-1);
        if (!pData)
            pData = (char*)OVR_ALLOC(BufferSize);
        else 
            pData = (char*)OVR_REALLOC(pData, BufferSize);
    }
}
void    StringBuffer::Resize(UPInt _size)
{
    Reserve(_size);
    LengthIsSize = false;
    Size = _size;
    if (pData)
        pData[Size] = 0;
}

void StringBuffer::Clear()
{
    Resize(0);
    /*
    if (pData != pEmptyNullData)
    {
        OVR_FREE(pHeap, pData);
        pData = pEmptyNullData;
        Size = BufferSize = 0;
        LengthIsSize = false;
    }
    */
}
// Appends a character
void     StringBuffer::AppendChar(UInt32 ch)
{
    char    buff[8];
    UPInt   origSize = GetSize();

    // Converts ch into UTF8 string and fills it into buff. Also increments index according to the number of bytes
    // in the UTF8 string.
    SPInt   srcSize = 0;
    UTF8Util::EncodeChar(buff, &srcSize, ch);
    OVR_ASSERT(srcSize >= 0);
    
    UPInt size = origSize + srcSize;
    Resize(size);
    memcpy(pData + origSize, buff, srcSize);
}

// Append a string
void     StringBuffer::AppendString(const wchar_t* pstr, SPInt len)
{
    if (!pstr)
        return;

    SPInt   srcSize     = UTF8Util::GetEncodeStringSize(pstr, len);
    UPInt   origSize    = GetSize();
    UPInt   size        = srcSize + origSize;

    Resize(size);
    UTF8Util::EncodeString(pData + origSize,  pstr, len);
}

void      StringBuffer::AppendString(const char* putf8str, SPInt utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == -1)
        utf8StrSz = (SPInt)OVR_strlen(putf8str);

    UPInt   origSize    = GetSize();
    UPInt   size        = utf8StrSz + origSize;

    Resize(size);
    memcpy(pData + origSize, putf8str, utf8StrSz);
}


void      StringBuffer::operator = (const char* pstr)
{
    pstr = pstr ? pstr : "";
    UPInt size = OVR_strlen(pstr);
    Resize(size);
    memcpy(pData, pstr, size);
}

void      StringBuffer::operator = (const wchar_t* pstr)
{
    pstr = pstr ? pstr : L"";
    UPInt size = (UPInt)UTF8Util::GetEncodeStringSize(pstr);
    Resize(size);
    UTF8Util::EncodeString(pData, pstr);
}

void      StringBuffer::operator = (const String& src)
{
    Resize(src.GetSize());
    memcpy(pData, src.ToCStr(), src.GetSize());
}


// Inserts substr at posAt
void      StringBuffer::Insert(const char* substr, UPInt posAt, SPInt len)
{
    UPInt     oldSize    = Size;
    UPInt     insertSize = (len < 0) ? OVR_strlen(substr) : (UPInt)len;    
    UPInt     byteIndex  = LengthIsSize ? posAt : 
                           (UPInt)UTF8Util::GetByteIndex(posAt, pData, (SPInt)Size);

    OVR_ASSERT(byteIndex <= oldSize);
    Reserve(oldSize + insertSize);

    memmove(pData + byteIndex + insertSize, pData + byteIndex, oldSize - byteIndex + 1);
    memcpy (pData + byteIndex, substr, insertSize);
    LengthIsSize = false;
    Size = oldSize + insertSize;
    pData[Size] = 0;
}

// Inserts character at posAt
UPInt     StringBuffer::InsertCharAt(UInt32 c, UPInt posAt)
{
    char    buf[8];
    SPInt   len = 0;
    UTF8Util::EncodeChar(buf, &len, c);
    OVR_ASSERT(len >= 0);
    buf[(UPInt)len] = 0;

    Insert(buf, posAt, len);
    return (UPInt)len;
}

} // OVR
