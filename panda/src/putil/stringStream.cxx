// Filename: stringStream.cxx
// Created by:  drose (03Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "stringStream.h"

////////////////////////////////////////////////////////////////////
//     Function: StringStream::read
//       Access: Published
//  Description: Extracts up to max_length characters from the data
//               stream and stores them in the indicated string.
////////////////////////////////////////////////////////////////////
void StringStream::
read(string &data, size_t max_length) {
  flush();
  char *buffer = (char *)PANDA_MALLOC_ARRAY(max_length);
  size_t length = _buf.read_chars(buffer, max_length);
  data.append(buffer, length);
  PANDA_FREE_ARRAY(buffer);
}
