// Filename: particleSystem.h
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <pandabase.h>
#include <pointerTo.h>
#include <physical.h>
#include <node.h>
#include <referenceCount.h>
#include <renderRelation.h>

#include <deque>

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
private:
  bool birth_particle(void);
  void kill_particle(int pool_index);
  void birth_litter(void);
  void resize_pool(void);

  deque< int > _free_particle_fifo;

  int _particle_pool_size;
  int _living_particles;
  float _birth_rate;
  float _tics_since_birth;
  int _litter_size;
  int _litter_delta;
  float _system_age;
  float _system_lifespan;

  PT(BaseParticleFactory) _factory;
  PT(BaseParticleEmitter) _emitter;
  PT(BaseParticleRenderer) _renderer;
  ParticleSystemManager *_manager;

  bool _template_system_flag;

  // _render_parent is the ALREADY ALLOC'D node under which this
  // system will render its particles.

  PT(Node) _render_parent;
  PT(RenderRelation) _render_arc;

  bool _active_system_flag;
  bool _local_velocity_flag;
  bool _system_grows_older_flag;

  // information for systems that will spawn

  bool _spawn_on_death_flag;
  PT(Node) _spawn_render_node;
  vector< PT(ParticleSystem) > _spawn_templates;

  void spawn_child_system(BaseParticle *bp);

  // information for spawned systems

  bool _i_was_spawned_flag;
  PT(RenderRelation) _physical_node_arc;

public:

  // constructor/destructor

  ParticleSystem(int pool_size = 0);
  ParticleSystem(const ParticleSystem& copy);
  ~ParticleSystem(void);

  // access/queries

  INLINE void set_pool_size(int size);
  INLINE int get_pool_size(void) const;

  INLINE void set_birth_rate(float new_br);
  INLINE float get_birth_rate(void) const;

  INLINE void set_litter_size(int new_ls);
  INLINE int get_litter_size(void) const;

  INLINE void set_litter_delta(int new_ld);
  INLINE int get_litter_delta(void) const;

  INLINE void set_renderer(BaseParticleRenderer *r);
  INLINE BaseParticleRenderer *get_renderer(void) const;

  INLINE void set_emitter(BaseParticleEmitter *e);
  INLINE BaseParticleEmitter *get_emitter(void) const;

  INLINE void set_factory(BaseParticleFactory *f);
  INLINE BaseParticleFactory *get_factory(void) const;

  INLINE void set_active_system_flag(bool a);
  INLINE bool get_active_system_flag(void) const;

  INLINE void set_local_velocity_flag(bool lv);
  INLINE bool get_local_velocity_flag(void) const;

  INLINE void set_spawn_on_death_flag(bool sod);
  INLINE bool get_spawn_on_death_flag(void) const;

  INLINE void set_system_grows_older_flag(bool sgo);
  INLINE bool get_system_grows_older_flag(void) const;

  INLINE void set_system_lifespan(float sl);
  INLINE float get_system_lifespan(void) const;

  INLINE void set_system_age(float age);
  INLINE float get_system_age(void) const;

  INLINE void set_spawn_render_node(Node *node);
  INLINE Node *get_spawn_render_node(void) const;

  INLINE bool get_i_was_spawned_flag(void) const;
  INLINE int get_living_particles(void) const;

  INLINE void set_template_system_flag(bool tsf);

  INLINE Node *get_render_parent(void) const;
  INLINE void set_render_parent(Node *node);

  // particle template vector

  INLINE void add_spawn_template(ParticleSystem *ps);
  INLINE void clear_spawn_templates(void);

  // methods

  INLINE void render(void);
  void update(float dt);

  friend class ParticleSystemManager;
};

#include "particleSystem.I"

#endif // PARTICLESYSTEM_H

