// Filename: dcPackData.cxx
// Created by:  drose (15Jun04)
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

#include "dcPackData.h"

static const size_t extra_size = 50;

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::set_used_length
//       Access: Private
//  Description: Ensures that the buffer has at least size bytes, and
//               sets the _used_length to the indicated value; grows
//               the buffer if it does not.
////////////////////////////////////////////////////////////////////
void DCPackData::
set_used_length(size_t size) {
  if (size > _allocated_size) {
    _allocated_size = size * size + extra_size;
    char *new_buf = new char[_allocated_size];
    if (_used_length > 0) {
      memcpy(new_buf, _buffer, _used_length);
    }
    if (_buffer != NULL) {
      delete[] _buffer;
    }
    _buffer = new_buf;
  }

  _used_length = size;
}
