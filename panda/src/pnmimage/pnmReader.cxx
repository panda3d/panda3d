// Filename: pnmReader.cxx
// Created by:  drose (14Jun00)
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

#include "pnmReader.h"
#include "virtualFileSystem.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PNMReader::
~PNMReader() {
  if (_owns_file) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    // We're assuming here that the file was opened via VFS.  That
    // may not necessarily be the case, but we don't make that
    // distinction.  However, at the moment at least, that
    // distinction doesn't matter, since vfs->close_read_file()
    // just deletes the file pointer anyway.
    vfs->close_read_file(_file);
  }
  _file = (istream *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::read_data
//       Access: Public, Virtual
//  Description: Reads in an entire image all at once, storing it in
//               the pre-allocated _x_size * _y_size array and alpha
//               pointers.  (If the image type has no alpha channel,
//               alpha is ignored.)  Returns the number of rows
//               correctly read.
//
//               Derived classes need not override this if they
//               instead provide supports_read_row() and read_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMReader::
read_data(xel *array, xelval *alpha) {
  if (!is_valid()) {
    return 0;
  }

  int y;
  for (y = 0; y < _y_size; y++) {
    if (!read_row(array + y * _x_size, alpha + y * _x_size)) {
      return y;
    }
  }

  return _y_size;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMReader::supports_read_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader is capable
//               of returning the data one row at a time, via repeated
//               calls to read_row().  Returns false if the only way
//               to read from this file is all at once, via
//               read_data().
////////////////////////////////////////////////////////////////////
bool PNMReader::
supports_read_row() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::read_row
//       Access: Public, Virtual
//  Description: If supports_read_row(), above, returns true, this
//               function may be called repeatedly to read the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully read, false
//               if there is an error or end of file.
////////////////////////////////////////////////////////////////////
bool PNMReader::
read_row(xel *, xelval *) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMReader::supports_stream_read
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader can read
//               from a general stream (including pipes, etc.), or
//               false if the reader must occasionally fseek() on its
//               input stream, and thus only disk streams are
//               supported.
////////////////////////////////////////////////////////////////////
bool PNMReader::
supports_stream_read() const {
  return false;
}
