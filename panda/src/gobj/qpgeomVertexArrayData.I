// Filename: qpgeomVertexArrayData.I
// Created by:  drose (17Mar05)
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
//     Function: qpGeomVertexArrayData::get_array_format
//       Access: Published
//  Description: Returns the format object that describes this array.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexArrayFormat *qpGeomVertexArrayData::
get_array_format() const {
  return _array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that describes to the
//               rendering backend how often the vertex data will be
//               modified and/or rendered.  This is provided as a
//               performance optimization only, and does not
//               constraing actual usage; although it may be an
//               important optimization.
//
//               This may only be specified to the GeomVertexArrayData
//               constructor.  If you need to change it, you must
//               create a new GeomVertexArrayData object (but you can
//               just assign the same data pointer to the new object).
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexArrayData::UsageHint qpGeomVertexArrayData::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::get_data
//       Access: Published
//  Description: Returns a const pointer to the actual vertex data,
//               for application code to directly examine (but not
//               modify).
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar qpGeomVertexArrayData::
get_data() const {
  CDReader cdata(_cycler);
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::modify_data
//       Access: Published
//  Description: Returns a modifiable pointer to the actual vertex
//               array, so that application code may directly
//               manipulate the vertices.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar qpGeomVertexArrayData::
modify_data() {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other GeomVertexData has the same
  // pointer, so make a copy of it first.
  CDWriter cdata(_cycler);

  if (cdata->_data.get_ref_count() > 1) {
    PTA_uchar orig_data = cdata->_data;
    cdata->_data = PTA_uchar();
    cdata->_data.v() = orig_data.v();
  }
  ++(cdata->_modified);

  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::set_data
//       Access: Published
//  Description: Replaces the vertex data array with a completely new
//               array.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexArrayData::
set_data(CPTA_uchar array) {
  CDWriter cdata(_cycler);
  cdata->_data = (PTA_uchar &)array;
  ++(cdata->_modified);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices stored in the array,
//               based on the number of bytes and the stride.  This
//               should be the same for all arrays within a given
//               GeomVertexData object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayData::
get_num_vertices() const {
  return get_num_bytes() / _array_format->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::clear_vertices
//       Access: Published
//  Description: Removes all of the vertices in the array.
//               Functionally equivalent to set_num_vertices(0).
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexArrayData::
clear_vertices() {
  set_data(PTA_uchar());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::get_num_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the array.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexArrayData::
get_num_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the array vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq qpGeomVertexArrayData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexArrayData::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexArrayData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexArrayData::CData::
CData(const qpGeomVertexArrayData::CData &copy) :
  _data(copy._data),
  _modified(copy._modified)
{
}
