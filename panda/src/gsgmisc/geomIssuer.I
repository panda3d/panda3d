// Filename: geomIssuer.I
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
//     Function: GeomIssuer::issue_vertex
//       Access: Public
//  Description: Uses the function pointers set up by the constructor
//               to actually issue the vertex to the rendering
//               backend.  If the vertex does not have the indicated
//               bind type, nothing is done.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_vertex(GeomBindType bind, Geom::VertexIterator &i) {
  _vertex_command[bind](_geom, i, _gsg);
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
  _normal_command[bind](_geom, i, _gsg);
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
  _texcoord_command[bind](_geom, i, _gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::issue_texcoord
//       Access: Public
//  Description: Handles the case of multitexture coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GeomIssuer::
issue_texcoord(GeomBindType bind, Geom::MultiTexCoordIterator &i) {
  _multitexcoord_command[bind](_geom, i, _gsg);
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


