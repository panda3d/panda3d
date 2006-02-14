// Filename: geomVertexArrayData.I
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
//     Function: GeomVertexArrayData::get_array_format
//       Access: Published
//  Description: Returns the format object that describes this array.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomVertexArrayData::
get_array_format() const {
  return _array_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that describes to the
//               rendering backend how often the vertex data will be
//               modified and/or rendered.  See geomEnums.h.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::UsageHint GeomVertexArrayData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::has_column
//       Access: Published
//  Description: Returns true if the array has the named column,
//               false otherwise.  This is really just a shortcut for
//               asking the same thing from the format.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexArrayData::
has_column(const InternalName *name) const {
  return _array_format->has_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_num_rows
//       Access: Published
//  Description: Returns the number of rows stored in the array,
//               based on the number of bytes and the stride.  This
//               should be the same for all arrays within a given
//               GeomVertexData object.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayData::
get_num_rows() const {
  return get_data_size_bytes() / _array_format->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::clear_rows
//       Access: Published
//  Description: Removes all of the rows in the array.
//               Functionally equivalent to set_num_rows(0).
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexArrayData::
clear_rows() {
  set_data(PTA_uchar());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the array.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexArrayData::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the array vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomVertexArrayData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::get_data
//       Access: Public
//  Description: Returns a const pointer to the actual vertex data,
//               for application code to directly examine (but not
//               modify).
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar GeomVertexArrayData::
get_data() const {
  CDReader cdata(_cycler);
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::CData::
CData() :
  _usage_hint(UH_unspecified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexArrayData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData::CData::
CData(const GeomVertexArrayData::CData &copy) :
  _usage_hint(copy._usage_hint),
  _data(copy._data),
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const GeomVertexArrayData &obj) {
  obj.output(out);
  return out;
}
