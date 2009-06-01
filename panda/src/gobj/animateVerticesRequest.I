// Filename: animateVerticesRequest.I
// Created by:  pratt (20Nov07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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
//     Function: AnimateVerticesRequest::Constructor
//       Access: Published
//  Description: Create a new AnimateVerticesRequest.
////////////////////////////////////////////////////////////////////
INLINE AnimateVerticesRequest::
AnimateVerticesRequest(GeomVertexData *geom_vertex_data) :
  _geom_vertex_data(geom_vertex_data),
  _is_ready(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimateVerticesRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.
////////////////////////////////////////////////////////////////////
INLINE bool AnimateVerticesRequest::
is_ready() const {
  return _is_ready;
}
