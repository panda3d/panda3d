// Filename: animateVerticesRequest.I
// Created by:  pratt (20Nov07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
