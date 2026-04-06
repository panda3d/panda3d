/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem.I
 * @author charles
 * @date 2000-06-14
 */

/**
 * Populates an attached GeomNode structure with the particle geometry for
 * rendering.  This is a wrapper for accessability.
 */
INLINE void ParticleSystem::
render() {
  _renderer->render(_physics_objects, _living_particles);
}

/**
 * Forces the birth of a particle litter this frame by resetting
 * _tics_since_birth
 */
INLINE void ParticleSystem::
induce_labor() {
  _tics_since_birth = _cur_birth_rate;
}

/**
 * Resets the system to its start state by resizing to 0, then resizing back
 * to current size.
 */
INLINE void ParticleSystem::
clear_to_initial() {
  BaseParticle *bp;
  int i;

  for(i = 0; i < (int)_physics_objects.size(); i++) {
    bp = (BaseParticle *)_physics_objects[i].p();
    if(bp->get_alive()) {
      kill_particle(i);
    }
  }
  _tics_since_birth = 0.0f;
}

/**
 * Causes system to use birth rate set by set_birth_rate().
 * If first_birth_delay is specified, it specifies the number of seconds to wait
 * in addition to the birth rate before the first particle is birthed.  It is
 * legal for this to be a negative value, which causes the first birth to happen
 * sooner rather than later.
 */
INLINE void ParticleSystem::
soft_start(PN_stdfloat br, PN_stdfloat first_birth_delay) {
  if (br > 0.0) {
    set_birth_rate(br);
  }
  _cur_birth_rate = _birth_rate;
  _tics_since_birth = -first_birth_delay;
}

/**
 * Causes system to use birth rate set by set_soft_birth_rate()
 */
INLINE void ParticleSystem::
soft_stop(PN_stdfloat br) {
  if (br > 0.0)
    set_soft_birth_rate(br);
  _cur_birth_rate = _soft_birth_rate;
  _tics_since_birth = 0.0f;
}

/**

 */
INLINE void ParticleSystem::
set_pool_size(int size) {
  resize_pool(size);
}

/**

 */
INLINE void ParticleSystem::
set_birth_rate(PN_stdfloat new_br) {
  _birth_rate = new_br;
  _cur_birth_rate = _birth_rate;
  if(IS_NEARLY_ZERO(_birth_rate)) _birth_rate = NEARLY_ZERO(PN_stdfloat);
}

/**

 */
INLINE void ParticleSystem::
set_soft_birth_rate(PN_stdfloat new_br) {
  _soft_birth_rate = new_br;
  if(IS_NEARLY_ZERO(_soft_birth_rate)) _soft_birth_rate = NEARLY_ZERO(PN_stdfloat);
}

/**

 */
INLINE void ParticleSystem::
set_litter_size(int new_ls) {
  _litter_size = new_ls;
}

/**

 */
INLINE void ParticleSystem::
set_litter_spread(int new_ls) {
  _litter_spread = new_ls;
}

/**

 */
INLINE void ParticleSystem::
set_renderer(BaseParticleRenderer *r) {
  _renderer = r;
  _renderer->resize_pool(_particle_pool_size);

  _render_node_path.remove_node();
  _render_node_path = _renderer->get_render_node_path();
  _render_node_path.reparent_to(_render_parent);
}

/**

 */
INLINE void ParticleSystem::
set_emitter(BaseParticleEmitter *e) {
  _emitter = e;
}

/**

 */
INLINE void ParticleSystem::
set_factory(BaseParticleFactory *f) {
  int pool_size = _particle_pool_size;
  set_pool_size(0);
  _factory = f;
  clear_physics_objects();
  set_pool_size(pool_size);
}

/**

 */
INLINE void ParticleSystem::
set_floor_z(PN_stdfloat z) {
  _floor_z = z;
}

/**

 */
INLINE void ParticleSystem::
set_active_system_flag(bool a) {
  _active_system_flag = a;
}

/**

 */
INLINE void ParticleSystem::
set_local_velocity_flag(bool lv) {
  _local_velocity_flag = lv;
}

/**

 */
INLINE void ParticleSystem::
set_spawn_on_death_flag(bool sod) {
  _spawn_on_death_flag = sod;
}

/**

 */
INLINE void ParticleSystem::
set_system_grows_older_flag(bool sgo) {
  _system_grows_older_flag = sgo;
}

/**

 */
INLINE void ParticleSystem::
set_system_lifespan(PN_stdfloat sl) {
  _system_lifespan = sl;
}

/**

 */
INLINE void ParticleSystem::
set_system_age(PN_stdfloat age) {
  _system_age = age;
}

/**

 */
INLINE void ParticleSystem::
set_spawn_render_node(PandaNode *node) {
  set_spawn_render_node_path(NodePath(node));
}

/**

 */
INLINE void ParticleSystem::
set_spawn_render_node_path(const NodePath &node) {
  _spawn_render_node_path = node;
}

/**

 */
INLINE void ParticleSystem::
set_render_parent(PandaNode *node) {
  set_render_parent(NodePath(node));
}

/**

 */
INLINE void ParticleSystem::
set_render_parent(const NodePath &node) {
  _render_node_path.remove_node();

  _render_parent = node;
  _render_node_path = _renderer->get_render_node_path();
  _render_node_path.reparent_to(_render_parent);
}

/**

 */
INLINE void ParticleSystem::
set_template_system_flag(bool tsf) {
  _template_system_flag = tsf;
}

/**

 */
INLINE void ParticleSystem::
add_spawn_template(ParticleSystem *ps) {
  _spawn_templates.push_back(ps);
}

/**

 */
INLINE void ParticleSystem::
clear_spawn_templates() {
  _spawn_templates.erase(_spawn_templates.begin(),
                         _spawn_templates.end());
}

/**

 */
INLINE void ParticleSystem::
clear_floor_z() {
  _floor_z = -HUGE_VAL;
}

/**

 */
INLINE int ParticleSystem::
get_pool_size() const {
  return _particle_pool_size;
}

/**

 */
INLINE PN_stdfloat ParticleSystem::
get_birth_rate() const {
  return _birth_rate;
}

/**

 */
INLINE PN_stdfloat ParticleSystem::
get_soft_birth_rate() const {
  return _soft_birth_rate;
}

/**

 */
INLINE int ParticleSystem::
get_litter_size() const {
  return _litter_size;
}

/**

 */
INLINE int ParticleSystem::
get_litter_spread() const {
  return _litter_spread;
}

/**

 */
INLINE BaseParticleRenderer *ParticleSystem::
get_renderer() const {
  return _renderer;
}

/**

 */
INLINE BaseParticleEmitter *ParticleSystem::
get_emitter() const {
  return _emitter;
}

/**

 */
INLINE BaseParticleFactory *ParticleSystem::
get_factory() const {
  return _factory;
}

/**

 */
INLINE PN_stdfloat ParticleSystem::
get_floor_z() const {
  return _floor_z;
}

/**

*/
INLINE PN_stdfloat ParticleSystem::
get_tics_since_birth() const {
  return _tics_since_birth;
}

/**

 */
INLINE int ParticleSystem::
get_living_particles() const {
  return _living_particles;
}

/**

 */
INLINE bool ParticleSystem::
get_active_system_flag() const {
  return _active_system_flag;
}

/**

 */
INLINE bool ParticleSystem::
get_local_velocity_flag() const {
  return _local_velocity_flag;
}

/**

 */
INLINE bool ParticleSystem::
get_spawn_on_death_flag() const {
  return _spawn_on_death_flag;
}

/**

 */
INLINE bool ParticleSystem::
get_system_grows_older_flag() const {
  return _system_grows_older_flag;
}

/**

 */
INLINE PN_stdfloat ParticleSystem::
get_system_lifespan() const {
  return _system_lifespan;
}

/**

 */
INLINE PN_stdfloat ParticleSystem::
get_system_age() const {
  return _system_age;
}

/**

 */
INLINE bool ParticleSystem::
get_i_was_spawned_flag() const {
  return _i_was_spawned_flag;
}

/**

 */
INLINE PandaNode *ParticleSystem::
get_spawn_render_node() const {
  return _spawn_render_node_path.node();
}

/**

 */
INLINE NodePath ParticleSystem::
get_spawn_render_node_path() const {
  return _spawn_render_node_path;
}

/**

 */
INLINE NodePath ParticleSystem::
get_render_parent() const {
  return _render_parent;
}
