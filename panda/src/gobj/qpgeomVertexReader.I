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
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _num_vertices(0),
  _reader(NULL)
{
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
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _num_vertices(0),
  _reader(NULL)
{
  set_data_type(name);
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
  _vertex_data(vertex_data),
  _array(0),
  _data_type(NULL),
  _pointer(NULL),
  _start_vertex(0),
  _read_vertex(0),
  _num_vertices(0),
  _reader(NULL)
{
  set_data_type(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexReader::
~qpGeomVertexReader() {
  if (_reader != (Reader *)NULL) {
    delete _reader;
    _reader = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               reader is processing.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexData *qpGeomVertexReader::
get_vertex_data() const {
  return _vertex_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_data_type
//       Access: Published
//  Description: Sets up the reader to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_data_type(int data_type) {
  return set_data_type(_vertex_data->get_format()->get_array_with(data_type),
                       _vertex_data->get_format()->get_data_type(data_type));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_data_type
//       Access: Published
//  Description: Sets up the reader to use the data type with the
//               indicated name.
//
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_data_type(const string &name) {
  return set_data_type(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_data_type
//       Access: Published
//  Description: Sets up the reader to use the data type with the
//               indicated name.
//
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
set_data_type(const InternalName *name) {
  return set_data_type(_vertex_data->get_format()->get_array_with(name),
                       _vertex_data->get_format()->get_data_type(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::has_data_type
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexReader::
has_data_type() const {
  return (_data_type != (qpGeomVertexDataType *)NULL);
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
//     Function: qpGeomVertexReader::get_data_type
//       Access: Published
//  Description: Returns the description of the data type that the
//               reader is working on.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexDataType *qpGeomVertexReader::
get_data_type() const {
  return _data_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_vertex
//       Access: Published
//  Description: Sets the start, read, and write index to the
//               indicated value.  The reader will begin traversing
//               from the given vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::
set_vertex(int vertex) {
  _start_vertex = vertex;
  if (_data_type != (qpGeomVertexDataType *)NULL) {
    set_pointer(_start_vertex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_start_vertex
//       Access: Published
//  Description: Returns the vertex index at which the reader
//               started.  It will return to this vertex if you reset
//               the current data_type.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_start_vertex() const {
  return _start_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_read_vertex
//       Access: Published
//  Description: Returns the current read vertex index of the
//               reader.  This is the index whose value will be
//               returned by the next call to get_data*().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_read_vertex() const {
  return _read_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices in the vertex data.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_num_vertices() const {
  return _num_vertices;
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
  return _read_vertex >= _num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data1f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 1-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE float qpGeomVertexReader::
get_data1f() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, 0.0f);
  return _reader->get_data1f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data2f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 2-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2f &qpGeomVertexReader::
get_data2f() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, LVecBase2f::zero());
  return _reader->get_data2f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data3f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 3-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &qpGeomVertexReader::
get_data3f() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, LVecBase3f::zero());
  return _reader->get_data3f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data4f
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 4-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &qpGeomVertexReader::
get_data4f() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, LVecBase4f::zero());
  return _reader->get_data4f(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data1i
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 1-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexReader::
get_data1i() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, 0);
  return _reader->get_data1i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data2i
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 2-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data2i() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, 0);
  return _reader->get_data2i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data3i
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 3-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data3i() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, 0);
  return _reader->get_data3i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_data4i
//       Access: Published
//  Description: Returns the data associated with the read vertex,
//               expressed as a 4-component value, and advances the
//               read vertex.
////////////////////////////////////////////////////////////////////
INLINE const int *qpGeomVertexReader::
get_data4i() {
  nassertr(_data_type != (qpGeomVertexDataType *)NULL, 0);
  return _reader->get_data4i(inc_pointer());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_pointer
//       Access: Private
//  Description: Sets up the internal write pointer, etc. to use the
//               indicated vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::
set_pointer(int vertex) {
  nassertv(_data_type != (qpGeomVertexDataType *)NULL);
  _read_vertex = vertex;
  CPT(qpGeomVertexArrayData) array_data = _vertex_data->get_array(_array);
  _pointer = array_data->get_data() + _data_type->get_start() + _stride * _read_vertex;
  _num_vertices = array_data->get_num_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::inc_pointer
//       Access: Private
//  Description: Increments to the next vertex, and returns the data
//               pointer as it was before incrementing.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *qpGeomVertexReader::
inc_pointer() {
  nassertr(_read_vertex < _num_vertices, NULL);
  nassertr(_pointer == _vertex_data->get_array(_array)->get_data() + _data_type->get_start() + _stride * _read_vertex, NULL);

  const unsigned char *orig_pointer = _pointer;
  _pointer += _stride;
  ++_read_vertex;
  return orig_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::maybe_scale_color
//       Access: Public
//  Description: Converts an integer (typically a uint8) value to a
//               floating-point value.  If the contents value
//               indicates this is a color value, scales it into the
//               range 0..1 per convention; otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE float qpGeomVertexReader::Reader::
maybe_scale_color(unsigned int value) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    return (float)value / 255.0f;
    
  default:
    return (float)value;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::maybe_scale_color
//       Access: Public
//  Description: Converts a pair of integers into the _v2 member.  See
//               one-parameter maybe_scale_color() for more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::Reader::
maybe_scale_color(unsigned int a, unsigned int b) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _v2.set((float)a / 255.0f,
            (float)b / 255.0f);
    break;
    
  default:
    _v2.set((float)a, (float)b);
    break;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::maybe_scale_color
//       Access: Public
//  Description: Converts a pair of integers into the _v3 member.  See
//               one-parameter maybe_scale_color() for more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::Reader::
maybe_scale_color(unsigned int a, unsigned int b, unsigned int c) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _v3.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f);
    break;
    
  default:
    _v3.set((float)a, (float)b, (float)c);
    break;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::maybe_scale_color
//       Access: Public
//  Description: Converts a pair of integers into the _v4 member.  See
//               one-parameter maybe_scale_color() for more info.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexReader::Reader::
maybe_scale_color(unsigned int a, unsigned int b, unsigned int c,
                  unsigned int d) {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_rgba:
  case qpGeomVertexDataType::C_argb:
    _v4.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f,
            (float)d / 255.0f);
    break;
    
  default:
    _v4.set((float)a, (float)b, (float)c, (float)d);
    break;
  }
}
    
