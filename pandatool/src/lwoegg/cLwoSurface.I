// Filename: cLwoSurface.I
// Created by:  drose (25Apr01)
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
//     Function: CLwoSurface::get_name
//       Access: Public
//  Description: Returns the name of the surface.  Each surface in a
//               given Lightwave file should have a unique name.
////////////////////////////////////////////////////////////////////
INLINE const string &CLwoSurface::
get_name() const {
  return _surface->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::has_named_uvs
//       Access: Public
//  Description: Returns true if the surface is set up to reference
//               UV's stored on the vertices, by name (as opposed to
//               generated UV's, which is the more common Lightwave
//               case).  In this case, get_uv_name() can be called to
//               return the name of the UV's.
////////////////////////////////////////////////////////////////////
INLINE bool CLwoSurface::
has_named_uvs() const {
  return (_block != (CLwoSurfaceBlock *)NULL &&
          _block->_projection_mode == LwoSurfaceBlockProjection::M_uv);
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::get_uv_name
//       Access: Public
//  Description: Returns the name of the set of UV's that are
//               associated with this surface, if has_named_uvs() is
//               true.
////////////////////////////////////////////////////////////////////
INLINE const string &CLwoSurface::
get_uv_name() const {
  return _block->_uv_name;
}
