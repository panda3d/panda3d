// Filename: CollisionHandlerGravity.I
// Created by:  drose (16Mar02)
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
//     Function: CollisionHandlerGravity::set_offset
//       Access: Public
//  Description: Sets the linear offset to add to (or subtract from)
//               the highest detected collision point to determine the
//               actual height at which to set the collider.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
set_offset(float offset) {
  _offset = offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_offset
//       Access: Public
//  Description: Returns the linear offset to add to (or subtract from)
//               the highest detected collision point to determine the
//               actual height at which to set the collider.
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::set_reach
//       Access: Public
//  Description: Sets the reach to add to (or subtract from)
//               the highest collision point
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
set_reach(float reach) {
  _reach = reach;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_reach
//       Access: Public
//  Description: Returns the reach to add to (or subtract from)
//               the highest collision point
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_reach() const {
  return _reach;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_airborne_height
//       Access: Public
//  Description: Return the height of the object from the ground.
//
//               The object might not necessarily be at rest.  Use
//               is_on_ground() if you want to know whether the
//               object is on the ground and at rest.
//
//               See Also: is_in_outer_space()
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_airborne_height() const {
  return _airborne_height;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::is_on_ground
//       Access: Public
//  Description: Is the object at rest?
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerGravity::
is_on_ground() const {
  // Testing for 0.0f here is not as foolhardy as it may appear.  The
  // handle_entries() function will set these values to 0.0f if they
  // are within a threshold.
  return get_airborne_height() == 0.0f && _current_velocity == 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_impact_velocity
//       Access: Public
//  Description: How hard did the object hit the ground.
//               This value is set on impact with the ground.
//               You may want to watch (poll) on is_on_groun() and
//               when that is true, call get_impact_velocity().
//               Normally I avoid polling, but we are calling
//               is_on_ground() frequently anyway.
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_impact_velocity() const {
  return _impact_velocity;
}

////////////////////////////////////////////////////////////////////
//    Function : CollisionHandlerGravity::get_contact_normal
//      Access : Public
// Description : 
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionHandlerGravity::
get_contact_normal() const {
  return _contact_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::add_velocity
//       Access: Public
//  Description: Adds the sepcified amount to the current velocity.
//               This is mostly here allow this common operation to
//               be faster for scripting, but it's also more concise
//               even in cpp.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
add_velocity(float velocity) {
  _current_velocity += velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::set_velocity
//       Access: Public
//  Description: Sets the current vertical velocity.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
set_velocity(float velocity) {
  _current_velocity = velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_velocity
//       Access: Public
//  Description: Gets the current vertical velocity.
//
//               Generally, negative values mean the object is in
//               free fall; while postive values mean the object has
//               vertical thrust.
//
//               A zero value does not necessarily mean the object
//               on the ground, it may also be weightless and/or at
//               the apex of its jump.
//
//               See Also: is_on_ground() and get_gravity()
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_velocity() const {
  return _current_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::set_gravity
//       Access: Public
//  Description: Sets the linear gravity force (always plumb).
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
set_gravity(float gravity) {
  _gravity = gravity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_gravity
//       Access: Public
//  Description: Gets the linear gravity force (always plumb).
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_gravity() const {
  return _gravity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::set_max_velocity
//       Access: Public
//  Description: Sets the maximum speed at which the object will be
//               allowed to descend towards a floor below it, in units
//               per second.  Set this to zero to allow it to
//               instantly teleport any distance.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerGravity::
set_max_velocity(float max_velocity) {
  _max_velocity = max_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::get_max_velocity
//       Access: Public
//  Description: Retrieves the maximum speed at which the object will
//               be allowed to descend towards a floor below it, in
//               units per second.  See set_max_velocity().
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerGravity::
get_max_velocity() const {
  return _max_velocity;
}
