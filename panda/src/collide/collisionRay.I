// Filename: collisionRay.I
// Created by:  drose (22Jun00)
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
//     Function: CollisionRay::Default Constructor
//       Access: Public
//  Description: Creates an invalid ray.  This isn't terribly useful;
//               it's expected that the user will subsequently adjust
//               the ray via set_origin()/set_direction() or
//               set_from_lens().
////////////////////////////////////////////////////////////////////
INLINE CollisionRay::
CollisionRay() :
  _origin(LPoint3f(0.0, 0.0, 0.0)),
  _direction(LVector3f(0.0, 0.0, 0.0))
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionRay::
CollisionRay(const LPoint3f &origin, const LVector3f &direction) :
  _origin(origin), _direction(direction)
{
  nassertv(_direction != LPoint3f::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionRay::
CollisionRay(float ox, float oy, float oz,
             float dx, float dy, float dz) :
  _origin(ox, oy, oz), _direction(dx, dy, dz)
{
  nassertv(_direction != LPoint3f::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionRay::
CollisionRay(const CollisionRay &copy) :
  CollisionSolid(copy),
  _origin(copy._origin),
  _direction(copy._direction)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_origin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionRay::
set_origin(const LPoint3f &origin) {
  _origin = origin;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_origin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionRay::
set_origin(float x, float y, float z) {
  set_origin(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::get_origin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionRay::
get_origin() const {
  return _origin;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_direction
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionRay::
set_direction(const LVector3f &direction) {
  _direction = direction;
  mark_bound_stale();
  mark_viz_stale();
  nassertv(_direction != LPoint3f::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_direction
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionRay::
set_direction(float x, float y, float z) {
  set_direction(LVector3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::get_direction
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionRay::
get_direction() const {
  return _direction;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_from_lens
//       Access: Public
//  Description: Accepts a LensNode and a 2-d point in the range
//               [-1,1].  Sets the CollisionRay so that it begins at
//               the LensNode's near plane and extends to
//               infinity, making it suitable for picking objects from
//               the screen given a camera and a mouse location.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionRay::
set_from_lens(LensNode *camera, float px, float py) {
  return set_from_lens(camera, LPoint2f(px, py));
}
