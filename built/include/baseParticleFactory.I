/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleFactory.I
 * @author charles
 * @date 2000-07-05
 */

/**
 * public
 */
INLINE void BaseParticleFactory::
set_lifespan_base(PN_stdfloat lb) {
  _lifespan_base = lb;
}

/**
 * public
 */
INLINE void BaseParticleFactory::
set_lifespan_spread(PN_stdfloat ld) {
  _lifespan_spread = ld;
}

/**
 * public
 */
INLINE void BaseParticleFactory::
set_mass_base(PN_stdfloat mb) {
  nassertv(mb >= _mass_spread);
  _mass_base = mb;
}

/**
 * public
 */
INLINE void BaseParticleFactory::
set_mass_spread(PN_stdfloat md) {
  nassertv(md < _mass_base);
  _mass_spread = md;
}

/**
 * public
 */
INLINE void BaseParticleFactory::
set_terminal_velocity_base(PN_stdfloat tvb) {
  _terminal_velocity_base = tvb;
}

/**
 * public
 */
INLINE void BaseParticleFactory::
set_terminal_velocity_spread(PN_stdfloat tvd) {
  _terminal_velocity_spread = tvd;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_lifespan_base() const {
  return _lifespan_base;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_lifespan_spread() const {
  return _lifespan_spread;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_mass_base() const {
  return _mass_base;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_mass_spread() const {
  return _mass_spread;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_terminal_velocity_base() const {
  return _terminal_velocity_base;
}

/**
 * public
 */
INLINE PN_stdfloat BaseParticleFactory::
get_terminal_velocity_spread() const {
  return _terminal_velocity_spread;
}
