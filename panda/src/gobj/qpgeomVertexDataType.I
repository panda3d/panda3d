// Filename: qpgeomVertexDataType.I
// Created by:  drose (06Mar05)
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
//     Function: qpGeomVertexDataType::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexDataType::
qpGeomVertexDataType(const qpGeomVertexDataType &copy) :
  _name(copy._name),
  _num_components(copy._num_components),
  _num_values(copy._num_values),
  _numeric_type(copy._numeric_type),
  _contents(copy._contents),
  _start(copy._start),
  _component_bytes(copy._component_bytes),
  _total_bytes(copy._total_bytes)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexDataType::
operator = (const qpGeomVertexDataType &copy) {
  _name = copy._name;
  _num_components = copy._num_components;
  _num_values = copy._num_values;
  _numeric_type = copy._numeric_type;
  _contents = copy._contents;
  _start = copy._start;
  _component_bytes = copy._component_bytes;
  _total_bytes = copy._total_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_name
//       Access: Published
//  Description: Returns the name of this particular data field,
//               e.g. "vertex" or "normal".  The name may be a
//               user-defined string, or it may be one of the standard
//               system-defined field types.  Only the system-defined
//               field types are used for the actual rendering.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *qpGeomVertexDataType::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_num_components
//       Access: Published
//  Description: Returns the number of components of each data type:
//               the number of instances of the NumericType in each
//               element.  This is usually, but not always, the same
//               thing as get_num_values().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_num_values
//       Access: Published
//  Description: Returns the number of numeric values of each data
//               type: the number of distinct numeric values that go
//               into each element.  This is usually, but not always,
//               the same thing as get_num_components(); the
//               difference is in the case of a composite numeric type
//               like NT_packed_8888, which has four numeric values
//               per component.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
get_num_values() const {
  return _num_values;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_numeric_type
//       Access: Published
//  Description: Returns the token representing the numeric type of
//               the data storage.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexDataType::NumericType qpGeomVertexDataType::
get_numeric_type() const {
  return _numeric_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_contents
//       Access: Published
//  Description: Returns the token representing the semantic meaning of
//               the stored value.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexDataType::Contents qpGeomVertexDataType::
get_contents() const {
  return _contents;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_start
//       Access: Published
//  Description: Returns the byte within the array record at which
//               this data type starts.  This can be set to non-zero
//               to implement interleaved arrays.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
get_start() const {
  return _start;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_component_bytes
//       Access: Published
//  Description: Returns the number of bytes used by each component
//               (that is, by one element of the numeric type).
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
get_component_bytes() const {
  return _component_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_total_bytes
//       Access: Published
//  Description: Returns the number of bytes used by the data type:
//               component_bytes * num_components.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
get_total_bytes() const {
  return _total_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::overlaps_with
//       Access: Published
//  Description: Returns true if this data type overlaps with any of
//               the bytes in the indicated range, false if it does
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexDataType::
overlaps_with(int start_byte, int num_bytes) const {
  return (_start < start_byte + num_bytes &&
          _start + _total_bytes > start_byte);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::compare_to
//       Access: Public
//  Description: This is used to unquify data types, and hence
//               formats, for the GeomVertexFormat registry.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexDataType::
compare_to(const qpGeomVertexDataType &other) const {
  if (_name != other._name) {
    return _name < other._name ? -1 : 1;
  }
  if (_num_components != other._num_components) {
    return _num_components - other._num_components;
  }
  if (_numeric_type != other._numeric_type) {
    return (int)_numeric_type - (int)other._numeric_type;
  }
  if (_contents != other._contents) {
    return (int)_contents - (int)other._contents;
  }
  if (_start != other._start) {
    return _start - other._start;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::operator ==
//       Access: Public
//  Description: Returns true if the two data types are exactly
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexDataType::
operator == (const qpGeomVertexDataType &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexDataType::
operator != (const qpGeomVertexDataType &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::operator <
//       Access: Public
//  Description: This is used to put data types in order within a
//               particular GeomVertexArrayFormat.  Note that it is
//               *not* in the same space as operator == and operator
//               !=.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexDataType::
operator < (const qpGeomVertexDataType &other) const {
  if (_start != other._start) { 
    return _start < other._start;
  }
  if (_total_bytes < other._total_bytes) {
    return _total_bytes < other._total_bytes;
  }
  return 0;
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexDataType &obj) {
  obj.output(out);
  return out;
}
