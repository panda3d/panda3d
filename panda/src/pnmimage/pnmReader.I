// Filename: pnmReader.I
// Created by:  drose (16Jun00)
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
//     Function: PNMReader::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMReader::
PNMReader(PNMFileType *type, istream *file, bool owns_file) :
  _type(type),
  _owns_file(owns_file),
  _file(file),
  _is_valid(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::get_type
//       Access: Public
//  Description: Returns a pointer to the PNMFileType object that
//               created this PNMReader.
////////////////////////////////////////////////////////////////////
INLINE PNMFileType *PNMReader::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::is_valid
//       Access: Public
//  Description: Returns true if the PNMReader can be used to read
//               data, false if something is wrong.
////////////////////////////////////////////////////////////////////
INLINE bool PNMReader::
is_valid() const {
  return _is_valid;
}
