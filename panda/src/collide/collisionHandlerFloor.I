// Filename: collisionHandlerFloor.I
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::set_offset
//       Access: Public
//  Description: Sets the linear offset to add to (or subtract from)
//               the highest detected collision point to determine the
//               actual height at which to set the collider.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerFloor::
set_offset(float offset) {
  _offset = offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::get_offset
//       Access: Public
//  Description: Returns the linear offset to add to (or subtract from)
//               the highest detected collision point to determine the
//               actual height at which to set the collider.
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerFloor::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::set_max_velocity
//       Access: Public
//  Description: Sets the maximum speed at which the object will be
//               allowed to descend towards a floor below it, in units
//               per second.  Set this to zero to allow it to
//               instantly teleport any distance.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerFloor::
set_max_velocity(float max_velocity) {
  _max_velocity = max_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::get_max_velocity
//       Access: Public
//  Description: Retrieves the maximum speed at which the object will
//               be allowed to descend towards a floor below it, in
//               units per second.  See set_max_velocity().
////////////////////////////////////////////////////////////////////
INLINE float CollisionHandlerFloor::
get_max_velocity() const {
  return _max_velocity;
}
