// Filename: qpgeomPrimitive.I
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
//     Function: qpGeomPrimitive::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertex vertices used by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_num_vertices() const {
  CDReader cdata(_cycler);
  return cdata->_vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_vertex
//       Access: Published
//  Description: Returns the ith vertex index in the table.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_vertex(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_vertices.size(), -1);
  return cdata->_vertices[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_vertices
//       Access: Published
//  Description: Returns a const pointer to the vertex index array so
//               application code can read it directly.  Do not
//               attempt to modify the returned array; use
//               modify_vertices() or set_vertices() for this.
////////////////////////////////////////////////////////////////////
INLINE CPTA_ushort qpGeomPrimitive::
get_vertices() const {
  CDReader cdata(_cycler);
  return cdata->_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_vertices
//       Access: Published
//  Description: Returns a const pointer to the primitive lengths
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_lengths() or set_lengths() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a lengths array: since all the primitives
//               have the same number of vertices, it is not needed.
////////////////////////////////////////////////////////////////////
INLINE CPTA_int qpGeomPrimitive::
get_lengths() const {
  CDReader cdata(_cycler);
  return cdata->_lengths;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_min_vertex() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    ((qpGeomPrimitive *)this)->recompute_minmax();
  }
  return cdata->_min_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_max_vertex() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    ((qpGeomPrimitive *)this)->recompute_minmax();
  }
  return cdata->_max_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive::CData::
CData() :
  _got_minmax(true),
  _min_vertex(0),
  _max_vertex(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive::CData::
CData(const qpGeomPrimitive::CData &copy) :
  _vertices(copy._vertices),
  _lengths(copy._lengths),
  _got_minmax(copy._got_minmax),
  _min_vertex(copy._min_vertex),
  _max_vertex(copy._max_vertex)
{
}
