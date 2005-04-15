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
  if (_reader != (Reader *)NULL) {
    delete _reader;
    _reader = NULL;
  }
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
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
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
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
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
//               This also resets the read vertex number to the start
//               vertex (the same value passed to a previous call to
//               set_vertex(), or 0 if set_vertex() was never called.)
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
  return (_column != (qpGeomVertexColumn *)NULL);
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
  return _column;
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
  if (_column != (qpGeomVertexColumn *)NULL) {
    set_pointer(_start_vertex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::get_start_vertex
//       Access: Published
//  Description: Returns the vertex index at which the reader
//               started.  It will return to this vertex if you reset
//               the current column.
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, 0.0f);
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, LVecBase2f::zero());
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, LVecBase3f::zero());
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, LVecBase4f::zero());
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, 0);
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, 0);
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, 0);
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
  nassertr(_column != (qpGeomVertexColumn *)NULL, 0);
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
  nassertv(_column != (qpGeomVertexColumn *)NULL);
  _read_vertex = vertex;
  if (_vertex_data != (const qpGeomVertexData *)NULL) {
    _array_data = _vertex_data->get_array(_array);
  }
  _pointer = _array_data->get_data() + _column->get_start() + _stride * _read_vertex;
  _num_vertices = _array_data->get_num_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::inc_pointer
//       Access: Private
//  Description: Increments to the next vertex, and returns the data
//               pointer as it was before incrementing.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *qpGeomVertexReader::
inc_pointer() {
#ifndef NDEBUG
  nassertr(_read_vertex < _num_vertices, empty_buffer);
  if (_vertex_data != (const qpGeomVertexData *)NULL){ 
    nassertr(_pointer == _vertex_data->get_array(_array)->get_data() + _column->get_start() + _stride * _read_vertex, empty_buffer);
  } else {
    nassertr(_pointer == _array_data->get_data() + _column->get_start() + _stride * _read_vertex, empty_buffer);
  }
#endif

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
  if (_column->get_contents() == C_color) {
    return (float)value / 255.0f;
  } else {
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
  if (_column->get_contents() == C_color) {
    _v2.set((float)a / 255.0f,
            (float)b / 255.0f);
  } else {    
    _v2.set((float)a, (float)b);
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
  if (_column->get_contents() == C_color) {
    _v3.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f);
  } else {    
    _v3.set((float)a, (float)b, (float)c);
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
  if (_column->get_contents() == C_color) {
    _v4.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f,
            (float)d / 255.0f);
  } else {
    _v4.set((float)a, (float)b, (float)c, (float)d);
  }
}
    
