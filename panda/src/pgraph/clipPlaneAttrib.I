// Filename: clipPlaneAttrib.I
// Created by:  drose (11Jul02)
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
//     Function: ClipPlaneAttrib::Constructor
//       Access: Private
//  Description: Use ClipPlaneAttrib::make() to construct a new
//               ClipPlaneAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneAttrib::
ClipPlaneAttrib() {
  _operation = O_set;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_operation
//       Access: Published
//  Description: Returns the basic operation type of the ClipPlaneAttrib.
//               If this is O_set, the planes listed here completely
//               replace any planes that were already on.  If this is
//               O_add, the planes here are added to the set of of
//               planes that were already on, and if O_remove, the
//               planes here are removed from the set of planes that
//               were on.
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneAttrib::Operation ClipPlaneAttrib::
get_operation() const {
  return _operation;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_num_planes
//       Access: Published
//  Description: Returns the number of planes listed in the attribute.
////////////////////////////////////////////////////////////////////
INLINE int ClipPlaneAttrib::
get_num_planes() const {
  return _planes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_plane
//       Access: Published
//  Description: Returns the nth planes listed in the attribute.
////////////////////////////////////////////////////////////////////
INLINE PlaneNode *ClipPlaneAttrib::
get_plane(int n) const {
  nassertr(n >= 0 && n < (int)_planes.size(), (PlaneNode *)NULL);
  return _planes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::add_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane added to the list of planes.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) ClipPlaneAttrib::
add_plane(PlaneNode *plane) const {
  if (_operation == O_remove) {  
    return compose(make(O_remove, plane));
  } else {
    return compose(make(O_add, plane));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::remove_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane removed from the list of
//               planes.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) ClipPlaneAttrib::
remove_plane(PlaneNode *plane) const {
  if (_operation == O_remove) {  
    return compose(make(O_add, plane));
  } else {
    return compose(make(O_remove, plane));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::is_identity
//       Access: Published
//  Description: Returns true if this is an identity attrib: it does
//               not change the set of planes in use.
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
is_identity() const {
  return _operation != O_set && _planes.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::is_all_off
//       Access: Published
//  Description: Returns true if this attrib turns off all planes and
//               turns none on.
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
is_all_off() const {
  return _operation == O_set && _planes.empty();
}
