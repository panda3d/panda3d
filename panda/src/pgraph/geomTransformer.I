// Filename: geomTransformer.I
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


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
//     Function: GeomTransformer::SourceVertices::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceVertices::
operator < (const GeomTransformer::SourceVertices &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
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
//     Function: GeomTransformer::SourceColors::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceColors::
operator < (const GeomTransformer::SourceColors &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
  }
  return (_color.compare_to(other._color) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceTextureColors::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceTextureColors::
operator < (const GeomTransformer::SourceTextureColors &other) const {
  if (_vertex_data != other._vertex_data) {
    return _vertex_data < other._vertex_data;
  }
  if (_tex != other._tex) {
    return _tex < other._tex;
  }
  if (_ts != other._ts) {
    return _ts < other._ts;
  }
  if (_tma != other._tma) {
    return _tma < other._tma;
  }
  if (_keep_vertex_color != other._keep_vertex_color) {
    return (int)_keep_vertex_color < (int)other._keep_vertex_color;
  }
  return (_base_color.compare_to(other._base_color) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceFormat::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceFormat::
operator < (const GeomTransformer::SourceFormat &other) const {
  if (_format != other._format) {
    return _format < other._format;
  }
  return _vertex_data < other._vertex_data;
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
  if (_animation_type != other._animation_type) {
    return (int)_animation_type < (int)other._animation_type;
  }
  return _name < other._name;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::VertexDataAssoc::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomTransformer::VertexDataAssoc::
VertexDataAssoc() {
  _might_have_unused = false;
}


