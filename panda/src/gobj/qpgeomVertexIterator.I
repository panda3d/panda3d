// Filename: qpgeomVertexIterator.I
// Created by:  drose (10Mar05)
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
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(qpGeomVertexData *data) :
  _data(data),
  _const_data(false),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.  This flavor creates the
//               iterator specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(qpGeomVertexData *data, const string &name) :
  _data(data),
  _const_data(false),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.  This flavor creates the
//               iterator specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(qpGeomVertexData *data, const InternalName *name) :
  _data(data),
  _const_data(false),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(const qpGeomVertexData *data) :
  _data((qpGeomVertexData *)data),
  _const_data(true),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.  This flavor creates the
//               iterator specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(const qpGeomVertexData *data, const string &name) :
  _data((qpGeomVertexData *)data),
  _const_data(true),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::Constructor
//       Access: Published
//  Description: Constructs a new iterator to process the vertices of
//               the indicated data object.  This flavor creates the
//               iterator specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexIterator::
qpGeomVertexIterator(const qpGeomVertexData *data, const InternalName *name) :
  _data((qpGeomVertexData *)data),
  _const_data(true),
  _array(0),
  _data_type(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _write_vertex(0)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data
//       Access: Published
//  Description: Returns the current data object that the iterator is
//               processing.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexData *qpGeomVertexIterator::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data_type
//       Access: Published
//  Description: Sets up the iterator to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets the read and write vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data_type(int data_type) {
  nassertv(_data->get_format()->get_array_with(data_type) >= 0);
  set_data_type(_data->get_format()->get_array_with(data_type),
                _data->get_format()->get_data_type(data_type));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data_type
//       Access: Published
//  Description: Sets up the iterator to use the data type with the
//               indicated name.
//
//               This also resets the read and write vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data_type(const string &name) {
  set_data_type(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data_type
//       Access: Published
//  Description: Sets up the iterator to use the data type with the
//               indicated name.
//
//               This also resets the read and write vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data_type(const InternalName *name) {
  nassertv(_data->get_format()->get_array_with(name) >= 0);
  set_data_type(_data->get_format()->get_array_with(name),
                _data->get_format()->get_data_type(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data_type
//       Access: Published
//  Description: Sets up the iterator to use the indicated data_type
//               description on the given array.
//
//               This also resets the read and write vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data_type(int array, const qpGeomVertexDataType *data_type) {
  nassertv(array >= 0 && array < _data->get_num_arrays());
  nassertv(data_type != (qpGeomVertexDataType *)NULL);
  _array = array;
  _data_type = data_type;

  _read_vertex = _start_vertex;
  _write_vertex = _start_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the iterator is working on.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexIterator::
get_array() const {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data_type
//       Access: Published
//  Description: Returns the description of the data type that the
//               iterator is working on.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexDataType *qpGeomVertexIterator::
get_data_type() const {
  return _data_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_vertex
//       Access: Published
//  Description: Sets the start, read, and write index to the
//               indicated value.  The iterator will begin traversing
//               from the given vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_vertex(int vertex) {
  _start_vertex = vertex;
  _read_vertex = vertex;
  _write_vertex = vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_start_vertex
//       Access: Published
//  Description: Returns the vertex index at which the iterator
//               started.  It will return to this vertex if you reset
//               the current data_type.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexIterator::
get_start_vertex() const {
  return _start_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_read_vertex
//       Access: Published
//  Description: Returns the current read vertex index of the
//               iterator.  This is the index whose value will be
//               returned by the next call to get_vertex().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexIterator::
get_read_vertex() const {
  return _read_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_write_vertex
//       Access: Published
//  Description: Returns the current write vertex index of the
//               iterator.  This is the index whose value will be
//               modified by the next call to set_vertex().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexIterator::
get_write_vertex() const {
  return _write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data1f
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data1f(float data) {
  nassertv(!_const_data);
  _data->set_data(_array, _data_type, _write_vertex, &data, 1);
  ++_write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data2f(float x, float y) {
  set_data2f(LVecBase2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data2f(const LVecBase2f &data) {
  nassertv(!_const_data);
  _data->set_data(_array, _data_type, _write_vertex, data.get_data(), 2);
  ++_write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data3f(float x, float y, float z) {
  set_data3f(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data3f(const LVecBase3f &data) {
  nassertv(!_const_data);
  _data->set_data(_array, _data_type, _write_vertex, data.get_data(), 3);
  ++_write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data4f(float x, float y, float z, float w) {
  set_data4f(LVecBase4f(x, y, z, w));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data4f(const LVecBase4f &data) {
  nassertv(!_const_data);
  _data->set_data(_array, _data_type, _write_vertex, data.get_data(), 4);
  ++_write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data1f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 1-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE float qpGeomVertexIterator::
get_data1f() {
  float data;
  _data->get_data(_array, _data_type, _read_vertex, &data, 1);
  ++_read_vertex;
  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data2f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 2-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f qpGeomVertexIterator::
get_data2f() {
  float data[4];
  int num_values = min(_data_type->get_num_values(), 4);
  _data->get_data(_array, _data_type, _read_vertex, data, num_values);
  ++_read_vertex;
  LVecBase2f result;
  qpGeomVertexData::to_vec2(result, data, num_values);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data3f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 3-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f qpGeomVertexIterator::
get_data3f() {
  float data[4];
  int num_values = min(_data_type->get_num_values(), 4);
  _data->get_data(_array, _data_type, _read_vertex, data, num_values);
  ++_read_vertex;
  LVecBase3f result;
  qpGeomVertexData::to_vec3(result, data, num_values);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data4f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 4-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4f qpGeomVertexIterator::
get_data4f() {
  float data[4];
  int num_values = min(_data_type->get_num_values(), 4);
  _data->get_data(_array, _data_type, _read_vertex, data, num_values);
  ++_read_vertex;
  LVecBase4f result;
  qpGeomVertexData::to_vec4(result, data, num_values);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::set_data1i
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexIterator::
set_data1i(int data) {
  nassertv(!_const_data);
  _data->set_data(_array, _data_type, _write_vertex, &data, 1);
  ++_write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexIterator::get_data1i
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 1-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexIterator::
get_data1i() {
  int data;
  _data->get_data(_array, _data_type, _read_vertex, &data, 1);
  ++_read_vertex;
  return data;
}
