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
//     Function: qpGeomVertexArrayFormat::register_format
//       Access: Published, Static
//  Description: Adds the indicated format to the registry, if there
//               is not an equivalent format already there; in either
//               case, returns the pointer to the equivalent format
//               now in the registry.
//
//               This is similar to
//               GeomVertexFormat::register_format(), except that you
//               generally need not call it explicitly.  Calling
//               GeomVertexFormat::register_format() automatically
//               registers all of the nested array formats.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexArrayFormat) qpGeomVertexArrayFormat::
register_format(const qpGeomVertexArrayFormat *format) {
  return get_registry()->register_format((qpGeomVertexArrayFormat *)format);
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
//     Function: qpGeomVertexArrayFormat::get_num_columns
//       Access: Published
//  Description: Returns the number of different columns in the
//               array.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayFormat::
get_num_columns() const {
  return (int)_columns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_column
//       Access: Published
//  Description: Returns the ith column of the array.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexColumn *qpGeomVertexArrayFormat::
get_column(int i) const {
  nassertr(i >= 0 && i < (int)_columns.size(), NULL);
  consider_sort_columns();
  return _columns[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::has_column
//       Access: Published
//  Description: Returns true if the array has the named column,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexArrayFormat::
has_column(const InternalName *name) const {
  return (get_column(name) != (qpGeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::get_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexArrayFormat::Registry *qpGeomVertexArrayFormat::
get_registry() {
  if (_registry == (Registry *)NULL) {
    make_registry();
  }
  return _registry;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayFormat::consider_sort_columns
//       Access: Private
//  Description: Resorts the _columns vector if necessary.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexArrayFormat::
consider_sort_columns() const {
  if (_columns_unsorted) {
    ((qpGeomVertexArrayFormat *)this)->sort_columns();
  }
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexArrayFormat &obj) {
  obj.output(out);
  return out;
}
