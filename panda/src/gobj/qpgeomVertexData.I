// Filename: qpgeomVertexData.I
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
//     Function: qpGeomVertexData::get_format
//       Access: Published
//  Description: Returns a pointer to the GeomVertexFormat structure
//               that defines this data.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexData::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that was passed to the
//               constructor, and which will be passed to each array
//               data object created initially, and arrays created as
//               the result of a convert_to() operation.  See
//               GeomUsageHint.
//
//               However, each individual array may be replaced with a
//               different array object with an independent usage hint
//               specified, so there is no guarantee that the
//               individual arrays all have the same usage_hint.
////////////////////////////////////////////////////////////////////
INLINE qpGeomUsageHint::UsageHint qpGeomVertexData::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_num_vertices
//       Access: Published
//  Description: Sets the length of the array to n vertices in all of
//               the various arrays (presumably by adding vertices).
//               The new vertex data is uninitialized.
//
//               The return value is true if the number of vertices
//               was changed, false if the object already contained n
//               vertices (or if there was some error).
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
set_num_vertices(int n) {
  CDWriter cdata(_cycler);
  return do_set_num_vertices(n, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_num_arrays
//       Access: Published
//  Description: Returns the number of individual arrays stored within
//               the data.  This must match
//               get_format()->get_num_arrays().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexData::
get_num_arrays() const {
  CDReader cdata(_cycler);
  return cdata->_arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_array
//       Access: Published
//  Description: Returns a const pointer to the vertex data for the
//               indicated array, for application code to directly
//               examine (but not modify) the underlying vertex data.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexArrayData *qpGeomVertexData::
get_array(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);
  return cdata->_arrays[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_transform_blend_palette
//       Access: Published
//  Description: Returns a const pointer to the TransformBlendPalette
//               assigned to this data.  Vertices within the table
//               will index into this palette to indicate their
//               dynamic skinning information; this table is used when
//               the vertex animation is to be performed by the CPU
//               (but also see get_transform_palette()).
//
//               This will return NULL if the vertex data does not
//               have a TransformBlendPalette assigned (which implies
//               the vertices will not be animated by the CPU).
////////////////////////////////////////////////////////////////////
INLINE const TransformBlendPalette *qpGeomVertexData::
get_transform_blend_palette() const {
  CDReader cdata(_cycler);
  return cdata->_transform_blend_palette;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_transform_blend_palette
//       Access: Published
//  Description: Sets the TransformBlendPalette pointer to NULL,
//               removing the palette from the vertex data.  This
//               disables CPU-driven vertex animation.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexData::
clear_transform_blend_palette() {
  set_transform_blend_palette(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq qpGeomVertexData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CData::
CData(const qpGeomVertexData::CData &copy) :
  _arrays(copy._arrays),
  _transform_blend_palette(copy._transform_blend_palette),
  _computed_vertices(copy._computed_vertices),
  _computed_vertices_modified(copy._computed_vertices_modified),
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexData &obj) {
  obj.output(out);
  return out;
}
