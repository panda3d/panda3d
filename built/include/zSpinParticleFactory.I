/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zSpinParticleFactory.I
 * @author charles
 * @date 2000-08-16
 */

/**

 */
INLINE void ZSpinParticleFactory::
set_initial_angle(PN_stdfloat angle) {
  _initial_angle = angle;
}

/**

 */
INLINE void ZSpinParticleFactory::
set_final_angle(PN_stdfloat angle) {
  _final_angle = angle;
}

/**

 */
INLINE void ZSpinParticleFactory::
set_initial_angle_spread(PN_stdfloat spread) {
  _initial_angle_spread = spread;
}

/**

 */
INLINE void ZSpinParticleFactory::
set_final_angle_spread(PN_stdfloat spread) {
  _final_angle_spread = spread;
}

/**

 */
INLINE PN_stdfloat ZSpinParticleFactory::
get_initial_angle() const {
  return _initial_angle;
}

/**

 */
INLINE PN_stdfloat ZSpinParticleFactory::
get_final_angle() const {
  return _final_angle;
}

/**

 */
INLINE PN_stdfloat ZSpinParticleFactory::
get_initial_angle_spread() const {
  return _initial_angle_spread;
}

/**

 */
INLINE PN_stdfloat ZSpinParticleFactory::
get_final_angle_spread() const {
  return _final_angle_spread;
}

/**

 */
INLINE PN_stdfloat ZSpinParticleFactory::
get_angular_velocity() const {
  return _angular_velocity;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity(PN_stdfloat v) {
  _angular_velocity = v;
}

INLINE PN_stdfloat ZSpinParticleFactory::
get_angular_velocity_spread() const {
  return _angular_velocity_spread;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity_spread(PN_stdfloat spread) {
  _angular_velocity_spread = spread;
}


INLINE void ZSpinParticleFactory::
enable_angular_velocity(bool bEnabled) {
  _bUseAngularVelocity = bEnabled;
}

INLINE bool ZSpinParticleFactory::
get_angular_velocity_enabled() const {
  return _bUseAngularVelocity;
}
