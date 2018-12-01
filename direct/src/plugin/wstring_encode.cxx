/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wstring_encode.cxx
 * @author drose
 * @date 2011-08-29
 */

#include "wstring_encode.h"

#include <ctype.h>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32

#ifdef _WIN32
/**
 * Encodes std::wstring to std::string using UTF-8.
 */
bool
wstring_to_string(std::string &result, const std::wstring &source) {
  bool success = false;
  int size = WideCharToMultiByte(CP_UTF8, 0, source.data(), source.length(),
                                 nullptr, 0, nullptr, nullptr);
  if (size > 0) {
    char *buffer = new char[size];
    int rc = WideCharToMultiByte(CP_UTF8, 0, source.data(), source.length(),
                                 buffer, size, nullptr, nullptr);
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
/**
 * Decodes std::string to std::wstring using UTF-8.
 */
bool
string_to_wstring(std::wstring &result, const std::string &source) {
  bool success = false;
  int size = MultiByteToWideChar(CP_UTF8, 0, source.data(), source.length(),
                                 nullptr, 0);
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
