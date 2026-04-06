/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * emission type assignment
 */
INLINE void BaseParticleEmitter::
set_emission_type(emissionType et) {
  _emission_type = et;
}

/**
 * emission type query
 */
INLINE BaseParticleEmitter::emissionType BaseParticleEmitter::
get_emission_type() const {
  return _emission_type;
}

/**
 * assignment of explicit emission launch vector
 */
INLINE void BaseParticleEmitter::
set_explicit_launch_vector(const LVector3& elv) {
  _explicit_launch_vector = elv;
}

/**
 * query for explicit emission launch vector
 */
INLINE LVector3 BaseParticleEmitter::
get_explicit_launch_vector() const {
  return _explicit_launch_vector;
}

/**
 * assignment of radiate emission origin point
 */
INLINE void BaseParticleEmitter::
set_radiate_origin(const LPoint3& ro) {
  _radiate_origin = ro;
}

/**
 * query for explicit emission launch vector
 */
INLINE LPoint3 BaseParticleEmitter::
get_radiate_origin() const {
  return _radiate_origin;
}

/**
 * amplitude assignment
 */
INLINE void BaseParticleEmitter::
set_amplitude(PN_stdfloat a) {
  _amplitude = a;
}

/**
 * amplitude query
 */
INLINE PN_stdfloat BaseParticleEmitter::
get_amplitude() const {
  return _amplitude;
}

/**
 * amplitude spread assignment
 */
INLINE void BaseParticleEmitter::
set_amplitude_spread(PN_stdfloat as) {
  _amplitude_spread = as;
}

/**
 * amplitude spread query
 */
INLINE PN_stdfloat BaseParticleEmitter::
get_amplitude_spread() const {
  return _amplitude_spread;
}

/**
 * user-defined force
 */
INLINE void BaseParticleEmitter::
set_offset_force(const LVector3& of) {
  _offset_force = of;
}

/**
 * user-defined force
 */
INLINE LVector3 BaseParticleEmitter::
get_offset_force() const {
  return _offset_force;
}
