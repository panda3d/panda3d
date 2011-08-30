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
    char *buffer = new char[size + 1];
    int rc = WideCharToMultiByte(CP_UTF8, 0, source.data(), source.length(),
                                 buffer, size, NULL, NULL);
    if (rc != 0) {
      buffer[size] = 0;
      result = buffer;
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
    wchar_t *buffer = new wchar_t[size + 1];
    int rc = MultiByteToWideChar(CP_UTF8, 0, source.data(), source.length(),
                                 buffer, size);
    if (rc != 0) {
      buffer[size] = 0;
      result = buffer;
      success = true;
    }
    delete[] buffer;
  }

  return success;
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: wstring ostream operator
//  Description: Converts the wstring to utf-8 for output.
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, const wstring &str) {
  string result;
  if (wstring_to_string(result, str)) {
    out << result;
  }
  return out;
}
