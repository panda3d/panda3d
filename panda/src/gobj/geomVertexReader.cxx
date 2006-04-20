// Filename: geomVertexReader.cxx
// Created by:  drose (25Mar05)
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

#include "geomVertexReader.h"


#ifndef NDEBUG
  // This is defined just for the benefit of having something non-NULL
  // to return from a nassertr() call.
const unsigned char GeomVertexReader::empty_buffer[100] = { 0 };
#endif

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexReader::set_column
//       Access: Published
//  Description: Sets up the reader to use the indicated column
//               description on the given array.
//
//               This also resets the current read row number to the
//               start row (the same value passed to a previous call
//               to set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomVertexReader::
set_column(int array, const GeomVertexColumn *column) {
  if (_data_reader == (const GeomVertexDataPipelineReader *)NULL &&
      _array_reader == (const GeomVertexArrayDataPipelineReader *)NULL) {
    return false;
  }

  if (column == (const GeomVertexColumn *)NULL) {
    // Clear the data type.
    _array = -1;
    _packer = NULL;
    _stride = 0;
    _pointer = NULL;
    _pointer_end = NULL;

    return false;
  }

  if (_data_reader != (const GeomVertexDataPipelineReader *)NULL) {
#ifndef NDEBUG
    _array = -1;
    _packer = NULL;
    nassertr(array >= 0 && array < _data_reader->get_num_arrays(), false);
#endif
    _array = array;
    const GeomVertexArrayDataPipelineReader *array_reader = _data_reader->get_array_reader(_array);
    _stride = array_reader->get_array_format()->get_stride();

  } else {
    _stride = _array_reader->get_array_format()->get_stride();
  }

  _packer = column->_packer;
  
  set_pointer(_start_row);
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexReader::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomVertexReader::
output(ostream &out) const {
  const GeomVertexColumn *column = get_column();
  if (column == (GeomVertexColumn *)NULL) {
    out << "GeomVertexReader()";
    
  } else {
    out << "GeomVertexReader, array = " << get_array_data()
        << ", column = " << column->get_name()
        << " (" << get_packer()->get_name()
        << "), read row " << get_read_row();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexReader::initialize
//       Access: Private
//  Description: Called only by the constructor.
////////////////////////////////////////////////////////////////////
void GeomVertexReader::
initialize() {
  if (_data_reader != (const GeomVertexDataPipelineReader *)NULL) {
    _data_reader->check_array_readers();
  }

  _array = 0;
  _packer = NULL;
  _pointer_begin = NULL;
  _pointer_end = NULL;
  _pointer = NULL;
  _start_row = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexReader::clear_reader
//       Access: Private
//  Description: Destructs the GeomVertexDataPipelineReader, when
//               necessary, for instance when this object destructs.
////////////////////////////////////////////////////////////////////
void GeomVertexReader::
clear_reader() {
  nassertv(_owns_reader);

  if (_data_reader != (const GeomVertexDataPipelineReader *)NULL) {
    delete (GeomVertexDataPipelineReader *)_data_reader;
    _data_reader = NULL;
  } else {
    nassertv(_array_reader != (const GeomVertexArrayDataPipelineReader *)NULL);
    delete (GeomVertexArrayDataPipelineReader *)_array_reader;
    _array_reader = NULL;
  }

  _owns_reader = false;
}
