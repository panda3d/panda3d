// Filename: pnmWriter.I
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
//     Function: PNMWriter::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMWriter::
PNMWriter(PNMFileType *type, ostream *file, bool owns_file) :
  _type(type),
  _owns_file(owns_file),
  _file(file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::get_type
//       Access: Public
//  Description: Returns a pointer to the PNMFileType object that
//               created this PNMWriter.
////////////////////////////////////////////////////////////////////
INLINE PNMFileType *PNMWriter::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::set_color_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
set_color_type(ColorType type) {
  set_num_channels((int)type);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::set_num_channels
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
set_num_channels(int num_channels) {
  nassertv(num_channels >= 1 && num_channels <= 4);
  _num_channels = num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::set_maxval
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
set_maxval(xelval maxval) {
  _maxval = maxval;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::set_x_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
set_x_size(int x_size) {
  nassertv(x_size >= 0);
  _x_size = x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::set_y_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
set_y_size(int y_size) {
  nassertv(y_size >= 0);
  _y_size = y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::copy_header_from
//       Access: Public
//  Description: Initializes all the data in the header (x_size,
//               y_size, num_channels, etc.) to the same values
//               indicated in the given header.  This should be done
//               before writing anything to the file.
////////////////////////////////////////////////////////////////////
INLINE void PNMWriter::
copy_header_from(const PNMImageHeader &header) {
  PNMImageHeader::operator = (header);
}
