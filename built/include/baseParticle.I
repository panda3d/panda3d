/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticle.I
 * @author charles
 * @date 2000-06-16
 */

INLINE void BaseParticle::set_age(PN_stdfloat age) {
  _age = age;
}

INLINE void BaseParticle::set_lifespan(PN_stdfloat lifespan) {
  _lifespan = lifespan;
}

INLINE void BaseParticle::set_alive(bool alive) {
  _alive = alive;
}

INLINE void BaseParticle::set_index(int index) {
  _index = index;
}

INLINE PN_stdfloat BaseParticle::get_age() const {
  return _age;
}

INLINE PN_stdfloat BaseParticle::get_lifespan() const {
  return _lifespan;
}

INLINE bool BaseParticle::get_alive() const {
  return _alive;
}

INLINE int BaseParticle::get_index() const {
  return _index;
}

INLINE PN_stdfloat BaseParticle::get_parameterized_age() const {
  if (_lifespan <= 0) return 1.0;
  return _age / _lifespan;
}

INLINE PN_stdfloat BaseParticle::get_parameterized_vel() const {
  if (IS_NEARLY_ZERO(get_terminal_velocity())) return 0.0;
  return (get_velocity().length()) / get_terminal_velocity();
}
