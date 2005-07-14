// Filename: baseParticle.I
// Created by:  charles (16Jun00)
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

INLINE void BaseParticle::set_age(float age) {
  _age = age;
}

INLINE void BaseParticle::set_lifespan(float lifespan) {
  _lifespan = lifespan;
}

INLINE void BaseParticle::set_alive(bool alive) {
  _alive = alive;
}

INLINE void BaseParticle::set_index(int index) {
  _index = index;
}

INLINE float BaseParticle::get_age() const {
  return _age;
}

INLINE float BaseParticle::get_lifespan() const {
  return _lifespan;
}

INLINE bool BaseParticle::get_alive() const {
  return _alive;
}

INLINE int BaseParticle::get_index() const {
  return _index;
}

INLINE float BaseParticle::get_parameterized_age() const {
  if (_lifespan <= 0) return 1.0;
  return _age / _lifespan;
}

INLINE float BaseParticle::get_parameterized_vel() const {
  if (IS_NEARLY_ZERO(get_terminal_velocity())) return 0.0;
  return (get_velocity().length()) / get_terminal_velocity();
}
