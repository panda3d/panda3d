// Filename: collisionSolid.I
// Created by:  drose (27Jun00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::set_tangible
//       Access: Public
//  Description: Sets the current state of the 'tangible' flag.  Set
//               this true to make the solid tangible, so that a
//               CollisionHandlerPusher will not allow another object
//               to intersect it, or false to make it intangible, so
//               that a CollisionHandlerPusher will ignore it except
//               to throw an event.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
set_tangible(bool tangible) {
  if (tangible != _tangible) {
    _tangible = tangible;
    mark_viz_stale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::is_tangible
//       Access: Public
//  Description: Returns whether the solid is considered 'tangible' or
//               not.  An intangible solid has no effect in a
//               CollisionHandlerPusher (except to throw an event);
//               it's useful for defining 'trigger' planes and
//               spheres, that cause an effect when passed through.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionSolid::
is_tangible() const {
  return _tangible;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::mark_viz_stale
//       Access: Protected
//  Description: Called internally when the visualization may have
//               been compromised by some change to internal state and
//               will need to be recomputed the next time it is
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
mark_viz_stale() {
  _viz_stale = true;
}
