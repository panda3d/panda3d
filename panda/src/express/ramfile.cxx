// Filename: ramfile.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "ramfile.h"

////////////////////////////////////////////////////////////////////
//     Function: Ramfile::readline
//       Access: Published
//  Description: Assumes the stream represents a text file, and
//               extracts one line up to and including the trailing
//               newline character.  Returns empty string when the end
//               of file is reached.
//
//               The interface here is intentionally designed to be
//               similar to that for Python's File.readline()
//               function.
////////////////////////////////////////////////////////////////////
string Ramfile::
readline() {
  size_t start = _pos;
  while (_pos < _data.length() && _data[_pos] != '\n') {
    ++_pos;
  }

  if (_pos < _data.length() && _data[_pos] == '\n') {
    // Include the newline character also.
    ++_pos;
  }

  return _data.substr(start, _pos - start);
}

