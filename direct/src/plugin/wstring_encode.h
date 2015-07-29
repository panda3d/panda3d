// Filename: wstring_encode.h
// Created by:  drose (29Jun09)
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

#ifndef WSTRING_ENCODE_H
#define WSTRING_ENCODE_H

#include <string>
using namespace std;

// Presently, these two functions are implemented only for Windows,
// which is the only place they are needed.  (Only Windows requires
// wstrings for filenames.)
#ifdef _WIN32
bool wstring_to_string(string &result, const wstring &source);
bool string_to_wstring(wstring &result, const string &source);

// We declare this inline so it won't conflict with the similar
// function defined in Panda's textEncoder.h.
inline ostream &operator << (ostream &out, const wstring &str) {
  string result;
  if (wstring_to_string(result, str)) {
    out << result;
  }
  return out;
}

#endif // _WIN32

#endif


