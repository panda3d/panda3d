// Filename: collisionSolid.I
// Created by:  drose (27Jun00)
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
//     Function: CollisionSolid::set_tangible
//       Access: Published
//  Description: Sets the current state of the 'tangible' flag.  Set
//               this true to make the solid tangible, so that a
//               CollisionHandlerPusher will not allow another object
//               to intersect it, or false to make it intangible, so
//               that a CollisionHandlerPusher will ignore it except
//               to throw an event.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
set_tangible(bool tangible) {
  if (tangible) {
    _flags |= F_tangible;
  } else {
    _flags &= ~F_tangible;
  }
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::is_tangible
//       Access: Published
//  Description: Returns whether the solid is considered 'tangible' or
//               not.  An intangible solid has no effect in a
//               CollisionHandlerPusher (except to throw an event);
//               it's useful for defining 'trigger' planes and
//               spheres, that cause an effect when passed through.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionSolid::
is_tangible() const {
  return (_flags & F_tangible) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::set_effective_normal
//       Access: Published
//  Description: Records a false normal for this CollisionSolid that
//               will be reported by the collision system with all
//               collisions into it, instead of its actual normal.
//               This is useful as a workaround for the problem of an
//               avatar wanting to stand on a sloping ground; by
//               storing a false normal, the ground appears to be
//               perfectly level, and the avatar does not tend to
//               slide down it.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
set_effective_normal(const LVector3f &effective_normal) {
  _effective_normal = effective_normal;
  _flags |= F_effective_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::clear_effective_normal
//       Access: Published
//  Description: Removes the normal previously set by
//               set_effective_normal().
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
clear_effective_normal() {
  _flags &= ~F_effective_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::has_effective_normal
//       Access: Published
//  Description: Returns true if a special normal was set by
//               set_effective_normal(), false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionSolid::
has_effective_normal() const {
  return respect_effective_normal && (_flags & F_effective_normal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_effective_normal
//       Access: Published
//  Description: Returns the normal that was set by
//               set_effective_normal().  It is an error to call this
//               unless has_effective_normal() returns true.
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionSolid::
get_effective_normal() const {
  nassertr(has_effective_normal(), LVector3f::zero());
  return _effective_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::set_respect_effective_normal
//       Access: Published
//  Description: This is only meaningful for CollisionSolids that will
//               be added to a traverser as colliders.  It is normally
//               true, but if set false, it means that this particular
//               solid does not care about the "effective" normal of
//               other solids it meets, but rather always uses the
//               true normal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSolid::
set_respect_effective_normal(bool respect_effective_normal) {
  // For historical reasons, the bit we store is the opposite of the
  // bool flag we present.
  if (respect_effective_normal) {
    _flags &= ~F_ignore_effective_normal;
  } else {
    _flags |= F_ignore_effective_normal;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_respect_effective_normal
//       Access: Published
//  Description: See set_respect_effective_normal().
////////////////////////////////////////////////////////////////////
INLINE bool CollisionSolid::
get_respect_effective_normal() const {
  return (_flags & F_ignore_effective_normal) == 0;
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
  _flags |= F_viz_geom_stale;
}
