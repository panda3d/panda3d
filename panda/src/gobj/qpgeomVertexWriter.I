// Filename: qpgeomVertexWriter.I
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
//     Function: qpGeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexWriter::
qpGeomVertexWriter(qpGeomVertexData *vertex_data) :
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _write_vertex(0),
  _num_vertices(0),
  _writer(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.  This flavor creates the
//               writer specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexWriter::
qpGeomVertexWriter(qpGeomVertexData *vertex_data, const string &name) :
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _write_vertex(0),
  _num_vertices(0),
  _writer(NULL)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Constructor
//       Access: Published
//  Description: Constructs a new writer to process the vertices of
//               the indicated data object.  This flavor creates the
//               writer specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexWriter::
qpGeomVertexWriter(qpGeomVertexData *vertex_data, const InternalName *name) :
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _write_vertex(0),
  _num_vertices(0),
  _writer(NULL)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexWriter::
~qpGeomVertexWriter() {
  if (_writer != (Writer *)NULL) {
    delete _writer;
    _writer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               writer is processing.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData *qpGeomVertexWriter::
get_vertex_data() const {
  return _vertex_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data_type
//       Access: Published
//  Description: Sets up the writer to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets the write vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexWriter::
set_data_type(int data_type) {
  return set_data_type(_vertex_data->get_format()->get_array_with(data_type),
                       _vertex_data->get_format()->get_data_type(data_type));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data_type
//       Access: Published
//  Description: Sets up the writer to use the data type with the
//               indicated name.
//
//               This also resets the write vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexWriter::
set_data_type(const string &name) {
  return set_data_type(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data_type
//       Access: Published
//  Description: Sets up the writer to use the data type with the
//               indicated name.
//
//               This also resets the write number to the start vertex
//               (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexWriter::
set_data_type(const InternalName *name) {
  return set_data_type(_vertex_data->get_format()->get_array_with(name),
                       _vertex_data->get_format()->get_data_type(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::has_data_type
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexWriter::
has_data_type() const {
  return (_data_type != (qpGeomVertexDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the writer is working on.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexWriter::
get_array() const {
  return _array;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_data_type
//       Access: Published
//  Description: Returns the description of the data type that the
//               writer is working on.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexDataType *qpGeomVertexWriter::
get_data_type() const {
  return _data_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_vertex
//       Access: Published
//  Description: Sets the start, write, and write index to the
//               indicated value.  The writer will begin traversing
//               from the given vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_vertex(int vertex) {
  _start_vertex = vertex;
  if (_data_type != (qpGeomVertexDataType *)NULL) {
    set_pointer(_start_vertex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_start_vertex
//       Access: Published
//  Description: Returns the vertex index at which the writer
//               started.  It will return to this vertex if you reset
//               the current data_type.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexWriter::
get_start_vertex() const {
  return _start_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_write_vertex
//       Access: Published
//  Description: Returns the current write vertex index of the
//               writer.  This is the index whose value will be
//               returned by the next call to get_data*().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexWriter::
get_write_vertex() const {
  return _write_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices in the vertex data.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexWriter::
get_num_vertices() const {
  return _num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::is_at_end
//       Access: Published
//  Description: Returns true if the writer is currently at the end of
//               the list of vertices, false otherwise.  If this is
//               true, another call to set_data*() will result in a
//               crash, but another call to add_data*() will add a new
//               vertex.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexWriter::
is_at_end() const {
  return _write_vertex >= _num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data1f
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data1f(float data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data1f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data2f(float x, float y) {
  set_data2f(LVecBase2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data2f(const LVecBase2f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data2f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data3f(float x, float y, float z) {
  set_data3f(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data3f(const LVecBase3f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data3f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data4f(float x, float y, float z, float w) {
  set_data4f(LVecBase4f(x, y, z, w));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data4f(const LVecBase4f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data4f(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data1i
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data1i(int data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data1i(inc_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data2i
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data2i(int a, int b) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data2i(inc_pointer(), a, b);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data2i
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data2i(const int data[2]) {
  set_data2i(data[0], data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data3i
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data3i(int a, int b, int c) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data3i(inc_pointer(), a, b, c);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data3i
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data3i(const int data[3]) {
  set_data3i(data[0], data[1], data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data4i
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data4i(int a, int b, int c, int d) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data4i(inc_pointer(), a, b, c, d);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data4i
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               It is an error for the write vertex to advance past
//               the end of data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_data4i(const int data[4]) {
  set_data4i(data[0], data[1], data[2], data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data1f
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data1f(float data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data1f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data2f(float x, float y) {
  add_data2f(LVecBase2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data2f
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data2f(const LVecBase2f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data2f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data3f(float x, float y, float z) {
  add_data3f(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data3f
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data3f(const LVecBase3f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data3f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data4f(float x, float y, float z, float w) {
  add_data4f(LVecBase4f(x, y, z, w));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data4f
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data4f(const LVecBase4f &data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data4f(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data1i
//       Access: Published
//  Description: Sets the write vertex to a particular 1-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data1i(int data) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data1i(inc_add_pointer(), data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data2i
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data2i(int a, int b) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data2i(inc_add_pointer(), a, b);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data2i
//       Access: Published
//  Description: Sets the write vertex to a particular 2-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data2i(const int data[2]) {
  add_data2i(data[0], data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data3i
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data3i(int a, int b, int c) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data3i(inc_add_pointer(), a, b, c);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data3i
//       Access: Published
//  Description: Sets the write vertex to a particular 3-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data3i(const int data[3]) {
  add_data3i(data[0], data[1], data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data4i
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data4i(int a, int b, int c, int d) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _writer->set_data4i(inc_add_pointer(), a, b, c, d);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::add_data4i
//       Access: Published
//  Description: Sets the write vertex to a particular 4-component
//               value, and advances the write vertex.
//
//               If the write vertex advances past the end of data,
//               implicitly adds a new vertex to the data.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
add_data4i(const int data[4]) {
  add_data4i(data[0], data[1], data[2], data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_pointer
//       Access: Private
//  Description: Sets up the internal write pointer, etc. to use the
//               indicated vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::
set_pointer(int vertex) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _write_vertex = vertex;
  PT(qpGeomVertexArrayData) array_data = _vertex_data->modify_array(_array);
  _pointer = array_data->modify_data() + _data_type->get_start() + _stride * _write_vertex;
  _num_vertices = array_data->get_num_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::inc_pointer
//       Access: Private
//  Description: Increments to the next vertex, and returns the data
//               pointer as it was before incrementing.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *qpGeomVertexWriter::
inc_pointer() {
  nassertr(_write_vertex < _num_vertices, NULL);
  nassertr(_pointer == _vertex_data->get_array(_array)->get_data() + _data_type->get_start() + _stride * _write_vertex, NULL);

  unsigned char *orig_pointer = _pointer;
  _pointer += _stride;
  ++_write_vertex;
  return orig_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::inc_add_pointer
//       Access: Private
//  Description: Increments to the next vertex, and returns the data
//               pointer as it was before incrementing.  If we are at
//               or past the end of data, implicitly adds more
//               vertices first.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *qpGeomVertexWriter::
inc_add_pointer() {
  if (_write_vertex >= _num_vertices) {
    // Reset the data pointer.
    _vertex_data->set_num_vertices(max(_write_vertex + 1, _vertex_data->get_num_vertices()));
    set_pointer(_write_vertex);
  }
  return inc_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::maybe_scale_color
//       Access: Public
//  Description: Converts a floating-point value to a uint8 value.  If
//               the contents value indicates this is a color value,
//               scales it into the range 0..255 per convention;
//               otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE unsigned int qpGeomVertexWriter::Writer::
maybe_scale_color(float data) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    return (unsigned int)(data * 255.0f);
    
  default:
    return (unsigned int)data;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::maybe_scale_color
//       Access: Public
//  Description: Converts an LVecBase2f into a pair of uint8
//               values.  See one-parameter maybe_scale_color() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::Writer::
maybe_scale_color(const LVecBase2f &data) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
    break;
    
  default:
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    break;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::maybe_scale_color
//       Access: Public
//  Description: Converts an LVecBase3f into a pair of uint8
//               values.  See one-parameter maybe_scale_color() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::Writer::
maybe_scale_color(const LVecBase3f &data) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
    _c = (unsigned int)(data[2] * 255.0f);
    break;
    
  default:
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
    break;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::maybe_scale_color
//       Access: Public
//  Description: Converts an LVecBase4f into a pair of uint8
//               values.  See one-parameter maybe_scale_color() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexWriter::Writer::
maybe_scale_color(const LVecBase4f &data) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
    _c = (unsigned int)(data[2] * 255.0f);
    _d = (unsigned int)(data[3] * 255.0f);
    break;
    
  default:
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
    _d = (unsigned int)data[3];
    break;
  }
}
