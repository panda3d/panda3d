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
