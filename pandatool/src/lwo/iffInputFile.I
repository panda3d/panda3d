// Filename: iffInputFile.I
// Created by:  drose (24Apr01)
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


////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::set_filename
//       Access: Public
//  Description: Indicates the filename that the InputFile is
//               currently opened on.
////////////////////////////////////////////////////////////////////
INLINE void IffInputFile::
set_filename(const Filename &filename) {
  _filename = filename;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_filename
//       Access: Public
//  Description: Returns the filename that the InputFile is
//               currently opened on, if available.
////////////////////////////////////////////////////////////////////
INLINE const Filename &IffInputFile::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::is_eof
//       Access: Public
//  Description: Returns true if the last read operation failed
//               because of reaching EOF, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool IffInputFile::
is_eof() const {
  return _eof;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_bytes_read
//       Access: Public
//  Description: Returns the number of bytes read so far from the
//               input file.
////////////////////////////////////////////////////////////////////
INLINE size_t IffInputFile::
get_bytes_read() const {
  return _bytes_read;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::align
//       Access: Public
//  Description: If the current file pointer is not positioned on an
//               even-byte boundary, reads and discards one byte so
//               that it is.
////////////////////////////////////////////////////////////////////
INLINE void IffInputFile::
align() {
  if ((_bytes_read & 1) != 0) {
    get_int8();
  }
}
