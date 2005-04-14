// Filename: geomTransformer.I
// Created by:  drose (14Mar02)
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
//     Function: GeomTransformer::set_usage_hint
//       Access: Published
//  Description: Specifies the ceiling UsageHint that will be applied
//               to transformed geometry.  If the source geometry's
//               usage hint is greater (more static) than this, it
//               will be reduced to this level.  If the source
//               geometry's usage hit is less (more dynamic) than
//               this, it will be preserved at its current level.
////////////////////////////////////////////////////////////////////
INLINE void GeomTransformer::
set_usage_hint(qpGeom::UsageHint usage_hint) {
  _usage_hint = usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::get_usage_hint
//       Access: Published
//  Description: Returns the UsageHint that will be applied to
//               generated geometry.  See set_usage_hint().
////////////////////////////////////////////////////////////////////
INLINE qpGeom::UsageHint GeomTransformer::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::get_max_collect_vertices
//       Access: Public
//  Description: Returns the maximum number of vertices that may be
//               put into a single GeomVertexData as a result of
//               collecting multiple objects in collect_vertex_data().
////////////////////////////////////////////////////////////////////
INLINE int GeomTransformer::
get_max_collect_vertices() const {
  return _max_collect_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::set_max_collect_vertices
//       Access: Public
//  Description: Specifies the maximum number of vertices that may be
//               put into a single GeomVertexData as a result of
//               collecting multiple objects in collect_vertex_data().
////////////////////////////////////////////////////////////////////
INLINE void GeomTransformer::
set_max_collect_vertices(int max_collect_vertices) {
  _max_collect_vertices = max_collect_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::qpSourceVertices::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::qpSourceVertices::
operator < (const GeomTransformer::qpSourceVertices &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::qpSourceTexCoords::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::qpSourceTexCoords::
operator < (const GeomTransformer::qpSourceTexCoords &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
  }
  if (_from != other._from) {
    return _from < other._from;
  }
  if (_to != other._to) {
    return _to < other._to;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::qpSourceColors::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::qpSourceColors::
operator < (const GeomTransformer::qpSourceColors &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
  }
  return (_color.compare_to(other._color) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceVertices::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceVertices::
operator < (const GeomTransformer::SourceVertices &other) const {
  if (_coords != other._coords) {
    return _coords < other._coords;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceNormals::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceNormals::
operator < (const GeomTransformer::SourceNormals &other) const {
  if (_norms != other._norms) {
    return _norms < other._norms;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceTexCoords::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceTexCoords::
operator < (const GeomTransformer::SourceTexCoords &other) const {
  if (_texcoords != other._texcoords) {
    return _texcoords < other._texcoords;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceColors::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceColors::
operator < (const GeomTransformer::SourceColors &other) const {
  if (_colors != other._colors) {
    return _colors < other._colors;
  }
  return (_scale.compare_to(other._scale) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedKey::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::NewCollectedKey::
operator < (const GeomTransformer::NewCollectedKey &other) const {
  if (_format != other._format) {
    return _format < other._format;
  }
  if (_usage_hint != other._usage_hint) {
    return (int)_usage_hint < (int)other._usage_hint;
  }
  return _name < other._name;
}
