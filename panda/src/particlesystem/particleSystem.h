// Filename: particleSystem.h
// Created by:  charles (14Jun00)
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

#ifndef NDEBUG
//#define PSDEBUG
#endif

//#define PSSANITYCHECK

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "pandabase.h"
#include "pointerTo.h"
#include "physical.h"
#include "pandaNode.h"
#include "referenceCount.h"

#include "pdeque.h"

#include "baseParticle.h"
#include "baseParticleRenderer.h"
#include "baseParticleEmitter.h"
#include "baseParticleFactory.h"

class ParticleSystemManager;

////////////////////////////////////////////////////////////////////
//       Class : ParticleSystem
// Description : Contains and manages a particle system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ParticleSystem : public Physical {
PUBLISHED:
  // constructor/destructor

  ParticleSystem(int pool_size = 0);
  ParticleSystem(const ParticleSystem& copy);
  ~ParticleSystem();

  // access/queries
  INLINE void set_pool_size(int size);
  INLINE void set_birth_rate(float new_br);
  INLINE void set_litter_size(int new_ls);
  INLINE void set_litter_spread(int new_ls);
  INLINE void set_local_velocity_flag(bool lv);
  INLINE void set_system_grows_older_flag(bool sgo);
  INLINE void set_system_lifespan(float sl);
  INLINE void set_system_age(float age);
  INLINE void set_active_system_flag(bool a);
  INLINE void set_spawn_on_death_flag(bool sod);
  INLINE void set_spawn_render_node(PandaNode *node);
  INLINE void set_template_system_flag(bool tsf);
  INLINE void set_render_parent(PandaNode *node);
  INLINE void set_renderer(BaseParticleRenderer *r);
  INLINE void set_emitter(BaseParticleEmitter *e);
  INLINE void set_factory(BaseParticleFactory *f);
  INLINE void set_floor_z(float z);

  INLINE int get_pool_size() const;
  INLINE float get_birth_rate() const;
  INLINE int get_litter_size() const;
  INLINE int get_litter_spread() const;
  INLINE bool get_local_velocity_flag() const;
  INLINE bool get_system_grows_older_flag() const;
  INLINE float get_system_lifespan() const;
  INLINE float get_system_age() const;
  INLINE bool get_active_system_flag() const;
  INLINE bool get_spawn_on_death_flag() const;
  INLINE PandaNode *get_spawn_render_node() const;
  INLINE bool get_i_was_spawned_flag() const;
  INLINE int get_living_particles() const;
  INLINE PandaNode *get_render_parent() const;
  INLINE BaseParticleRenderer *get_renderer() const;
  INLINE BaseParticleEmitter *get_emitter() const;
  INLINE BaseParticleFactory *get_factory() const;
  INLINE float get_floor_z() const;

  // particle template vector

  INLINE void add_spawn_template(ParticleSystem *ps);
  INLINE void clear_spawn_templates();

  // methods

  INLINE void render();
  INLINE void induce_labor();
  void update(float dt);

  virtual void output(ostream &out) const;
  virtual void write_free_particle_fifo(ostream &out, int indent=0) const;
  virtual void write_spawn_templates(ostream &out, int indent=0) const;
  virtual void write(ostream &out, int indent=0) const;

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
  float _birth_rate;
  float _tics_since_birth;
  int _litter_size;
  int _litter_spread;
  float _system_age;
  float _system_lifespan;
  float _floor_z;

  PT(BaseParticleFactory) _factory;
  PT(BaseParticleEmitter) _emitter;
  PT(BaseParticleRenderer) _renderer;
  ParticleSystemManager *_manager;

  bool _template_system_flag;

  // _render_parent is the ALREADY ALLOC'D node under which this
  // system will render its particles.

  PT(PandaNode) _render_parent;
  PT(PandaNode) _render_node;

  bool _active_system_flag;
  bool _local_velocity_flag;
  bool _system_grows_older_flag;

  // information for systems that will spawn

  bool _spawn_on_death_flag;
  PT(PandaNode) _spawn_render_node;
  pvector< PT(ParticleSystem) > _spawn_templates;

  void spawn_child_system(BaseParticle *bp);

  // information for spawned systems
  bool _i_was_spawned_flag;

public:
  friend class ParticleSystemManager; // particleSystemManager.h
};

#include "particleSystem.I"

#endif // PARTICLESYSTEM_H

