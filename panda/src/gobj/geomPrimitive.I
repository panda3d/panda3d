// Filename: geomPrimitive.I
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
//     Function: GeomPrimitive::get_shade_model
//       Access: Published
//  Description: Returns the ShadeModel hint for this primitive.
//               This is intended as a hint to the renderer to tell it
//               how the per-vertex colors and normals are applied.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive::ShadeModel GeomPrimitive::
get_shade_model() const {
  return _shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::set_shade_model
//       Access: Published
//  Description: Changes the ShadeModel hint for this primitive.
//               This is different from the ShadeModelAttrib that
//               might also be applied from the scene graph.  This
//               does not affect the shade model that is in effect
//               when rendering, but rather serves as a hint to the
//               renderer to tell it how the per-vertex colors and
//               normals on this primitive are applied.
////////////////////////////////////////////////////////////////////
INLINE void GeomPrimitive::
set_shade_model(GeomPrimitive::ShadeModel shade_model) {
  _shade_model = shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint for this primitive.  See
//               geomEnums.h.  This has nothing to do with the usage
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
INLINE GeomPrimitive::UsageHint GeomPrimitive::
get_usage_hint() const {
  return _usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_type
//       Access: Public
//  Description: Returns the numeric type of the index column.
//               Normally, this will be either NT_uint16 or NT_uint32.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive::NumericType GeomPrimitive::
get_index_type() const {
  return _index_type;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::is_composite
//       Access: Published
//  Description: Returns true if the primitive is a composite
//               primitive such as a tristrip or trifan, or false if
//               it is a fundamental primitive such as a collection of
//               triangles.
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitive::
is_composite() const {
  return (get_num_vertices_per_primitive() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::is_indexed
//       Access: Published
//  Description: Returns true if the primitive is indexed, false
//               otherwise.  An indexed primitive stores a table of
//               index numbers into its GeomVertexData, so that it can
//               reference the vertices in any order.  A nonindexed
//               primitive, on the other hand, stores only the first
//               vertex number and number of vertices used, so that it
//               can only reference the vertices consecutively.
////////////////////////////////////////////////////////////////////
INLINE bool GeomPrimitive::
is_indexed() const {
  return (_vertices != (GeomVertexArrayData *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertex vertices used by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_num_vertices() const {
  if (_vertices == (GeomVertexArrayData *)NULL) {
    return _num_vertices;
  } else {
    return _vertices->get_num_rows();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by all the
//               primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
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
//     Function: GeomPrimitive::get_primitive_num_faces
//       Access: Published
//  Description: Returns the number of triangles or other fundamental
//               type (such as line segments) represented by the nth
//               primitive in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_primitive_num_faces(int n) const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  if (num_vertices_per_primitive == 0) {
    return get_primitive_num_vertices(n) - get_min_num_vertices_per_primitive() + 1;
  } else {
    return 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_min_vertex
//       Access: Published
//  Description: Returns the minimum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_min_vertex() const {
  if (!_got_minmax) {
    ((GeomPrimitive *)this)->recompute_minmax();
  }
  
  return _min_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_max_vertex
//       Access: Published
//  Description: Returns the maximum vertex index number used by all
//               the primitives in this object.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_max_vertex() const {
  if (!_got_minmax) {
    ((GeomPrimitive *)this)->recompute_minmax();
  }
  return _max_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_data_size_bytes
//       Access: Published
//  Description: Returns the number of bytes stored in the vertices
//               array.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_data_size_bytes() const {
  return _vertices->get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex index array is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomPrimitive::
get_modified() const {
  return _modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_vertices
//       Access: Public
//  Description: Returns a const pointer to the vertex index array so
//               application code can read it directly.  This might
//               return NULL if the primitive is nonindexed.  Do not
//               attempt to modify the returned array; use
//               modify_vertices() or set_vertices() for this.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayData *GeomPrimitive::
get_vertices() const {
  return _vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_stride
//       Access: Public
//  Description: A convenience function to return the gap between
//               successive index numbers, in bytes, of the index
//               data.
////////////////////////////////////////////////////////////////////
INLINE int GeomPrimitive::
get_index_stride() const {
  nassertr(is_indexed(), 0);
  return get_vertices()->get_array_format()->get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_data
//       Access: Public
//  Description: A convenience function to return the actual data of
//               the index array.
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar GeomPrimitive::
get_data() const {
  nassertr(is_indexed(), CPTA_uchar());
  return get_vertices()->get_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_ends
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
INLINE CPTA_int GeomPrimitive::
get_ends() const {
  return _ends;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_mins
//       Access: Public
//  Description: Returns a const pointer to the primitive mins
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_mins() or set_mins() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a mins array.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayData *GeomPrimitive::
get_mins() const {
  nassertr(is_indexed(), NULL);
  if (!_got_minmax) {
    ((GeomPrimitive *)this)->recompute_minmax();
  }
  return _mins;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_maxs
//       Access: Public
//  Description: Returns a const pointer to the primitive maxs
//               array so application code can read it directly.  Do
//               not attempt to modify the returned array; use
//               modify_maxs() or set_maxs() for this.
//
//               Note that simple primitive types, like triangles, do
//               not have a maxs array.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayData *GeomPrimitive::
get_maxs() const {
  nassertr(is_indexed(), NULL);
  if (!_got_minmax) {
    ((GeomPrimitive *)this)->recompute_minmax();
  }
  return _maxs;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::get_index_format
//       Access: Protected
//  Description: Returns a registered format appropriate for using to
//               store the index table.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexArrayFormat) GeomPrimitive::
get_index_format() const {
  return GeomVertexArrayFormat::register_format
    (new GeomVertexArrayFormat(InternalName::get_index(), 1, 
                               get_index_type(), C_index));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPrimitive::make_index_data
//       Access: Protected
//  Description: Creates and returns a new, empty index table.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomVertexArrayData) GeomPrimitive::
make_index_data() const {
  return new GeomVertexArrayData(get_index_format(), get_usage_hint());
}

INLINE ostream &
operator << (ostream &out, const GeomPrimitive &obj) {
  obj.output(out);
  return out;
}
