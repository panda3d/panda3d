// Filename: BaseParticle.I
// Created by:  charles (16Jun00)
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

INLINE float BaseParticle::get_age(void) const {
  return _age;
}

INLINE float BaseParticle::get_lifespan(void) const {
  return _lifespan;
}

INLINE bool BaseParticle::get_alive(void) const {
  return _alive;
}

INLINE float BaseParticle::get_parameterized_age(void) const {
  return _age / _lifespan;
}

INLINE float BaseParticle::get_parameterized_vel(void) const {
  return (get_velocity().length()) / get_terminal_velocity();
}
