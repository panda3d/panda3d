// Filename: geom.I
// Created by:  drose (04Feb99)
// 
////////////////////////////////////////////////////////////////////

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
INLINE Geom::
VertexIterator Geom::
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
