// Filename: qpgeomVertexArrayFormat.I
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
//     Function: qpGeomVertexArrayFormat::is_registered
//       Access: Published
//  Description: Returns true if this format has been registered,
//               false if it has not.  It may not be used for a Geom
//               until it has been registered, but once registered, it
//               may no longer be modified.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexArrayFormat::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_stride
//       Access: Published
//  Description: Returns the total number of bytes reserved in the
//               array for each vertex.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayFormat::
get_stride() const {
  return _stride;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::set_stride
//       Access: Published
//  Description: Changes the total number of bytes reserved in the
//               array for each vertex.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexArrayFormat::
set_stride(int stride) {
  nassertv(!_is_registered);
  _stride = stride;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_total_bytes
//       Access: Published
//  Description: Returns the total number of bytes used by the data
//               types within the format, including gaps between
//               elements.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayFormat::
get_total_bytes() const {
  return _total_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_pad_to
//       Access: Published
//  Description: Returns the byte divisor to which the data record
//               must be padded to meet hardware limitations.  For
//               instance, if this is 4, the stride will be
//               automatically rounded up to the next multiple of 4
//               bytes.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayFormat::
get_pad_to() const {
  return _pad_to;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_num_data_types
//       Access: Published
//  Description: Returns the number of different data types in the
//               specification.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayFormat::
get_num_data_types() const {
  return (int)_data_types.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_data_type
//       Access: Published
//  Description: Returns the ith data types of the specification.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexDataType *qpGeomVertexArrayFormat::
get_data_type(int i) const {
  nassertr(i >= 0 && i < (int)_data_types.size(), NULL);
  consider_sort_data_types();
  return _data_types[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::has_data_type
//       Access: Published
//  Description: Returns true if the format has the named data type,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexArrayFormat::
has_data_type(const InternalName *name) const {
  return (get_data_type(name) != (qpGeomVertexDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::consider_sort_data_types
//       Access: Private
//  Description: Resorts the _data_types vector if necessary.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexArrayFormat::
consider_sort_data_types() const {
  if (_data_types_unsorted) {
    ((qpGeomVertexArrayFormat *)this)->sort_data_types();
  }
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexArrayFormat &obj) {
  obj.output(out);
  return out;
}
