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
