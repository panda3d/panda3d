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
//     Function: qpGeomPrimitive::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint for this primitive.  See
//               GeomUsageHint.  This has nothing to do with the usage
//               hint associated with the primitive's vertices; this
//               only specifies how often the vertex indices that
//               define the primitive will be modified.
//
//               It is perfectly legal (and, in fact, common) for a
//               GeomPrimitive to have UH_static on itself, while
//               referencing vertex data with UH_dynamic.  This means
//               that the vertices themselves will be animated, but
//               the primitive will always reference the same set of
//               vertices from the pool.
////////////////////////////////////////////////////////////////////
INLINE qpGeomUsageHint::UsageHint qpGeomPrimitive::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_shade_model
//       Access: Published
//  Description: Returns the ShadeModel hint for this primitive.
//               This is intended as a hint to the renderer to tell it
//               how the per-vertex colors and normals are applied.
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive::ShadeModel qpGeomPrimitive::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::set_shade_model
//       Access: Published
//  Description: Changes the ShadeModel hint for this primitive.
//               This is different from the ShadeModelAttrib that
//               might also be applied from the scene graph.  This
//               does not affect the shade model that is in effect
//               when rendering, but rather serves as a hint to the
//               renderer to tell it how the per-vertex colors and
//               normals on this primitive are applied.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomPrimitive::
set_shade_model(qpGeomPrimitive::ShadeModel shade_model) {
  CDWriter cdata(_cycler);
  cdata->_shade_model = shade_model;
  cdata->_rotated_vertices.clear();
}

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
//     Function: qpGeomPrimitive::get_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_num_faces() const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    int num_primitives = get_num_primitives();
    int num_vertices = get_num_vertices();
    int min_num_vertices_per_primitive = get_min_num_vertices_per_primitive();
    int num_unused_vertices_per_primitive = get_num_unused_vertices_per_primitive();
    return num_vertices - (num_primitives * (min_num_vertices_per_primitive - 1)) - ((num_primitives - 1) * num_unused_vertices_per_primitive);
  } else {
    return get_num_primitives();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by the nth
//               primitive in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_primitive_num_faces(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    return get_primitive_num_vertices(n) - get_min_num_vertices_per_primitive() + 1;
  } else {
    return 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_min_vertex() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    CDWriter cdataw(((qpGeomPrimitive *)this)->_cycler, cdata);
    ((qpGeomPrimitive *)this)->recompute_minmax(cdataw);
    return cdataw->_min_vertex;
  }
  return cdata->_min_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by the
//               nth primitive in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_primitive_min_vertex(int n) const {
  CPTA_ushort mins = get_mins();
  nassertr(n >= 0 && n < (int)mins.size(), -1);
  return mins[n];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_max_vertex() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    CDWriter cdataw(((qpGeomPrimitive *)this)->_cycler, cdata);
    ((qpGeomPrimitive *)this)->recompute_minmax(cdataw);
    return cdataw->_max_vertex;
  }
  return cdata->_max_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_primitive_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by the
//               nth primitive in this object.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_primitive_max_vertex(int n) const {
  CPTA_ushort maxs = get_maxs();
  nassertr(n >= 0 && n < (int)maxs.size(), -1);
  return maxs[n];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the vertices
//               array.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomPrimitive::
get_data_size_bytes() const {
  CDReader cdata(_cycler);
  return cdata->_vertices.size() * sizeof(short);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex index array is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq qpGeomPrimitive::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_vertices
//       Access: Public
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
//     Function: qpGeomPrimitive::get_flat_first_vertices
//       Access: Public
//  Description: Returns a const pointer to the vertex index array,
//               suitable for traversing for a renderer that expects
//               the key vertex of each triangle in a flat-shaded
//               model to be the triangle's first vertex (as DirectX
//               does).
//
//               The result of this is dependent on the value
//               specified to set_shade_model().  If the shade model
//               indicates flat_last_vertex, this will return the
//               vertices rotated to bring them into first_vertex
//               compliance; otherwise, it will be the unmodified
//               result from get_vertices().
////////////////////////////////////////////////////////////////////
INLINE CPTA_ushort qpGeomPrimitive::
get_flat_first_vertices() const {
  CDReader cdata(_cycler);
  if (cdata->_shade_model == SM_flat_last_vertex) {
    if (cdata->_rotated_vertices == (const ushort *)NULL) {
      return ((qpGeomPrimitive *)this)->do_rotate(cdata);
    }
    return cdata->_rotated_vertices;
  } else {
    return cdata->_vertices;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_flat_last_vertices
//       Access: Public
//  Description: Returns a const pointer to the vertex index array,
//               suitable for traversing for a renderer that expects
//               the key vertex of each triangle in a flat-shaded
//               model to be the triangle's last vertex (as OpenGL
//               does).
//
//               The result of this is dependent on the value
//               specified to set_shade_model().  If the shade model
//               indicates flat_first_vertex, this will return the
//               vertices rotated to bring them into last_vertex
//               compliance; otherwise, it will be the unmodified
//               result from get_vertices().
////////////////////////////////////////////////////////////////////
INLINE CPTA_ushort qpGeomPrimitive::
get_flat_last_vertices() const {
  CDReader cdata(_cycler);
  if (cdata->_shade_model == SM_flat_first_vertex) {
    if (cdata->_rotated_vertices == (const ushort *)NULL) {
      return ((qpGeomPrimitive *)this)->do_rotate(cdata);
    }
    return cdata->_rotated_vertices;
  } else {
    return cdata->_vertices;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_ends
//       Access: Public
//  Description: Returns a const pointer to the primitive ends
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_ends() or set_ends() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a ends array: since all the primitives
//               have the same number of vertices, it is not needed.
////////////////////////////////////////////////////////////////////
INLINE CPTA_int qpGeomPrimitive::
get_ends() const {
  CDReader cdata(_cycler);
  return cdata->_ends;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_mins
//       Access: Public
//  Description: Returns a const pointer to the primitive mins
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_mins() or set_mins() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a mins array.
////////////////////////////////////////////////////////////////////
INLINE CPTA_ushort qpGeomPrimitive::
get_mins() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    CDWriter cdataw(((qpGeomPrimitive *)this)->_cycler, cdata);
    ((qpGeomPrimitive *)this)->recompute_minmax(cdataw);
    return cdataw->_mins;
  }
  return cdata->_mins;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::get_maxs
//       Access: Public
//  Description: Returns a const pointer to the primitive maxs
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_maxs() or set_maxs() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a maxs array.
////////////////////////////////////////////////////////////////////
INLINE CPTA_ushort qpGeomPrimitive::
get_maxs() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_minmax) {
    CDWriter cdataw(((qpGeomPrimitive *)this)->_cycler, cdata);
    ((qpGeomPrimitive *)this)->recompute_minmax(cdataw);
    return cdataw->_maxs;
  }
  return cdata->_maxs;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPrimitive::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive::CData::
CData() :
  _shade_model(SM_smooth),
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
  _shade_model(copy._shade_model),
  _vertices(copy._vertices),
  _rotated_vertices(copy._rotated_vertices),
  _ends(copy._ends),
  _mins(copy._mins),
  _maxs(copy._maxs),
  _got_minmax(copy._got_minmax),
  _min_vertex(copy._min_vertex),
  _max_vertex(copy._max_vertex)
{
}
