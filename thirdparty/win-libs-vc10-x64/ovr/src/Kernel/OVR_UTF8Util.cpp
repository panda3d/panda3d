/**************************************************************************

Filename    :   OVR_UTF8Util.cpp
Content     :   UTF8 Unicode character encoding/decoding support
Created     :   September 19, 2012
Notes       : 
Notes       :   Much useful info at "UTF-8 and Unicode FAQ"
                http://www.cl.cam.ac.uk/~mgk25/unicode.html

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_UTF8Util.h"

namespace OVR { namespace UTF8Util {

SPInt OVR_STDCALL GetLength(const char* buf, SPInt buflen)
{
    const char* p = buf;
    SPInt length = 0;

    if (buflen != -1)
    {
        while (p - buf < buflen)
        {
            // We should be able to have ASStrings with 0 in the middle.
            UTF8Util::DecodeNextChar_Advance0(&p);
            length++;
        }
    }
    else
    {
        while (UTF8Util::DecodeNextChar_Advance0(&p))
            length++;
    }
    
    return length;
}

UInt32 OVR_STDCALL GetCharAt(SPInt index, const char* putf8str, SPInt length)
{
    const char* buf = putf8str;
    UInt32  c = 0;

    if (length != -1)
    {
        while (buf - putf8str < length)
        {           
            c = UTF8Util::DecodeNextChar_Advance0(&buf);
            if (index == 0)
                return c;
            index--;
        }

        return c;
    }

    do 
    {
        c = UTF8Util::DecodeNextChar_Advance0(&buf);
        index--;

        if (c == 0)
        {
            // We've hit the end of the string; don't go further.
            OVR_ASSERT(index == 0);
            return c;
        }
    } while (index >= 0);

    return c;
}

SPInt OVR_STDCALL GetByteIndex(SPInt index, const char *putf8str, SPInt length)
{
    const char* buf = putf8str;

    if (length != -1)
    {
        while ((buf - putf8str) < length && index > 0)
        {
            UTF8Util::DecodeNextChar_Advance0(&buf);
            index--;
        }

        return buf-putf8str;
    }

    while (index > 0) 
    {
        UInt32 c = UTF8Util::DecodeNextChar_Advance0(&buf);
        index--;

        if (c == 0)
            return buf-putf8str;
    };

    return buf-putf8str;
}

int OVR_STDCALL GetEncodeCharSize(UInt32 ucs_character)
{
    if (ucs_character <= 0x7F)
        return 1;
    else if (ucs_character <= 0x7FF)
        return 2;
    else if (ucs_character <= 0xFFFF)
        return 3;
    else if (ucs_character <= 0x1FFFFF)
        return 4;
    else if (ucs_character <= 0x3FFFFFF)
        return 5;
    else if (ucs_character <= 0x7FFFFFFF)
        return 6;
    else
        return 0;
}

UInt32 OVR_STDCALL DecodeNextChar_Advance0(const char** putf8Buffer)
{
    UInt32  uc;
    char    c;
    
    // Security considerations:
    //
    // Changed, this is now only the case for DecodeNextChar:
    //  - If we hit a zero byte, we want to return 0 without stepping
    //    the buffer pointer past the 0. th
    //
    // If we hit an "overlong sequence"; i.e. a character encoded
    // in a longer multibyte string than is necessary, then we
    // need to discard the character.  This is so attackers can't
    // disguise dangerous characters or character sequences --
    // there is only one valid encoding for each character.
    //
    // If we decode characters { 0xD800 .. 0xDFFF } or { 0xFFFE,
    // 0xFFFF } then we ignore them; they are not valid in UTF-8.
    
    // This isn't actually an invalid character; it's a valid char that
    // looks like an inverted question mark.
#define INVALID_CHAR 0x0FFFD
    
#define FIRST_BYTE(mask, shift)     \
    uc = (c & (mask)) << (shift);
    
#define NEXT_BYTE(shift) \
    c = **putf8Buffer;   \
    if (c == 0) return 0; /* end of buffer, do not advance */   \
    if ((c & 0xC0) != 0x80) return INVALID_CHAR; /* standard check */  \
    (*putf8Buffer)++;    \
    uc |= (c & 0x3F) << shift;
    
    c = **putf8Buffer;
    (*putf8Buffer)++;
    if (c == 0)
        return 0;   // End of buffer.
    
    if ((c & 0x80) == 0) return (UInt32) c; // Conventional 7-bit ASCII.
    
    // Multi-byte sequences.
    if ((c & 0xE0) == 0xC0)
    {
        // Two-byte sequence.
        FIRST_BYTE(0x1F, 6);
        NEXT_BYTE(0);
        if (uc < 0x80) return INVALID_CHAR;  // overlong
        return uc;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        // Three-byte sequence.
        FIRST_BYTE(0x0F, 12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x800) return INVALID_CHAR; // overlong
        // Not valid ISO 10646, but Flash requires these to work
        // see AS3 test e15_5_3_2_3 for String.fromCharCode().charCodeAt(0)
        // if (uc >= 0x0D800 && uc <= 0x0DFFF) return INVALID_CHAR;
        // if (uc == 0x0FFFE || uc == 0x0FFFF) return INVALID_CHAR; // not valid ISO 10646
        return uc;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        // Four-byte sequence.
        FIRST_BYTE(0x07, 18);
        NEXT_BYTE(12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x010000) return INVALID_CHAR;  // overlong
        return uc;
    }
    else if ((c & 0xFC) == 0xF8)
    {
        // Five-byte sequence.
        FIRST_BYTE(0x03, 24);
        NEXT_BYTE(18);
        NEXT_BYTE(12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x0200000) return INVALID_CHAR; // overlong
        return uc;
    }
    else if ((c & 0xFE) == 0xFC)
    {
        // Six-byte sequence.
        FIRST_BYTE(0x01, 30);
        NEXT_BYTE(24);
        NEXT_BYTE(18);
        NEXT_BYTE(12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x04000000) return INVALID_CHAR;    // overlong
        return uc;
    }
    else
    {
        // Invalid.
        return INVALID_CHAR;
    }
}


void OVR_STDCALL EncodeChar(char* pbuffer, SPInt* pindex, UInt32 ucs_character)
{
    if (ucs_character <= 0x7F)
    {
        // Plain single-byte ASCII.
        pbuffer[(*pindex)++] = (char) ucs_character;
    }
    else if (ucs_character <= 0x7FF)
    {
        // Two bytes.
        pbuffer[(*pindex)++] = 0xC0 | (char)(ucs_character >> 6);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
    }
    else if (ucs_character <= 0xFFFF)
    {
        // Three bytes.
        pbuffer[(*pindex)++] = 0xE0 | (char)(ucs_character >> 12);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
    }
    else if (ucs_character <= 0x1FFFFF)
    {
        // Four bytes.
        pbuffer[(*pindex)++] = 0xF0 | (char)(ucs_character >> 18);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
    }
    else if (ucs_character <= 0x3FFFFFF)
    {
        // Five bytes.
        pbuffer[(*pindex)++] = 0xF8 | (char)(ucs_character >> 24);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 18) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
    }
    else if (ucs_character <= 0x7FFFFFFF)
    {
        // Six bytes.
        pbuffer[(*pindex)++] = 0xFC | (char)(ucs_character >> 30);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 24) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 18) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
        pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
    }
    else
    {
        // Invalid char; don't encode anything.
    }
}

SPInt OVR_STDCALL GetEncodeStringSize(const wchar_t* pchar, SPInt length)
{
    SPInt len = 0;
    if (length != -1)
        for (int i = 0; i < length; i++)
        {
            len += GetEncodeCharSize(pchar[i]);
        }
    else
        for (int i = 0;; i++)
        {
            if (pchar[i] == 0)
                return len;
            len += GetEncodeCharSize(pchar[i]);
        }
    return len;
}

void OVR_STDCALL EncodeString(char *pbuff, const wchar_t* pchar, SPInt length)
{
    SPInt ofs = 0;
    if (length != -1)
    {
        for (int i = 0; i < length; i++)
        {            
            EncodeChar(pbuff, &ofs, pchar[i]);
        }
    }
    else
    {
        for (int i = 0;; i++)
        {
            if (pchar[i] == 0)
                break;
            EncodeChar(pbuff, &ofs, pchar[i]);
        }
    }
    pbuff[ofs] = 0;
}

UPInt OVR_STDCALL DecodeString(wchar_t *pbuff, const char* putf8str, SPInt bytesLen)
{
    wchar_t *pbegin = pbuff;
    if (bytesLen == -1)
    {
        while (1)
        {
            UInt32 ch = DecodeNextChar_Advance0(&putf8str);
            if (ch == 0)
                break;
            else if (ch >= 0xFFFF)
                ch = 0xFFFD;
            *pbuff++ = wchar_t(ch);
        }
    }
    else
    {
        const char* p = putf8str;
        while ((p - putf8str) < bytesLen)
        {
            UInt32 ch = DecodeNextChar_Advance0(&p);
            if (ch >= 0xFFFF)
                ch = 0xFFFD;
            *pbuff++ = wchar_t(ch);
        }
    }

    *pbuff = 0;
    return pbuff - pbegin;
}


#ifdef UTF8_UNIT_TEST

// Compile this test case with something like:
//
// gcc utf8.cpp -g -I.. -DUTF8_UNIT_TEST -lstdc++ -o utf8_test
//
//    or
//
// cl utf8.cpp -Zi -Od -DUTF8_UNIT_TEST -I..
//
// If possible, try running the test program with the first arg
// pointing at the file:
//
// http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
// 
// and examine the results by eye to make sure they are acceptable to
// you.


#include "base/utility.h"
#include <stdio.h>


bool    check_equal(const char* utf8_in, const UInt32* ucs_in)
{
    for (;;)
    {
        UInt32  next_ucs = *ucs_in++;
        UInt32  next_ucs_from_utf8 = utf8::decode_next_unicode_character(&utf8_in);
        if (next_ucs != next_ucs_from_utf8)
        {
            return false;
        }
        if (next_ucs == 0)
        {
            OVR_ASSERT(next_ucs_from_utf8 == 0);
            break;
        }
    }
    
    return true;
}


void    log_ascii(const char* line)
{
    for (;;)
    {
        unsigned char   c = (unsigned char) *line++;
        if (c == 0)
        {
            // End of line.
            return;
        }
        else if (c != '\n'
            && (c < 32 || c > 127))
        {
            // Non-printable as plain ASCII.
            printf("<0x%02X>", (int) c);
        }
        else
        {
            printf("%c", c);
        }
    }
}


void    log_ucs(const UInt32* line)
{
    for (;;)
    {
        UInt32  uc = *line++;
        if (uc == 0)
        {
            // End of line.
            return;
        }
        else if (uc != '\n'
            && (uc < 32 || uc > 127))
        {
            // Non-printable as plain ASCII.
            printf("<U-%04X>", uc);
        }
        else
        {
            printf("%c", (char) uc);
        }
    }
}


// Simple canned test.
int main(int argc, const char* argv[])
{
    {
        const char* test8 = "Ignacio Castaño";
        const UInt32    test32[] =
        {
            0x49, 0x67, 0x6E, 0x61, 0x63,
                0x69, 0x6F, 0x20, 0x43, 0x61,
                0x73, 0x74, 0x61, 0xF1, 0x6F,
                0x00
        };
        
        OVR_ASSERT(check_equal(test8, test32));
    }
        
        // If user passed an arg, try reading the file as UTF-8 encoded text.
        if (argc > 1)
        {
            const char* filename = argv[1];
            FILE*   fp = fopen(filename, "rb");
            if (fp == NULL)
            {
                printf("Can't open file '%s'\n", filename);
                return 1;
            }
            
            // Read lines from the file, encode/decode them, and highlight discrepancies.
            const int LINE_SIZE = 200;  // max line size
            char    line_buffer_utf8[LINE_SIZE];
            char    reencoded_utf8[6 * LINE_SIZE];
            UInt32  line_buffer_ucs[LINE_SIZE];
            
            int byte_counter = 0;
            for (;;)
            {
                int c = fgetc(fp);
                if (c == EOF)
                {
                    // Done.
                    break;
                }
                line_buffer_utf8[byte_counter++] = c;
                if (c == '\n' || byte_counter >= LINE_SIZE - 2)
                {
                    // End of line.  Process the line.
                    line_buffer_utf8[byte_counter++] = 0;   // terminate.
                    
                    // Decode into UCS.
                    const char* p = line_buffer_utf8;
                    UInt32* q = line_buffer_ucs;
                    for (;;)
                    {
                        UInt32  uc = UTF8Util::DecodeNextChar(&p);
                        *q++ = uc;
                        
                        OVR_ASSERT(q < line_buffer_ucs + LINE_SIZE);
                        OVR_ASSERT(p < line_buffer_utf8 + LINE_SIZE);
                        
                        if (uc == 0) break;
                    }
                    
                    // Encode back into UTF-8.
                    q = line_buffer_ucs;
                    int index = 0;
                    for (;;)
                    {
                        UInt32  uc = *q++;
                        OVR_ASSERT(index < LINE_SIZE * 6 - 6);
                        int last_index = index;
                        UTF8Util::EncodeChar(reencoded_utf8, &index, uc);
                        OVR_ASSERT(index <= last_index + 6);
                        if (uc == 0) break;
                    }
                    
                    // This can be useful for debugging.
#if 0
                    // Show the UCS and the re-encoded UTF-8.
                    log_ucs(line_buffer_ucs);
                    log_ascii(reencoded_utf8);
#endif // 0
                    
                    OVR_ASSERT(check_equal(line_buffer_utf8, line_buffer_ucs));
                    OVR_ASSERT(check_equal(reencoded_utf8, line_buffer_ucs));
                    
                    // Start next line.
                    byte_counter = 0;
                }
            }
            
            fclose(fp);
        }
        
        return 0;
}


#endif // UTF8_UNIT_TEST

}} // namespace UTF8Util::OVR

