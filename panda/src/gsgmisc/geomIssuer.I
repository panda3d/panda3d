// Filename: geomIssuer.I
// Created by:  drose (04Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::issue_vertex
//       Access: Public
//  Description: Uses the function pointers set up by the constructor
//               to actually issue the vertex to the rendering
//               backend.  If the vertex does not have the indicated
//               bind type, nothing is done.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_vertex(GeomBindType bind, Geom::VertexIterator &i) {
  _vertex_command[bind](_geom, i);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::issue_normal
//       Access: Public
//  Description: Uses the function pointers set up by the constructor
//               to actually issue the normal to the rendering
//               backend.  If the normal does not have the indicated
//               bind type, nothing is done.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_normal(GeomBindType bind, Geom::NormalIterator &i) {
  _normal_command[bind](_geom, i);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::issue_texcoord
//       Access: Public
//  Description: Uses the function pointers set up by the constructor
//               to actually issue the texCoord to the rendering
//               backend.  If the texCoord does not have the indicated
//               bind type, nothing is done.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_texcoord(GeomBindType bind, Geom::TexCoordIterator &i) {
  _texcoord_command[bind](_geom, i);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::issue_color
//       Access: Public
//  Description: Uses the function pointers set up by the constructor
//               to actually issue the color to the rendering
//               backend.  If the color does not have the indicated
//               bind type, nothing is done.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_color(GeomBindType bind, Geom::ColorIterator &i) {
  _color_command[bind](_geom, i, _gsg);
}


