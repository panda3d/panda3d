// Filename: geom.I
// Created by:  drose (04Feb99)
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
//     Function: Geom::get_binding
//       Access: Published
//  Description: Returns the GeomBindType which indicates the
//               assignment of colors (or normals, etc.) to the
//               primitives.
////////////////////////////////////////////////////////////////////
INLINE GeomBindType Geom::
get_binding(int attr) const {
  return _bind[attr];
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::has_any_texcoords
//       Access: Published
//  Description: Returns true if the Geom defines any texture
//               coordinates at all, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Geom::
has_any_texcoords() const {
  return !_texcoords_by_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::has_texcoords
//       Access: Published
//  Description: Returns true if the Geom defines a set of texture
//               coordinates for the indicated multitexture stage
//               name, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Geom::
has_texcoords(const TexCoordName *name) const {
  TexCoordsByName::const_iterator tci = _texcoords_by_name.find(name);
  return (tci != _texcoords_by_name.end());
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_coords_array
//       Access: Published
//  Description: Returns the array of vertex coordinates associated
//               with the Geom.  This must have binding type
//               G_PER_VERTEX.  It may either be indexed or
//               nonindexed, depending on whether get_coords_index()
//               returns a NULL array.
////////////////////////////////////////////////////////////////////
INLINE PTA_Vertexf Geom::
get_coords_array() const {
  return _coords;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_normals_array
//       Access: Published
//  Description: Returns the array of normals associated with the
//               Geom.  This may have any binding type.  It may either
//               be indexed or nonindexed, depending on whether
//               get_normals_index() returns a NULL array.
////////////////////////////////////////////////////////////////////
INLINE PTA_Normalf Geom::
get_normals_array() const {
  return _norms;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_colors_array
//       Access: Published
//  Description: Returns the array of colors associated with the
//               Geom.  This may have any binding type.  It may either
//               be indexed or nonindexed, depending on whether
//               get_colors_index() returns a NULL array.
////////////////////////////////////////////////////////////////////
INLINE PTA_Colorf Geom::
get_colors_array() const {
  return _colors;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords_array
//       Access: Published
//  Description: Returns the array of texcoords associated with the
//               Geom.  This may have a binding type of G_PER_VERTEX
//               or G_OFF.  It may either be indexed or nonindexed,
//               depending on whether get_texcoords_index() returns a
//               NULL array.
//
//               This method is used in the single-texture case: it
//               returns the default texture coordinates only.  In the
//               presence of multitexturing, use the version of
//               get_texcoords_array() method that takes a
//               TexCoordName parameter.
////////////////////////////////////////////////////////////////////
INLINE PTA_TexCoordf Geom::
get_texcoords_array() const {
  return get_texcoords_array(TexCoordName::get_default());
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords_array
//       Access: Published
//  Description: Returns the texcoords array in the geom with the
//               indicated name.  This allows multitexturing support
//               with multiple sets of texture coordinates in the same
//               geom.  The return value will be NULL if there are no
//               texture coordinates with the indicated name.
////////////////////////////////////////////////////////////////////
INLINE PTA_TexCoordf Geom::
get_texcoords_array(const TexCoordName *name) const {
  TexCoordsByName::const_iterator tci = _texcoords_by_name.find(name);
  if (tci != _texcoords_by_name.end()) {
    return (*tci).second._texcoords;
  }
  return PTA_TexCoordf();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_coords_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in coords_array.
////////////////////////////////////////////////////////////////////
INLINE PTA_ushort Geom::
get_coords_index() const {
  return _vindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_normals_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in normals_array.
////////////////////////////////////////////////////////////////////
INLINE PTA_ushort Geom::
get_normals_index() const {
  return _nindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_colors_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in colors_array.
////////////////////////////////////////////////////////////////////
INLINE PTA_ushort Geom::
get_colors_index() const {
  return _cindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in texcoords_array.
//
//               This method is used in the single-texture case: it
//               returns the default texture coordinates only.  In the
//               presence of multitexturing, use the version of
//               get_texcoords_array() method that takes a
//               TexCoordName parameter.
////////////////////////////////////////////////////////////////////
INLINE PTA_ushort Geom::
get_texcoords_index() const {
  return get_texcoords_index(TexCoordName::get_default());
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords_index
//       Access: Published
//  Description: Returns the texcoords index within the geom with the
//               indicated name.  This allows multitexturing support
//               with multiple sets of texture coordinates in the same
//               geom.  The return value will be NULL if there are no
//               texture coordinates with the indicated name, or if
//               the geom does not use indexed texcoords.
////////////////////////////////////////////////////////////////////
INLINE PTA_ushort Geom::
get_texcoords_index(const TexCoordName *name) const {
  TexCoordsByName::const_iterator tci = _texcoords_by_name.find(name);
  if (tci != _texcoords_by_name.end()) {
    return (*tci).second._tindex;
  }
  return PTA_ushort();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::are_texcoords_indexed
//       Access: Published
//  Description: Returns true if any texcoords on the Geom (and,
//               therefore, all of them) are indexed, false if there
//               are no texcoords on the Geom already or if the
//               texcoords on the Geom are nonindexed.
////////////////////////////////////////////////////////////////////
INLINE bool Geom::
are_texcoords_indexed() const {
  if (_texcoords_by_name.empty()) {
    return false;
  }
  TexCoordsByName::const_iterator tci = _texcoords_by_name.begin();
  return ((*tci).second._tindex != (ushort *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_num_prims
//       Access: Published
//  Description: Sets the number of primitives in the Geom.  The
//               meaning of this depends on the precise type of Geom;
//               generally, each prim is one triangle in a GeomTri, or
//               one tristrip in a GeomTristrip.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
set_num_prims(int num) {
  _numprims = num;
  if (!uses_components()) {
    // except for strips & fans with the length arrays, total verts will be simply this
    _num_vertices = _numprims*get_num_vertices_per_prim();
  }
  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_num_prims
//       Access: Published
//  Description: Returns the number of primitives in the Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_prims() const {
  return _numprims;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_lengths
//       Access: Published
//  Description: Sets the array that indicates the length (number of
//               vertices) of each primitive.  This array should have
//               get_num_prims() entries.  This only has meaning for
//               composite type Geoms, like a GeomTristrip; it is
//               ignored for simple Geoms, like a GeomTri.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
set_lengths(const PTA_int &lengths) {
  _primlengths = lengths;
  _num_vertices = sum_lengths(lengths);
  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_lengths
//       Access: Published
//  Description: Returns the array the indicates the length (number of
//               vertices) of each primitive.  This array will
//               generally only be defined for composite type Geoms,
//               like a GeomTristrip; it is meaningless for simple
//               Geoms.
////////////////////////////////////////////////////////////////////
INLINE PTA_int Geom::
get_lengths() const {
  return _primlengths;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices required by all all
//               the prims in the Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_vertices() const {
  return _num_vertices;
}



////////////////////////////////////////////////////////////////////
//
// make_vertex_iterator(), get_next_vertex()
// make_normal_iterator(), get_next_normal()
// make_texcoord_iterator(), get_next_texcoord()
// make_color_iterator(), get_next_color()
//
// These functions all work together to walk through the vertex (or
// normal, etc.) values associated with the Geom.  Begin with a call
// to make_vertex_iterator(), which returns an iterator value suitable
// for passing to get_next_vertex().  The first call to
// get_next_vertex() returns a const Vertexf &, which is the value of
// the first vertex.  Each subsequent call to get_next_vertex() will
// return the value of the next following vertex.
//
// The actual value of the vertex is returned, regardless of whether
// the vertex array is indexed or nonindexed.
//
// There is no end-of-array indicator.  It is up to the caller to know
// the length of the vertex array, and stop when the end is reached.
//
// Similar behavior is exhibited for normals, texcoords, and colors.
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_vertex_iterator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::VertexIterator Geom::
make_vertex_iterator() const {
  check_config();
  VertexIterator i;
  i._array = _coords;
  i._index = _vindex;
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Vertexf &Geom::
get_next_vertex(VertexIterator &viterator) const {
  return _get_vertex(viterator);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_normal_iterator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::NormalIterator Geom::
make_normal_iterator() const {
  check_config();
  NormalIterator i;
  i._array = _norms;
  i._index = _nindex;
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Normalf &Geom::
get_next_normal(NormalIterator &niterator) const {
  return _get_normal(niterator);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_texcoord_iterator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::TexCoordIterator Geom::
make_texcoord_iterator() const {
  return make_texcoord_iterator(TexCoordName::get_default());
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_texcoord_iterator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::TexCoordIterator Geom::
make_texcoord_iterator(const TexCoordName *texcoord_name) const {
  check_config();
  TexCoordIterator i;

  TexCoordsByName::const_iterator tci = 
    _texcoords_by_name.find(texcoord_name);
  if (tci != _texcoords_by_name.end()) {
    i._array = (*tci).second._texcoords;
    i._index = (*tci).second._tindex;;
  } else {
    i._array = NULL;
    i._index = NULL;
  }

  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_texcoord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TexCoordf &Geom::
get_next_texcoord(TexCoordIterator &tciterator) const {
  return _get_texcoord(tciterator);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_texcoord
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TexCoordf &Geom::
get_next_multitexcoord(MultiTexCoordIterator &tciterator, int n) const {
  return _get_texcoord(tciterator._stages[n]);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_color_iterator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::ColorIterator Geom::
make_color_iterator() const {
  check_config();
  ColorIterator i;
  i._array = _colors;
  i._index = _cindex;
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Geom::
get_next_color(ColorIterator &citerator) const {
  return _get_color(citerator);
}
