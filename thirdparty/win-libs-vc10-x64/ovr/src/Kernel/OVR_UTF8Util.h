/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_UTF8Util.h
Content     :   UTF8 Unicode character encoding/decoding support
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_UTF8Util_h
#define OVR_UTF8Util_h

#include "OVR_Types.h"

namespace OVR { namespace UTF8Util {

//-----------------------------------------------------------------------------------

// *** UTF8 string length and indexing.

// Determines the length of UTF8 string in characters.
// If source length is specified (in bytes), null 0 character is counted properly.
SPInt    OVR_STDCALL GetLength(const char* putf8str, SPInt length = -1);

// Gets a decoded UTF8 character at index; you can access up to the index returned
// by GetLength. 0 will be returned for out of bounds access.
UInt32   OVR_STDCALL GetCharAt(SPInt index, const char* putf8str, SPInt length = -1);

// Converts UTF8 character index into byte offset.
// -1 is returned if index was out of bounds.
SPInt    OVR_STDCALL GetByteIndex(SPInt index, const char* putf8str, SPInt length = -1);


// *** 16-bit Unicode string Encoding/Decoding routines.

// Determines the number of bytes necessary to encode a string.
// Does not count the terminating 0 (null) character.
SPInt    OVR_STDCALL GetEncodeStringSize(const wchar_t* pchar, SPInt length = -1);

// Encodes a unicode (UCS-2 only) string into a buffer. The size of buffer must be at
// least GetEncodeStringSize() + 1.
void     OVR_STDCALL EncodeString(char *pbuff, const wchar_t* pchar, SPInt length = -1);

// Decode UTF8 into a wchar_t buffer. Must have GetLength()+1 characters available.
// Characters over 0xFFFF are replaced with 0xFFFD.
// Returns the length of resulting string (number of characters)
UPInt    OVR_STDCALL DecodeString(wchar_t *pbuff, const char* putf8str, SPInt bytesLen = -1);


// *** Individual character Encoding/Decoding.

// Determined the number of bytes necessary to encode a UCS character.
int      OVR_STDCALL GetEncodeCharSize(UInt32 ucsCharacter);

// Encodes the given UCS character into the given UTF-8 buffer.
// Writes the data starting at buffer[offset], and 
// increments offset by the number of bytes written.
// May write up to 6 bytes, so make sure there's room in the buffer
void     OVR_STDCALL EncodeChar(char* pbuffer, SPInt* poffset, UInt32 ucsCharacter);

// Return the next Unicode character in the UTF-8 encoded buffer.
// Invalid UTF-8 sequences produce a U+FFFD character as output.
// Advances *utf8_buffer past the character returned. Pointer advance
// occurs even if the terminating 0 character is hit, since that allows
// strings with middle '\0' characters to be supported.
UInt32   OVR_STDCALL DecodeNextChar_Advance0(const char** putf8Buffer);

// Safer version of DecodeNextChar, which doesn't advance pointer if
// null character is hit.
inline UInt32 DecodeNextChar(const char** putf8Buffer)
{
    UInt32 ch = DecodeNextChar_Advance0(putf8Buffer);
    if (ch == 0)
        (*putf8Buffer)--;
    return ch;
}


}} // OVR::UTF8Util

#endif
