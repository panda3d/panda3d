/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionVisualizer.I
 * @author drose
 * @date 2003-04-17
 */

/**
 *
 */
INLINE CollisionVisualizer::SolidInfo::
SolidInfo() {
  _detected_count = 0;
  _missed_count = 0;
}


/**
 * Scales the points that are drawn to represent the surface and interior
 * intersection points of the collisions.  By default, these objects are drawn
 * at an arbitrary scale which is appropriate if the window units are the
 * default range -1 .. 1.  Change this scale accordinatly if the window units
 * are measured on some other scale or if you need to observe these objects in
 * a smaller window.
 */
INLINE void CollisionVisualizer::
set_point_scale(PN_stdfloat point_scale) {
  LightMutexHolder holder(_lock);
  _point_scale = point_scale;
}

/**
 * Returns the value last set by set_point_scale().
 */
INLINE PN_stdfloat CollisionVisualizer::
get_point_scale() const {
  LightMutexHolder holder(_lock);
  return _point_scale;
}

/**
 * Scales the line segments that are drawn to represent the normals of the
 * collisions.  By default, these objects are drawn at an arbitrary scale
 * which is appropriate if the scene units are measured in feet.  Change this
 * scale accordinatly if the scene units are measured on some other scale or
 * if you need to observe these normals from farther away.
 */
INLINE void CollisionVisualizer::
set_normal_scale(PN_stdfloat normal_scale) {
  LightMutexHolder holder(_lock);
  _normal_scale = normal_scale;
}

/**
 * Returns the value last set by set_normal_scale().
 */
INLINE PN_stdfloat CollisionVisualizer::
get_normal_scale() const {
  LightMutexHolder holder(_lock);
  return _normal_scale;
}

/**
 * This is provided to disambiguate the typecast to TypedObject, since we have
 * two TypedObjects in our inheritance chain.
 */
INLINE TypedObject *CollisionVisualizer::
as_typed_object() {
  // In fact, it really doesn't matter which one we pick.  Arbitrarily pick
  // the one that goes through PandaNode.
  return PandaNode::as_typed_object();
}

/**
 * This is provided to disambiguate the typecast to TypedObject, since we have
 * two TypedObjects in our inheritance chain.
 */
INLINE const TypedObject * CollisionVisualizer::
as_typed_object() const {
  // In fact, it really doesn't matter which one we pick.  Arbitrarily pick
  // the one that goes through PandaNode.
  return PandaNode::as_typed_object();
}
