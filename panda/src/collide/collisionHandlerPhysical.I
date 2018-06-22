/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPhysical.I
 * @author drose
 * @date 2002-03-16
 */

/**
 * Specifies an arbitrary NodePath that the handler is always considered to be
 * facing.  It does not detect collisions with surfaces that appear to be
 * facing away from this NodePath.  This works best when the collision
 * surfaces in question are polygons.
 */
INLINE void CollisionHandlerPhysical::
set_center(const NodePath &center) {
  _center = center;
}

/**
 * Clears the center NodePath specified with set_center.
 */
INLINE void CollisionHandlerPhysical::
clear_center() {
  _center = NodePath();
}

/**
 * Returns the NodePath specified with set_center, or the empty NodePath if
 * nothing has been specified.
 */
INLINE const NodePath &CollisionHandlerPhysical::
get_center() const {
  return _center;
}

/**
 * Returns true if a NodePath has been specified with set_center(), false
 * otherwise.
 */
INLINE bool CollisionHandlerPhysical::
has_center() const {
  return !_center.is_empty();
}

/**
 * Did the handler make any contacts with anything on the last collision pass?
 * Depending on how your world is setup, this can be used to tell if the
 * handler is out of the world (i.e.  out of bounds). That is the original use
 * of this call.
 */
INLINE bool CollisionHandlerPhysical::
has_contact() const {
  return _has_contact;
}

/**
 *
 */
INLINE void CollisionHandlerPhysical::ColliderDef::
set_target(const NodePath &target, DriveInterface *drive_interface) {
  _target = target;
  _drive_interface = drive_interface;
}

/**
 * Called by the handler when it has changed the transform on the target node,
 * this applies the change to the drive interface if one is specified.
 */
INLINE void CollisionHandlerPhysical::ColliderDef::
updated_transform() {
  if (_drive_interface != nullptr) {
    _drive_interface->set_mat(_target.get_mat());
    _drive_interface->force_dgraph();
  }
}
