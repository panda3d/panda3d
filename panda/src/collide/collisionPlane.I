// Filename: collisionPlane.I
// Created by:  drose (25Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Default Constructor
//       Access: Protected
//  Description: This is only for the convenience of CollisionPolygon.
//               Normally, you should not attempt to create an
//               uninitialized CollisionPlane.
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane(const Planef &plane) :
  _plane(plane)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane(const CollisionPlane &copy) :
  CollisionSolid(copy),
  _plane(copy._plane)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::get_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3f CollisionPlane::
get_normal() const {
  return _plane.get_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::dist_to_plane
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionPlane::
dist_to_plane(const LPoint3f &point) const {
  return _plane.dist_to_plane(point);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::set_plane
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionPlane::
set_plane(const Planef &plane) {
  _plane = plane;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::get_plane
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Planef &CollisionPlane::
get_plane() const {
  return _plane;
}
