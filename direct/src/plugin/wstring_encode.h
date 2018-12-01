/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wstring_encode.h
 * @author drose
 * @date 2009-06-29
 */

#ifndef WSTRING_ENCODE_H
#define WSTRING_ENCODE_H

#include <string>

// Presently, these two functions are implemented only for Windows, which is
// the only place they are needed.  (Only Windows requires wstrings for
// filenames.)
#ifdef _WIN32
bool wstring_to_string(std::string &result, const std::wstring &source);
bool string_to_wstring(std::wstring &result, const std::string &source);

// We declare this inline so it won't conflict with the similar function
// defined in Panda's textEncoder.h.
inline std::ostream &operator << (std::ostream &out, const std::wstring &str) {
  std::string result;
  if (wstring_to_string(result, str)) {
    out << result;
  }
  return out;
}

#endif // _WIN32

#endif
