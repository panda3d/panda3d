// Filename: qpgeomVertexReader.I
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


////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Constructor
//       Access: Published
//  Description: Constructs a new reader to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
qpGeomVertexReader(const qpGeomVertexData *vertex_data) :
  _vertex_data(vertex_data)
{
  initialize();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Constructor
//       Access: Published
//  Description: Constructs a new reader to process the vertices of
//               the indicated data object.  This flavor creates the
//               reader specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
qpGeomVertexReader(const qpGeomVertexData *vertex_data, const string &name) :
  _vertex_data(vertex_data)
{
  initialize();
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Constructor
//       Access: Published
//  Description: Constructs a new reader to process the vertices of
//               the indicated data object.  This flavor creates the
//               reader specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
qpGeomVertexReader(const qpGeomVertexData *vertex_data, 
                   const InternalName *name) :
  _vertex_data(vertex_data)
{
  initialize();
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Constructor
//       Access: Published
//  Description: Constructs a new reader to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
qpGeomVertexReader(const qpGeomVertexArrayData *array_data) :
  _array_data(array_data)
{
  initialize();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Constructor
//       Access: Published
//  Description: Constructs a new reader to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
qpGeomVertexReader(const qpGeomVertexArrayData *array_data, int column) :
  _array_data(array_data)
{
  initialize();
  set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
~qpGeomVertexReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               reader is processing.  This may return NULL if the
//               reader was constructed with just an array pointer.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexData *qpGeomVertexReader::
get_vertex_data() const {
  return _vertex_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_array_data
//       Access: Published
//  Description: Returns the particular array object that the
//               reader is currently processing.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexArrayData *qpGeomVertexReader::
get_array_data() const {
  return _array_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_column
//       Access: Published
//  Description: Sets up the reader to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets the read row number to the start row
//               (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_column(int column) {
  if (_vertex_data != (const qpGeomVertexData *)NULL) {
    return set_column(_vertex_data->get_format()->get_array_with(column),
                      _vertex_data->get_format()->get_column(column));
  }
  if (_array_data != (const qpGeomVertexArrayData *)NULL) {
    return set_column(0, _array_data->get_array_format()->get_column(column));
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_column
//       Access: Published
//  Description: Sets up the reader to use the data type with the
//               indicated name.
//
//               This also resets the read row number to the start row
//               (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_column(const string &name) {
  return set_column(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_column
//       Access: Published
//  Description: Sets up the reader to use the data type with the
//               indicated name.
//
//               This also resets the read row number to the start row
//               (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_column(const InternalName *name) {
  if (_vertex_data != (const qpGeomVertexData *)NULL) {
    return set_column(_vertex_data->get_format()->get_array_with(name),
                      _vertex_data->get_format()->get_column(name));
  }
  if (_array_data != (const qpGeomVertexArrayData *)NULL) {
    return set_column(0, _array_data->get_array_format()->get_column(name));
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::has_column
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
has_column() const {
  return (_packer != (qpGeomVertexColumn::Packer *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the reader is working on.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_array() const {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_column
//       Access: Published
//  Description: Returns the description of the data type that the
//               reader is working on.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexColumn *qpGeomVertexReader::
get_column() const {
  if (_packer != (qpGeomVertexColumn::Packer *)NULL) {
    return _packer->_column;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_row
//       Access: Published
//  Description: Sets the start row to the indicated value.  The
//               reader will begin reading from the indicated row;
//               each subsequent get_data*() call will return the data
//               from the subsequent row.  If set_column() is called,
//               the reader will return to this row.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::
set_row(int row) {
  _start_row = row;
  if (has_column()) {
    set_pointer(_start_row);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_start_row
//       Access: Published
//  Description: Returns the row index at which the reader started.
//               It will return to this row if you reset the current
//               column.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_start_row() const {
  return _start_row;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::is_at_end
//       Access: Published
//  Description: Returns true if the reader is currently at the end of
//               the list of vertices, false otherwise.  If this is
//               true, another call to get_data*() will result in a
//               crash.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
is_at_end() const {
  return _pointer >= _pointer_end;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data1f
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 1-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE float qpGeomVertexReader::
get_data1f() {
  nassertr(has_column(), 0.0f);
  return _packer->get_data1f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data2f
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 2-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2f &qpGeomVertexReader::
get_data2f() {
  nassertr(has_column(), LVecBase2f::zero());
  return _packer->get_data2f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data3f
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 3-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &qpGeomVertexReader::
get_data3f() {
  nassertr(has_column(), LVecBase3f::zero());
  return _packer->get_data3f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data4f
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 4-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &qpGeomVertexReader::
get_data4f() {
  nassertr(has_column(), LVecBase4f::zero());
  return _packer->get_data4f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data1i
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 1-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_data1i() {
  nassertr(has_column(), 0);
  return _packer->get_data1i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data2i
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 2-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data2i() {
  nassertr(has_column(), 0);
  return _packer->get_data2i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data3i
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 3-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data3i() {
  nassertr(has_column(), 0);
  return _packer->get_data3i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data4i
//       Access: Published
//  Description: Returns the data associated with the read row,
//               expressed as a 4-component value, and advances the
//               read row.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data4i() {
  nassertr(has_column(), 0);
  return _packer->get_data4i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_pointer
//       Access: Private
//  Description: Sets up the internal read pointer, etc. to read from
//               the indicated row.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::
set_pointer(int row) {
  nassertv(has_column());
  if (_vertex_data != (const qpGeomVertexData *)NULL) {
    const qpGeomVertexArrayData *array_data = _vertex_data->get_array(_array);
    _pointer = array_data->get_data();
    _pointer_end = _pointer + array_data->get_data_size_bytes();
    _pointer += _packer->_column->get_start() + _stride * row;

  } else {
    _pointer = _array_data->get_data();
    _pointer_end = _pointer + _array_data->get_data_size_bytes();
    _pointer += _packer->_column->get_start() + _stride * row;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::inc_pointer
//       Access: Private
//  Description: Increments to the next row, and returns the data
//               pointer as it was before incrementing.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *qpGeomVertexReader::
inc_pointer() {
#ifndef NDEBUG
  nassertr(_pointer < _pointer_end, empty_buffer);
  if (_vertex_data != (const qpGeomVertexData *)NULL){ 
    const qpGeomVertexArrayData *array_data = _vertex_data->get_array(_array);
    nassertr(_pointer >= array_data->get_data().p() && _pointer < array_data->get_data().p() + array_data->get_data_size_bytes(), empty_buffer);
  } else {
    nassertr(_pointer >= _array_data->get_data().p() && _pointer < _array_data->get_data().p() + _array_data->get_data_size_bytes(), empty_buffer);
  }
#endif

  const unsigned char *orig_pointer = _pointer;
  _pointer += _stride;
  return orig_pointer;
}
