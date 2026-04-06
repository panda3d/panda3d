/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerGravity.I
 * @author drose
 * @date 2002-03-16
 */

/**
 * Sets the linear offset to add to (or subtract from) the highest detected
 * collision point to determine the actual height at which to set the
 * collider.
 */
INLINE void CollisionHandlerGravity::
set_offset(PN_stdfloat offset) {
  _offset = offset;
}

/**
 * Returns the linear offset to add to (or subtract from) the highest detected
 * collision point to determine the actual height at which to set the
 * collider.
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_offset() const {
  return _offset;
}

/**
 * Sets the reach to add to (or subtract from) the highest collision point
 */
INLINE void CollisionHandlerGravity::
set_reach(PN_stdfloat reach) {
  _reach = reach;
}

/**
 * Returns the reach to add to (or subtract from) the highest collision point
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_reach() const {
  return _reach;
}

/**
 * Return the height of the object from the ground.
 *
 * The object might not necessarily be at rest.  Use is_on_ground() if you
 * want to know whether the object is on the ground and at rest.
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_airborne_height() const {
  return _airborne_height;
}

/**
 * Is the object at rest?
 */
INLINE bool CollisionHandlerGravity::
is_on_ground() const {
  // Testing for 0.0f here is not as foolhardy as it may appear.  The
  // handle_entries() function will set these values to 0.0f if they are
  // within a threshold.
  return get_airborne_height() == 0.0f && _current_velocity == 0.0f;
}

/**
 * How hard did the object hit the ground.  This value is set on impact with
 * the ground.  You may want to watch (poll) on is_on_ground() and when that is
 * true, call get_impact_velocity(). Normally I avoid polling, but we are
 * calling is_on_ground() frequently anyway.
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_impact_velocity() const {
  return _impact_velocity;
}

/**
 *
 */
INLINE const LVector3 &CollisionHandlerGravity::
get_contact_normal() const {
  return _contact_normal;
}

/**
 * Adds the sepcified amount to the current velocity.  This is mostly here
 * allow this common operation to be faster for scripting, but it's also more
 * concise even in cpp.
 */
INLINE void CollisionHandlerGravity::
add_velocity(PN_stdfloat velocity) {
  _current_velocity += velocity;
}

/**
 * Sets the current vertical velocity.
 */
INLINE void CollisionHandlerGravity::
set_velocity(PN_stdfloat velocity) {
  _current_velocity = velocity;
}

/**
 * Gets the current vertical velocity.
 *
 * Generally, negative values mean the object is in free fall; while postive
 * values mean the object has vertical thrust.
 *
 * A zero value does not necessarily mean the object on the ground, it may
 * also be weightless and/or at the apex of its jump.
 *
 * See Also: is_on_ground() and get_gravity()
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_velocity() const {
  return _current_velocity;
}

/**
 * Sets the linear gravity force (always plumb).
 */
INLINE void CollisionHandlerGravity::
set_gravity(PN_stdfloat gravity) {
  _gravity = gravity;
}

/**
 * Gets the linear gravity force (always plumb).
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_gravity() const {
  return _gravity;
}

/**
 * Sets the maximum speed at which the object will be allowed to descend
 * towards a floor below it, in units per second.  Set this to zero to allow
 * it to instantly teleport any distance.
 */
INLINE void CollisionHandlerGravity::
set_max_velocity(PN_stdfloat max_velocity) {
  _max_velocity = max_velocity;
}

/**
 * Retrieves the maximum speed at which the object will be allowed to descend
 * towards a floor below it, in units per second.  See set_max_velocity().
 */
INLINE PN_stdfloat CollisionHandlerGravity::
get_max_velocity() const {
  return _max_velocity;
}

/**
 * Enables old behavior required by Toontown (Sellbot Factory lava room is
 * good test case, lava and conveyor belt specifically). Behavior is to throw
 * enter/exit events only for floor that the toon is in contact with
 */
INLINE void CollisionHandlerGravity::
set_legacy_mode(bool legacy_mode) {
  _legacy_mode = legacy_mode;
}

/**
 * returns true if legacy mode is enabled
 */
INLINE bool CollisionHandlerGravity::
get_legacy_mode() const {
  return _legacy_mode;
}
