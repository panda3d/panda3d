// Filename: cLwoSurface.I
// Created by:  drose (25Apr01)
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
//     Function: CLwoSurface::has_uvs
//       Access: Public
//  Description: Returns true if check_texture() has been called and
//               the surface is known to have UV's available, either
//               computed or stored.  If this is true, get_uv() may be
//               called to determine each UV coordinate.
////////////////////////////////////////////////////////////////////
INLINE bool CLwoSurface::
has_uvs() const {
  return _has_uvs;
}
