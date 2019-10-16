/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem.h
 * @author charles
 * @date 2000-06-14
 */

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "pandabase.h"
#include "pointerTo.h"
#include "physical.h"
#include "pandaNode.h"
#include "referenceCount.h"
#include "pdeque.h"
#include "pStatTimer.h"
#include "baseParticle.h"
#include "baseParticleRenderer.h"
#include "baseParticleEmitter.h"
#include "baseParticleFactory.h"

class ParticleSystemManager;

/**
 * Contains and manages a particle system.
 */
class EXPCL_PANDA_PARTICLESYSTEM ParticleSystem : public Physical {
PUBLISHED:
  // constructordestructor

  explicit ParticleSystem(int pool_size = 0);
  ParticleSystem(const ParticleSystem& copy);
  ~ParticleSystem();

  // accessqueries
  INLINE void set_pool_size(int size);
  INLINE void set_birth_rate(PN_stdfloat new_br);
  INLINE void set_soft_birth_rate(PN_stdfloat new_br);
  INLINE void set_litter_size(int new_ls);
  INLINE void set_litter_spread(int new_ls);
  INLINE void set_local_velocity_flag(bool lv);
  INLINE void set_system_grows_older_flag(bool sgo);
  INLINE void set_system_lifespan(PN_stdfloat sl);
  INLINE void set_system_age(PN_stdfloat age);
  INLINE void set_active_system_flag(bool a);
  INLINE void set_spawn_on_death_flag(bool sod);
  INLINE void set_spawn_render_node(PandaNode *node);
  INLINE void set_spawn_render_node_path(const NodePath &node);
  INLINE void set_template_system_flag(bool tsf);
  INLINE void set_render_parent(PandaNode *node);
  INLINE void set_render_parent(const NodePath &node);
  INLINE void set_renderer(BaseParticleRenderer *r);
  INLINE void set_emitter(BaseParticleEmitter *e);
  INLINE void set_factory(BaseParticleFactory *f);
  INLINE void set_floor_z(PN_stdfloat z);

  INLINE void clear_floor_z();

  INLINE int get_pool_size() const;
  INLINE PN_stdfloat get_birth_rate() const;
  INLINE PN_stdfloat get_soft_birth_rate() const;
  INLINE int get_litter_size() const;
  INLINE int get_litter_spread() const;
  INLINE bool get_local_velocity_flag() const;
  INLINE bool get_system_grows_older_flag() const;
  INLINE PN_stdfloat get_system_lifespan() const;
  INLINE PN_stdfloat get_system_age() const;
  INLINE bool get_active_system_flag() const;
  INLINE bool get_spawn_on_death_flag() const;
  INLINE PandaNode *get_spawn_render_node() const;
  INLINE NodePath get_spawn_render_node_path() const;
  INLINE bool get_i_was_spawned_flag() const;
  INLINE int get_living_particles() const;
  INLINE NodePath get_render_parent() const;
  INLINE BaseParticleRenderer *get_renderer() const;
  INLINE BaseParticleEmitter *get_emitter() const;
  INLINE BaseParticleFactory *get_factory() const;
  INLINE PN_stdfloat get_floor_z() const;

  // particle template vector

  INLINE void add_spawn_template(ParticleSystem *ps);
  INLINE void clear_spawn_templates();

  // methods

  INLINE void render();
  INLINE void induce_labor();
  INLINE void clear_to_initial();
  INLINE void soft_stop(PN_stdfloat br = 0.0);
  INLINE void soft_start(PN_stdfloat br = 0.0);
  void update(PN_stdfloat dt);

  virtual void output(std::ostream &out) const;
  virtual void write_free_particle_fifo(std::ostream &out, int indent=0) const;
  virtual void write_spawn_templates(std::ostream &out, int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  #ifdef PSSANITYCHECK
  int sanity_check();
  #endif

  bool birth_particle();
  void kill_particle(int pool_index);
  void birth_litter();
  void resize_pool(int size);

  pdeque< int > _free_particle_fifo;

  int _particle_pool_size;
  int _living_particles;
  PN_stdfloat _cur_birth_rate;
  PN_stdfloat _birth_rate;
  PN_stdfloat _soft_birth_rate;
  PN_stdfloat _tics_since_birth;
  int _litter_size;
  int _litter_spread;
  PN_stdfloat _system_age;
  PN_stdfloat _system_lifespan;
  PN_stdfloat _floor_z;

  PT(BaseParticleFactory) _factory;
  PT(BaseParticleEmitter) _emitter;
  PT(BaseParticleRenderer) _renderer;
  ParticleSystemManager *_manager;

  bool _template_system_flag;

  // _render_parent is the ALREADY ALLOC'D node under which this system will
  // render its particles.

  NodePath _render_parent;
  NodePath _render_node_path;

  bool _active_system_flag;
  bool _local_velocity_flag;
  bool _system_grows_older_flag;

  // information for systems that will spawn

  bool _spawn_on_death_flag;
  NodePath _spawn_render_node_path;
  pvector< PT(ParticleSystem) > _spawn_templates;

  void spawn_child_system(BaseParticle *bp);

  // information for spawned systems
  bool _i_was_spawned_flag;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Physical::init_type();
    register_type(_type_handle, "ParticleSystem",
                  Physical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ParticleSystemManager; // particleSystemManager.h

  static PStatCollector _update_collector;
};

#include "particleSystem.I"

#endif // PARTICLESYSTEM_H
