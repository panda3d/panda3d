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
//     Function: Geom::get_coords_array
//       Access: Published
//  Description: Returns the array of vertex coordinates associated
//               with the Geom.  This must have binding type
//               G_PER_VERTEX.  It may either be indexed or
//               nonindexed, depending on whether get_coords_index()
//               returns a NULL array.
////////////////////////////////////////////////////////////////////
INLINE const PTA_Vertexf &Geom::
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
INLINE const PTA_Normalf &Geom::
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
INLINE const PTA_Colorf &Geom::
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
////////////////////////////////////////////////////////////////////
INLINE const PTA_TexCoordf &Geom::
get_texcoords_array() const {
  return _texcoords;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_coords_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in coords_array.
////////////////////////////////////////////////////////////////////
INLINE const PTA_ushort &Geom::
get_coords_index() const {
  return _vindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_normals_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in normals_array.
////////////////////////////////////////////////////////////////////
INLINE const PTA_ushort &Geom::
get_normals_index() const {
  return _nindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_colors_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in colors_array.
////////////////////////////////////////////////////////////////////
INLINE const PTA_ushort &Geom::
get_colors_index() const {
  return _cindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords_index
//       Access: Published
//  Description: Returns the array of indices that, if nonempty, will
//               be used to traverse the vertices in texcoords_array.
////////////////////////////////////////////////////////////////////
INLINE const PTA_ushort &Geom::
get_texcoords_index() const {
  return _tindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_num_prims
//       Access: Public
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
//       Access: Public
//  Description: Returns the number of primitives in the Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_prims(void) const {
  return _numprims;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_num_vertices
//       Access: Public
//  Description: Returns the number of vertices required by all all
//               the prims in the Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_vertices() const {
  return _num_vertices;
}

INLINE int PTA_int_arraysum(const PTA_int &lengths) {
    assert(lengths.size()>0);
    
    int *pLen=&lengths[0];
    int *pArrayEnd=pLen+lengths.size();
    int nVerts = 0;
    for (;pLen<pArrayEnd;pLen++) {
      nVerts += *pLen;
    }
    return nVerts;
}


////////////////////////////////////////////////////////////////////
//     Function: Geom::set_lengths
//       Access: Public
//  Description: Sets the array that indicates the length (number of
//               vertices) of each primitive.  This array should have
//               get_num_prims() entries.  This only has meaning for
//               composite type Geoms, like a GeomTristrip; it is
//               ignored for simple Geoms, like a GeomTri.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
set_lengths(const PTA_int &lengths) {
  _primlengths = lengths;
  _num_vertices = PTA_int_arraysum(lengths);
  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_lengths
//       Access: Public
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
  check_config();
  TexCoordIterator i;
  i._array = _texcoords;
  i._index = _tindex;
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
