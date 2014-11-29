/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Std.h
Content     :   Standard C function interface
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Std_h
#define OVR_Std_h

#include "OVR_Types.h"
#include <stdarg.h> // for va_list args
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if !defined(OVR_OS_WINCE) && defined(OVR_CC_MSVC) && (OVR_CC_MSVC >= 1400)
#define OVR_MSVC_SAFESTRING
#include <errno.h>
#endif

// Wide-char funcs
#include <wchar.h>
#include <wctype.h>

namespace OVR {

#if defined(OVR_OS_WIN32)
inline char* OVR_CDECL OVR_itoa(int val, char *dest, UPInt destsize, int radix)
{
#if defined(OVR_MSVC_SAFESTRING)
    _itoa_s(val, dest, destsize, radix);
    return dest;
#else
    OVR_UNUSED(destsize);
    return itoa(val, dest, radix);
#endif
}
#else // OVR_OS_WIN32
inline char* OVR_itoa(int val, char* dest, unsigned int len, int radix)
{
    if (val == 0)
    {
        if (len > 1)
        {
            dest[0] = '0';
            dest[1] = '\0';  
        }
        return dest;
    }

    int cur = val;
    unsigned int i    = 0; 
    unsigned int sign = 0;

    if (val < 0)
    {
        val = -val;
        sign = 1;
    }

    while ((val != 0) && (i < (len - 1 - sign)))        
    {
        cur    = val % radix;
        val   /= radix;

        if (radix == 16)
        {
            switch(cur)
            {
            case 10:
                dest[i] = 'a';
                break;
            case 11:
                dest[i] = 'b';
                break;
            case 12:
                dest[i] = 'c';
                break;
            case 13:
                dest[i] = 'd';
                break;
            case 14:
                dest[i] = 'e';
                break;
            case 15:
                dest[i] = 'f';
                break;
            default:
                dest[i] = (char)('0' + cur);
                break;
            }
        } 
        else
        {
            dest[i] = (char)('0' + cur);
        }
        ++i;
    }

    if (sign)
    {
        dest[i++] = '-';
    }

    for (unsigned int j = 0; j < i / 2; ++j)
    {
        char tmp        = dest[j];
        dest[j]         = dest[i - 1 - j];
        dest[i - 1 - j] = tmp;
    }
    dest[i] = '\0';

    return dest;
}

#endif


// String functions

inline UPInt OVR_CDECL OVR_strlen(const char* str)
{
    return strlen(str);
}

inline char* OVR_CDECL OVR_strcpy(char* dest, UPInt destsize, const char* src)
{
#if defined(OVR_MSVC_SAFESTRING)
    strcpy_s(dest, destsize, src);
    return dest;
#else
    OVR_UNUSED(destsize);
    return strcpy(dest, src);
#endif
}

inline char* OVR_CDECL OVR_strncpy(char* dest, UPInt destsize, const char* src, UPInt count)
{
#if defined(OVR_MSVC_SAFESTRING)
    strncpy_s(dest, destsize, src, count);
    return dest;
#else
    OVR_UNUSED(destsize);
    return strncpy(dest, src, count);
#endif
}

inline char * OVR_CDECL OVR_strcat(char* dest, UPInt destsize, const char* src)
{
#if defined(OVR_MSVC_SAFESTRING)
    strcat_s(dest, destsize, src);
    return dest;
#else
    OVR_UNUSED(destsize);
    return strcat(dest, src);
#endif
}

inline int OVR_CDECL OVR_strcmp(const char* dest, const char* src)
{
    return strcmp(dest, src);
}

inline const char* OVR_CDECL OVR_strchr(const char* str, char c)
{
    return strchr(str, c);
}

inline char* OVR_CDECL OVR_strchr(char* str, char c)
{
    return strchr(str, c);
}

inline const char* OVR_strrchr(const char* str, char c)
{
    UPInt len = OVR_strlen(str);
    for (UPInt i=len; i>0; i--)     
        if (str[i]==c) 
            return str+i;
    return 0;
}

inline const UByte* OVR_CDECL OVR_memrchr(const UByte* str, UPInt size, UByte c)
{
    for (SPInt i = (SPInt)size - 1; i >= 0; i--)     
    {
        if (str[i] == c) 
            return str + i;
    }
    return 0;
}

inline char* OVR_CDECL OVR_strrchr(char* str, char c)
{
    UPInt len = OVR_strlen(str);
    for (UPInt i=len; i>0; i--)     
        if (str[i]==c) 
            return str+i;
    return 0;
}


double OVR_CDECL OVR_strtod(const char* string, char** tailptr);

inline long OVR_CDECL OVR_strtol(const char* string, char** tailptr, int radix)
{
    return strtol(string, tailptr, radix);
}

inline long OVR_CDECL OVR_strtoul(const char* string, char** tailptr, int radix)
{
    return strtoul(string, tailptr, radix);
}

inline int OVR_CDECL OVR_strncmp(const char* ws1, const char* ws2, UPInt size)
{
    return strncmp(ws1, ws2, size);
}

inline UInt64 OVR_CDECL OVR_strtouq(const char *nptr, char **endptr, int base)
{
#if defined(OVR_CC_MSVC) && !defined(OVR_OS_WINCE)
    return _strtoui64(nptr, endptr, base);
#else
    return strtoull(nptr, endptr, base);
#endif
}

inline SInt64 OVR_CDECL OVR_strtoq(const char *nptr, char **endptr, int base)
{
#if defined(OVR_CC_MSVC) && !defined(OVR_OS_WINCE)
    return _strtoi64(nptr, endptr, base);
#else
    return strtoll(nptr, endptr, base);
#endif
}


inline SInt64 OVR_CDECL OVR_atoq(const char* string)
{
#if defined(OVR_CC_MSVC) && !defined(OVR_OS_WINCE)
    return _atoi64(string);
#else
    return atoll(string);
#endif
}

inline UInt64 OVR_CDECL OVR_atouq(const char* string)
{
  return OVR_strtouq(string, NULL, 10);
}


// Implemented in GStd.cpp in platform-specific manner.
int OVR_CDECL OVR_stricmp(const char* dest, const char* src);
int OVR_CDECL OVR_strnicmp(const char* dest, const char* src, UPInt count);

inline UPInt OVR_CDECL OVR_sprintf(char *dest, UPInt destsize, const char* format, ...)
{
    va_list argList;
    va_start(argList,format);
    UPInt ret;
#if defined(OVR_CC_MSVC)
    #if defined(OVR_MSVC_SAFESTRING)
        ret = _vsnprintf_s(dest, destsize, _TRUNCATE, format, argList);
        OVR_ASSERT(ret != -1);
    #else
        OVR_UNUSED(destsize);
        ret = _vsnprintf(dest, destsize - 1, format, argList); // -1 for space for the null character
        OVR_ASSERT(ret != -1);
        dest[destsize-1] = 0;
    #endif
#else
    OVR_UNUSED(destsize);
    ret = vsprintf(dest, format, argList);
    OVR_ASSERT(ret < destsize);
#endif
    va_end(argList);
    return ret;
}

inline UPInt OVR_CDECL OVR_vsprintf(char *dest, UPInt destsize, const char * format, va_list argList)
{
    UPInt ret;
#if defined(OVR_CC_MSVC)
    #if defined(OVR_MSVC_SAFESTRING)
        dest[0] = '\0';
        int rv = vsnprintf_s(dest, destsize, _TRUNCATE, format, argList);
        if (rv == -1)
        {
            dest[destsize - 1] = '\0';
            ret = destsize - 1;
        }
        else
            ret = (UPInt)rv;
    #else
        OVR_UNUSED(destsize);
        int rv = _vsnprintf(dest, destsize - 1, format, argList);
        OVR_ASSERT(rv != -1);
        ret = (UPInt)rv;
        dest[destsize-1] = 0;
    #endif
#else
    OVR_UNUSED(destsize);
    ret = (UPInt)vsprintf(dest, format, argList);
    OVR_ASSERT(ret < destsize);
#endif
    return ret;
}

// Returns the number of characters in the formatted string.
inline UPInt OVR_CDECL OVR_vscprintf(const char * format, va_list argList)
{
    UPInt ret;
#if defined(OVR_CC_MSVC)
    ret = (UPInt) _vscprintf(format, argList);
#else    
    ret = (UPInt) vsnprintf(NULL, 0, format, argList);
#endif
    return ret;       
}


wchar_t* OVR_CDECL OVR_wcscpy(wchar_t* dest, UPInt destsize, const wchar_t* src);
wchar_t* OVR_CDECL OVR_wcsncpy(wchar_t* dest, UPInt destsize, const wchar_t* src, UPInt count);
wchar_t* OVR_CDECL OVR_wcscat(wchar_t* dest, UPInt destsize, const wchar_t* src);
UPInt    OVR_CDECL OVR_wcslen(const wchar_t* str);
int      OVR_CDECL OVR_wcscmp(const wchar_t* a, const wchar_t* b);
int      OVR_CDECL OVR_wcsicmp(const wchar_t* a, const wchar_t* b);

inline int OVR_CDECL OVR_wcsicoll(const wchar_t* a, const wchar_t* b)
{
#if defined(OVR_OS_WIN32)
#if defined(OVR_CC_MSVC) && (OVR_CC_MSVC >= 1400)
    return ::_wcsicoll(a, b);
#else
    return ::wcsicoll(a, b);
#endif
#else
    // not supported, use regular wcsicmp
    return OVR_wcsicmp(a, b);
#endif
}

inline int OVR_CDECL OVR_wcscoll(const wchar_t* a, const wchar_t* b)
{
#if defined(OVR_OS_WIN32) || defined(OVR_OS_LINUX)
    return wcscoll(a, b);
#else
    // not supported, use regular wcscmp
    return OVR_wcscmp(a, b);
#endif
}

#ifndef OVR_NO_WCTYPE

inline int OVR_CDECL UnicodeCharIs(const UInt16* table, wchar_t charCode)
{
    unsigned offset = table[charCode >> 8];
    if (offset == 0) return 0;
    if (offset == 1) return 1;
    return (table[offset + ((charCode >> 4) & 15)] & (1 << (charCode & 15))) != 0;
}

extern const UInt16 UnicodeAlnumBits[];
extern const UInt16 UnicodeAlphaBits[];
extern const UInt16 UnicodeDigitBits[];
extern const UInt16 UnicodeSpaceBits[];
extern const UInt16 UnicodeXDigitBits[];

// Uncomment if necessary
//extern const UInt16 UnicodeCntrlBits[];
//extern const UInt16 UnicodeGraphBits[];
//extern const UInt16 UnicodeLowerBits[];
//extern const UInt16 UnicodePrintBits[];
//extern const UInt16 UnicodePunctBits[];
//extern const UInt16 UnicodeUpperBits[];

inline int OVR_CDECL OVR_iswalnum (wchar_t charCode) { return UnicodeCharIs(UnicodeAlnumBits,  charCode); }
inline int OVR_CDECL OVR_iswalpha (wchar_t charCode) { return UnicodeCharIs(UnicodeAlphaBits,  charCode); }
inline int OVR_CDECL OVR_iswdigit (wchar_t charCode) { return UnicodeCharIs(UnicodeDigitBits,  charCode); }
inline int OVR_CDECL OVR_iswspace (wchar_t charCode) { return UnicodeCharIs(UnicodeSpaceBits,  charCode); }
inline int OVR_CDECL OVR_iswxdigit(wchar_t charCode) { return UnicodeCharIs(UnicodeXDigitBits, charCode); }

// Uncomment if necessary
//inline int OVR_CDECL OVR_iswcntrl (wchar_t charCode) { return UnicodeCharIs(UnicodeCntrlBits,  charCode); }
//inline int OVR_CDECL OVR_iswgraph (wchar_t charCode) { return UnicodeCharIs(UnicodeGraphBits,  charCode); }
//inline int OVR_CDECL OVR_iswlower (wchar_t charCode) { return UnicodeCharIs(UnicodeLowerBits,  charCode); }
//inline int OVR_CDECL OVR_iswprint (wchar_t charCode) { return UnicodeCharIs(UnicodePrintBits,  charCode); }
//inline int OVR_CDECL OVR_iswpunct (wchar_t charCode) { return UnicodeCharIs(UnicodePunctBits,  charCode); }
//inline int OVR_CDECL OVR_iswupper (wchar_t charCode) { return UnicodeCharIs(UnicodeUpperBits,  charCode); }

int OVR_CDECL OVR_towupper(wchar_t charCode);
int OVR_CDECL OVR_towlower(wchar_t charCode);

#else // OVR_NO_WCTYPE

inline int OVR_CDECL OVR_iswspace(wchar_t c)
{
    return iswspace(c);
}

inline int OVR_CDECL OVR_iswdigit(wchar_t c)
{
    return iswdigit(c);
}

inline int OVR_CDECL OVR_iswxdigit(wchar_t c)
{
    return iswxdigit(c);
}

inline int OVR_CDECL OVR_iswalpha(wchar_t c)
{
    return iswalpha(c);
}

inline int OVR_CDECL OVR_iswalnum(wchar_t c)
{
    return iswalnum(c);
}

inline wchar_t OVR_CDECL OVR_towlower(wchar_t c)
{
    return (wchar_t)towlower(c);
}

inline wchar_t OVR_towupper(wchar_t c)
{
    return (wchar_t)towupper(c);
}

#endif // OVR_NO_WCTYPE

// ASCII versions of tolower and toupper. Don't use "char"
inline int OVR_CDECL OVR_tolower(int c)
{
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

inline int OVR_CDECL OVR_toupper(int c)
{
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}



inline double OVR_CDECL OVR_wcstod(const wchar_t* string, wchar_t** tailptr)
{
#if defined(OVR_OS_OTHER)
    OVR_UNUSED(tailptr);
    char buffer[64];
    char* tp = NULL;
    UPInt max = OVR_wcslen(string);
    if (max > 63) max = 63;
    unsigned char c = 0;
    for (UPInt i=0; i < max; i++)
    {
        c = (unsigned char)string[i];
        buffer[i] = ((c) < 128 ? (char)c : '!');
    }
    buffer[max] = 0;
    return OVR_strtod(buffer, &tp);
#else
    return wcstod(string, tailptr);
#endif
}

inline long OVR_CDECL OVR_wcstol(const wchar_t* string, wchar_t** tailptr, int radix)
{
#if defined(OVR_OS_OTHER)
    OVR_UNUSED(tailptr);
    char buffer[64];
    char* tp = NULL;
    UPInt max = OVR_wcslen(string);
    if (max > 63) max = 63;
    unsigned char c = 0;
    for (UPInt i=0; i < max; i++)
    {
        c = (unsigned char)string[i];
        buffer[i] = ((c) < 128 ? (char)c : '!');
    }
    buffer[max] = 0;
    return strtol(buffer, &tp, radix);
#else
    return wcstol(string, tailptr, radix);
#endif
}

} // OVR

#endif // OVR_Std_h
