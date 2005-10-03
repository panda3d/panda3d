// Filename: geomVertexWriter.I
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
//     Function: GeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexData *vertex_data) :
  _vertex_data(vertex_data)
{
  initialize();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.  This flavor creates the
//               writer specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexData *vertex_data, const string &name) :
  _vertex_data(vertex_data)
{
  initialize();
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.  This flavor creates the
//               writer specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexData *vertex_data, const InternalName *name) :
  _vertex_data(vertex_data)
{
  initialize();
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexArrayData *array_data) :
  _array_data(array_data)
{
  initialize();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexArrayData *array_data, int column) :
  _array_data(array_data)
{
  initialize();
  set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
GeomVertexWriter(const GeomVertexWriter &copy) :
  _vertex_data(copy._vertex_data),
  _array(copy._array),
  _array_data(copy._array_data),
  _packer(copy._packer),
  _stride(copy._stride),
  _pointer_begin(copy._pointer_begin),
  _pointer_end(copy._pointer_end),
  _pointer(copy._pointer),
  _start_row(copy._start_row)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
operator = (const GeomVertexWriter &copy) {
  _vertex_data = copy._vertex_data;
  _array = copy._array;
  _array_data = copy._array_data;
  _packer = copy._packer;
  _stride = copy._stride;
  _pointer_begin = copy._pointer_begin;
  _pointer_end = copy._pointer_end;
  _pointer = copy._pointer;
  _start_row = copy._start_row;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexWriter::
~GeomVertexWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               writer is processing.  This may return NULL if the
//               writer was constructed with just an array pointer.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData *GeomVertexWriter::
get_vertex_data() const {
  return _vertex_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_array_data
//       Access: Published
//  Description: Returns the particular array object that the
//               writer is currently processing.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData *GeomVertexWriter::
get_array_data() const {
  return _array_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_column
//       Access: Published
//  Description: Sets up the writer to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets the write row number to the start
//               row (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexWriter::
set_column(int column) {
  if (_vertex_data != (GeomVertexData *)NULL) {
    return set_column(_vertex_data->get_format()->get_array_with(column),
                      _vertex_data->get_format()->get_column(column));
  }
  if (_array_data != (GeomVertexArrayData *)NULL) {
    return set_column(0, _array_data->get_array_format()->get_column(column));
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_column
//       Access: Published
//  Description: Sets up the writer to use the data type with the
//               indicated name.
//
//               This also resets the write row number to the start
//               row (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexWriter::
set_column(const string &name) {
  return set_column(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_column
//       Access: Published
//  Description: Sets up the writer to use the data type with the
//               indicated name.
//
//               This also resets the write number to the start row
//               (the same value passed to a previous call to
//               set_row(), or 0 if set_row() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexWriter::
set_column(const InternalName *name) {
  if (_vertex_data != (GeomVertexData *)NULL) {
    return set_column(_vertex_data->get_format()->get_array_with(name),
                      _vertex_data->get_format()->get_column(name));
  }
  if (_array_data != (GeomVertexArrayData *)NULL) {
    return set_column(0, _array_data->get_array_format()->get_column(name));
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::has_column
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexWriter::
has_column() const {
  return (_packer != (GeomVertexColumn::Packer *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the writer is working on.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexWriter::
get_array() const {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_column
//       Access: Published
//  Description: Returns the description of the data type that the
//               writer is working on.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexColumn *GeomVertexWriter::
get_column() const {
  if (_packer != (GeomVertexColumn::Packer *)NULL) {
    return _packer->_column;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_row
//       Access: Published
//  Description: Sets the start row to the indicated value.  The
//               writer will begin writing to the indicated row;
//               each subsequent set_data*() call will store the data
//               into the subsequent row.  If set_column() is called,
//               the writer will return to this row.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_row(int row) {
  _start_row = row;
  if (has_column()) {
    quick_set_pointer(_start_row);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_start_row
//       Access: Published
//  Description: Returns the row index at which the writer
//               started.  It will return to this row if you reset
//               the current column.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexWriter::
get_start_row() const {
  return _start_row;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::get_write_row
//       Access: Published
//  Description: Returns the row index to which the data will be
//               written at the next call to set_data*() or
//               add_data*().
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexWriter::
get_write_row() const {
  return (int)(_pointer - _pointer_begin) / _stride;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::is_at_end
//       Access: Published
//  Description: Returns true if the writer is currently at the end of
//               the list of vertices, false otherwise.  If this is
//               true, another call to set_data*() will result in a
//               crash, but another call to add_data*() will add a new
//               row.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexWriter::
is_at_end() const {
  return _pointer >= _pointer_end;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data1f
//       Access: Published
//  Description: Sets the write row to a particular 1-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data1f(float data) {
  nassertv(has_column());
  _packer->set_data1f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data2f
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data2f(float x, float y) {
  set_data2f(LVecBase2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data2f
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data2f(const LVecBase2f &data) {
  nassertv(has_column());
  _packer->set_data2f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data3f
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data3f(float x, float y, float z) {
  set_data3f(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data3f
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data3f(const LVecBase3f &data) {
  nassertv(has_column());
  _packer->set_data3f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data4f
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data4f(float x, float y, float z, float w) {
  set_data4f(LVecBase4f(x, y, z, w));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data4f
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data4f(const LVecBase4f &data) {
  nassertv(has_column());
  _packer->set_data4f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data1i
//       Access: Published
//  Description: Sets the write row to a particular 1-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data1i(int data) {
  nassertv(has_column());
  _packer->set_data1i(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data2i
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data2i(int a, int b) {
  nassertv(has_column());
  _packer->set_data2i(inc_pointer(), a, b);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data2i
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data2i(const int data[2]) {
  set_data2i(data[0], data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data3i
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data3i(int a, int b, int c) {
  nassertv(has_column());
  _packer->set_data3i(inc_pointer(), a, b, c);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data3i
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data3i(const int data[3]) {
  set_data3i(data[0], data[1], data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data4i
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data4i(int a, int b, int c, int d) {
  nassertv(has_column());
  _packer->set_data4i(inc_pointer(), a, b, c, d);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_data4i
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               It is an error for the write row to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_data4i(const int data[4]) {
  set_data4i(data[0], data[1], data[2], data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data1f
//       Access: Published
//  Description: Sets the write row to a particular 1-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data1f(float data) {
  nassertv(has_column());
  _packer->set_data1f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data2f
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data2f(float x, float y) {
  add_data2f(LVecBase2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data2f
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data2f(const LVecBase2f &data) {
  nassertv(has_column());
  _packer->set_data2f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data3f
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data3f(float x, float y, float z) {
  add_data3f(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data3f
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data3f(const LVecBase3f &data) {
  nassertv(has_column());
  _packer->set_data3f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data4f
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data4f(float x, float y, float z, float w) {
  add_data4f(LVecBase4f(x, y, z, w));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data4f
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data4f(const LVecBase4f &data) {
  nassertv(has_column());
  _packer->set_data4f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data1i
//       Access: Published
//  Description: Sets the write row to a particular 1-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data1i(int data) {
  nassertv(has_column());
  _packer->set_data1i(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data2i
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data2i(int a, int b) {
  nassertv(has_column());
  _packer->set_data2i(inc_add_pointer(), a, b);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data2i
//       Access: Published
//  Description: Sets the write row to a particular 2-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data2i(const int data[2]) {
  add_data2i(data[0], data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data3i
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data3i(int a, int b, int c) {
  nassertv(has_column());
  _packer->set_data3i(inc_add_pointer(), a, b, c);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data3i
//       Access: Published
//  Description: Sets the write row to a particular 3-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data3i(const int data[3]) {
  add_data3i(data[0], data[1], data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data4i
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data4i(int a, int b, int c, int d) {
  nassertv(has_column());
  _packer->set_data4i(inc_add_pointer(), a, b, c, d);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::add_data4i
//       Access: Published
//  Description: Sets the write row to a particular 4-component
//               value, and advances the write row.
//
//               If the write row advances past the end of data,
//               implicitly adds a new row to the data.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
add_data4i(const int data[4]) {
  add_data4i(data[0], data[1], data[2], data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::set_pointer
//       Access: Private
//  Description: Sets up the array pointers freshly from the source
//               object (in case they have been reallocated recently),
//               and sets the internal pointer to the indicated row.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
set_pointer(int row) {
  if (_vertex_data != (const GeomVertexData *)NULL) {
    GeomVertexArrayData *array_data = _vertex_data->modify_array(_array);
    _pointer_begin = array_data->modify_data();
    _pointer_end = _pointer_begin + array_data->get_data_size_bytes();

  } else {
    _pointer_begin = _array_data->modify_data();
    _pointer_end = _pointer_begin + _array_data->get_data_size_bytes();
  }
  quick_set_pointer(row);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::quick_set_pointer
//       Access: Private
//  Description: Sets up the internal pointer to the indicated row,
//               without first verifying that arrays haven't been
//               reallocated.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexWriter::
quick_set_pointer(int row) {
  nassertv(has_column());

#ifdef _DEBUG
  if (_vertex_data != (const GeomVertexData *)NULL) {
    const GeomVertexArrayData *array_data = _vertex_data->get_array(_array);
    nassertv(_pointer_begin == array_data->get_data());
  } else {
    nassertv(_pointer_begin == _array_data->get_data());
  }
#endif

  _pointer = _pointer_begin + _packer->_column->get_start() + _stride * row;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::inc_pointer
//       Access: Private
//  Description: Increments to the next row, and returns the data
//               pointer as it was before incrementing.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *GeomVertexWriter::
inc_pointer() {
#ifdef _DEBUG
  nassertr(_pointer < _pointer_end, empty_buffer);
  if (_vertex_data != (GeomVertexData *)NULL){ 
    const GeomVertexArrayData *array_data = _vertex_data->get_array(_array);
    nassertr(_pointer >= array_data->get_data().p() && _pointer < array_data->get_data().p() + array_data->get_data_size_bytes(), empty_buffer);
  } else {
    nassertr(_pointer >= _array_data->get_data().p() && _pointer < _array_data->get_data().p() + _array_data->get_data_size_bytes(), empty_buffer);
  }
#endif

  unsigned char *orig_pointer = _pointer;
  _pointer += _stride;
  return orig_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexWriter::inc_add_pointer
//       Access: Private
//  Description: Increments to the next row, and returns the data
//               pointer as it was before incrementing.  If we are at
//               or past the end of data, implicitly adds more
//               rows first.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *GeomVertexWriter::
inc_add_pointer() {
  if (_pointer >= _pointer_end) {
    // Reset the data pointer.
    int write_row = get_write_row();
    if (_vertex_data != (GeomVertexData *)NULL) {
      _vertex_data->set_num_rows(max(write_row + 1, _vertex_data->get_num_rows()));
    } else {
      _array_data->set_num_rows(max(write_row + 1, _array_data->get_num_rows()));
    }
    set_pointer(write_row);
  }
  return inc_pointer();
}
