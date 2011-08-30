// Filename: wstring_encode.cxx
// Created by:  drose (29Aug11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "wstring_encode.h"

#include <ctype.h>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: wstring_to_string
//  Description: Encodes std::wstring to std::string using UTF-8.
////////////////////////////////////////////////////////////////////
bool
wstring_to_string(string &result, const wstring &source) {
  bool success = false;
  int size = WideCharToMultiByte(CP_UTF8, 0, source.data(), source.length(),
                                 NULL, 0, NULL, NULL);
  if (size > 0) {
    char *buffer = new char[size];
    int rc = WideCharToMultiByte(CP_UTF8, 0, source.data(), source.length(),
                                 buffer, size, NULL, NULL);
    if (rc != 0) {
      result.assign(buffer, size);
      success = true;
    }
    delete[] buffer;
  }

  return success;
}
#endif  // _WIN32

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: string_to_wstring
//  Description: Decodes std::string to std::wstring using UTF-8.
////////////////////////////////////////////////////////////////////
bool
string_to_wstring(wstring &result, const string &source) {
  bool success = false;
  int size = MultiByteToWideChar(CP_UTF8, 0, source.data(), source.length(),
                                 NULL, 0);
  if (size > 0) {
    wchar_t *buffer = new wchar_t[size];
    int rc = MultiByteToWideChar(CP_UTF8, 0, source.data(), source.length(),
                                 buffer, size);
    if (rc != 0) {
      result.assign(buffer, size);
      success = true;
    }
    delete[] buffer;
  }

  return success;
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: parse_hexdigit
//  Description: Parses a single hex digit.  Returns true on success,
//               false on failure.  On success, fills result with the
//               parsed value, an integer in the range 0..15.
////////////////////////////////////////////////////////////////////
bool
parse_hexdigit(int &result, char digit) {
  if (isdigit(digit)) {
    result = digit - '0';
    return true;
  } else if (isxdigit(digit)) {
    result = tolower(digit) - 'a' + 10;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: url_quote
//  Description: Applies URL quoting to source and stores the result
//               in result.
////////////////////////////////////////////////////////////////////
void
url_quote(string &result, const string &source) {
  ostringstream strm;
  strm << hex << setfill('0');
  for (size_t p = 0; p < source.length(); ++p) {
    if (source[p] < 0x20 || source[p] >= 0x7f) {
      strm << "%" << setw(2) << (unsigned int)(unsigned char)source[p];
    } else {
      switch (source[p]) {
        // We could quote all of these punctuation marks too, the same
        // way actual URL quoting does.  Maybe we will one day in the
        // future, though I don't think it matters much; mainly we're
        // relying on quoting to protect the high-bit characters.  For
        // now, then, we leave these unquoted, for compatibility with
        // the p3dpython from Panda3D 1.7, which didn't expect any
        // quoting at all.
        /*
      case ' ':
      case '<':
      case '>':
      case '#':
      case '%':
      case '{':
      case '}':
      case '|':
      case '\\':
      case '^':
      case '~':
      case '[':
      case ']':
      case '`':
      case ';':
      case '?':
      case ':':
      case '@':
      case '=':
      case '&':
      case '$':
        strm << "%" << setw(2) << (unsigned int)(unsigned char)source[p];
        break;
        */

      default:
        strm << (char)source[p];
      }
    }
  }
  result = strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: url_unquote
//  Description: Removes URL quoting from source and stores the result
//               in result.
////////////////////////////////////////////////////////////////////
void
url_unquote(string &result, const string &source) {
  result = string();
  size_t p = 0;
  while (p < source.length()) {
    if (source[p] == '%') {
      ++p;
      int ch = 0;
      if (p < source.length()) {
        int digit;
        if (parse_hexdigit(digit, source[p])) {
          ch += (digit << 4);
        }
        ++p;
      }
      if (p < source.length()) {
        int digit;
        if (parse_hexdigit(digit, source[p])) {
          ch += digit;
        }
        ++p;
      }
      result.push_back((char)ch);
    } else {
      result.push_back(source[p]);
      ++p;
    }
  }
}
