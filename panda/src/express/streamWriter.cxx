// Filename: streamWriter.cxx
// Created by:  drose (04Aug02)
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

#include "streamWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::pad_bytes
//       Access: Public
//  Description: Adds the indicated number of zero bytes to the
//               stream.
////////////////////////////////////////////////////////////////////
void StreamWriter::
pad_bytes(size_t size) {
  nassertv((int)size >= 0);

  while (size > 0) {
    _out->put('\0');
    size--;
  }
}
