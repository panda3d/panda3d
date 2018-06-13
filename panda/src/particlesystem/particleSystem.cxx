/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem.cxx
 * @author charles
 * @date 2000-06-14
 */

#include <stdlib.h>

#include "luse.h"
#include "lmat_ops.h"
#include "clockObject.h"
#include "physicsManager.h"
#include "physicalNode.h"
#include "nearly_zero.h"
#include "transformState.h"
#include "nodePath.h"

#include "config_particlesystem.h"
#include "particleSystem.h"
#include "particleSystemManager.h"
#include "pointParticleRenderer.h"
#include "pointParticleFactory.h"
#include "sphereSurfaceEmitter.h"
#include "pStatTimer.h"

using std::cout;
using std::endl;
using std::ostream;

TypeHandle ParticleSystem::_type_handle;

PStatCollector ParticleSystem::_update_collector("App:Particles:Update");

/**
 * Default Constructor.
 */
ParticleSystem::
ParticleSystem(int pool_size) :
  Physical(pool_size, false)
{
  _birth_rate = 0.5f;
  _cur_birth_rate = _birth_rate;
  _soft_birth_rate = HUGE_VAL;
  _tics_since_birth = 0.0;
  _litter_size = 1;
  _litter_spread = 0;
  _living_particles = 0;
  _active_system_flag = true;
  _local_velocity_flag = true;
  _spawn_on_death_flag = false;
  _system_grows_older_flag = false;
  _system_lifespan = 0.0f;
  _i_was_spawned_flag = false;
  _particle_pool_size = 0;
  _floor_z = -HUGE_VAL;

  // just in case someone tries to do something that requires the use of an
  // emitter, renderer, or factory before they've actually assigned one.  This
  // is ok, because assigning them (set_renderer(), set_emitter(), etc...)
  // forces them to set themselves up for the system, keeping the pool sizes
  // consistent.

  _render_node_path = NodePath();
  _render_parent = NodePath("ParticleSystem default render parent");

  set_emitter(new SphereSurfaceEmitter);

  set_renderer(new PointParticleRenderer);

  // set_factory(new PointParticleFactory);
  _factory = new PointParticleFactory;
  clear_physics_objects();

  set_pool_size(pool_size);
}

/**
 * Copy Constructor.
 */
ParticleSystem::
ParticleSystem(const ParticleSystem& copy) :
  Physical(copy),
  _system_age(0.0f),
  _template_system_flag(false)
{
  _birth_rate = copy._birth_rate;
  _cur_birth_rate = copy._cur_birth_rate;
  _litter_size = copy._litter_size;
  _litter_spread = copy._litter_spread;
  _active_system_flag = copy._active_system_flag;
  _local_velocity_flag = copy._local_velocity_flag;
  _spawn_on_death_flag = copy._spawn_on_death_flag;
  _i_was_spawned_flag = copy._i_was_spawned_flag;
  _system_grows_older_flag = copy._system_grows_older_flag;
  _emitter = copy._emitter;
  _renderer = copy._renderer->make_copy();
  _factory = copy._factory;

  _render_parent = copy._render_parent;
  _render_node_path = _renderer->get_render_node_path();
  _render_node_path.reparent_to(_render_parent);

  _tics_since_birth = 0.0;
  _system_lifespan = copy._system_lifespan;
  _living_particles = 0;

  set_pool_size(copy._particle_pool_size);
}

/**
 * You get the ankles and I'll get the wrists.
 */
ParticleSystem::
~ParticleSystem() {
  set_pool_size(0);

  if (!_template_system_flag) {
    _renderer.clear();
    _render_node_path.remove_node();
  }
}

/**
 * A new particle is born.  This doesn't allocate, resets an element from the
 * particle pool.
 */
bool ParticleSystem::
birth_particle() {
  int pool_index;

  // make sure there's room for a new particle
  if (_living_particles >= _particle_pool_size) {
    #ifdef PSDEBUG
    if (_living_particles > _particle_pool_size) {
      cout << "_living_particles > _particle_pool_size" << endl;
    }
    #endif
    return false;
  }

  #ifdef PSDEBUG
  if (0 == _free_particle_fifo.size()) {
    cout << "Error: _free_particle_fifo is empty, but _living_particles < _particle_pool_size" << endl;
    return false;
  }
  #endif

  pool_index = _free_particle_fifo.back();
  _free_particle_fifo.pop_back();

  // get a handle on our particle.
  BaseParticle *bp = (BaseParticle *) _physics_objects[pool_index].p();

  // start filling out the variables.
  _factory->populate_particle(bp);

  bp->set_alive(true);
  bp->set_active(true);
  bp->init();

  // get the location of the new particle.
  LPoint3 new_pos, world_pos;
  LVector3 new_vel;

  _emitter->generate(new_pos, new_vel);

  // go from birth space to render space
  NodePath physical_np = get_physical_node_path();
  NodePath render_np = _renderer->get_render_node_path();

  CPT(TransformState) transform = physical_np.get_transform(render_np);
  const LMatrix4 &birth_to_render_xform = transform->get_mat();
  world_pos = new_pos * birth_to_render_xform;

  // cout << "New particle at " << world_pos << endl;

  // possibly transform the initial velocity as well.
  if (_local_velocity_flag == false)
    new_vel = new_vel * birth_to_render_xform;

  bp->reset_position(world_pos/* + (NORMALIZED_RAND() * new_vel)*/);
  bp->set_velocity(new_vel);

  ++_living_particles;

  // propogate information down to renderer
  _renderer->birth_particle(pool_index);

  return true;
}

/**
 * spawns a new batch of particles
 */
void ParticleSystem::
birth_litter() {
  int litter_size, i;

  litter_size = _litter_size;

  if (_litter_spread != 0)
    litter_size += I_SPREAD(_litter_spread);

  for (i = 0; i < litter_size; ++i) {
    if (birth_particle() == false)
      return;
  }
}

/**
 * Creates a new particle system based on local template info and adds it to
 * the ps and physics managers
 */
void ParticleSystem::
spawn_child_system(BaseParticle *bp) {
  // first, make sure that the system exists in the graph via a physicalnode
  // reference.
  PhysicalNode *this_pn = get_physical_node();
  if (!this_pn) {
    physics_cat.error() << "ParticleSystem::spawn_child_system: "
                        << "Spawning system is not in the scene graph,"
                        << " aborting." << endl;
    return;
  }

  if (this_pn->get_num_parents() == 0) {
    physics_cat.error() << "ParticleSystem::spawn_child_system: "
                        << "PhysicalNode this system is contained in "
                        << "has no parent, aborting." << endl;
    return;
  }

  if (_spawn_templates.size() == 0) {
    physics_cat.error() << "ParticleSystem::spawn_child_system: "
                        << "no spawn templates present." << endl;
    return;
  }

  NodePath physical_np = get_physical_node_path();
  NodePath parent_np = physical_np.get_parent();

  PandaNode *parent = parent_np.node();

  // handle the spawn templates
  int new_ps_index = rand() % _spawn_templates.size();
  ParticleSystem *ps_template = _spawn_templates[new_ps_index];

  // create a new particle system
  PT(ParticleSystem) new_ps = new ParticleSystem(*ps_template);
  new_ps->_i_was_spawned_flag = true;

  // first, set up the render node info.
  new_ps->_render_parent = _spawn_render_node_path;
  new_ps->_render_node_path = new_ps->_renderer->get_render_node_path();
  new_ps->_render_node_path.reparent_to(new_ps->_render_parent);

  // now set up the new system's PhysicalNode.
  PT(PhysicalNode) new_pn = new PhysicalNode("new_pn");
  new_pn->add_physical(new_ps);

  // the transform on the new child has to represent the transform from the
  // current system up to its parent, and then subsequently down to the new
  // child.
  parent->add_child(new_pn);

  CPT(TransformState) transform = physical_np.get_transform(parent_np);
  const LMatrix4 &old_system_to_parent_xform = transform->get_mat();

  LMatrix4 child_space_xform = old_system_to_parent_xform *
    bp->get_lcs();

  new_pn->set_transform(TransformState::make_mat(child_space_xform));

  // tack the new system onto the managers
  _manager->attach_particlesystem(new_ps);
  get_physics_manager()->attach_physical(new_ps);
}

/**
 * Kills a particle, returns its slot to the empty stack.
 */
void ParticleSystem::
kill_particle(int pool_index) {
  // get a handle on our particle
  BaseParticle *bp = (BaseParticle *) _physics_objects[pool_index].p();

  // create a new system where this one died, maybe.
  if (_spawn_on_death_flag == true) {
    spawn_child_system(bp);
  }

  // tell everyone that it's dead
  bp->set_alive(false);
  bp->set_active(false);
  bp->die();

  _free_particle_fifo.push_back(pool_index);

  // tell renderer
  _renderer->kill_particle(pool_index);

  _living_particles--;
}

/**
 * Resizes the particle pool
 */
#ifdef PSDEBUG
#define PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
#endif
void ParticleSystem::
resize_pool(int size) {
  int i;
  int delta = size - _particle_pool_size;
  int po_delta = _particle_pool_size - _physics_objects.size();

  #ifdef PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
  cout << "resizing particle pool from " << _particle_pool_size
       << " to " << size << endl;
  #endif

  if (_factory.is_null()) {
    particlesystem_cat.error() << "ParticleSystem::resize_pool"
                               << " called with null _factory." << endl;
    return;
  }

  if (_renderer.is_null()) {
    particlesystem_cat.error() << "ParticleSystem::resize_pool"
                               << " called with null _renderer." << endl;
    return;
  }

  _particle_pool_size = size;

  // make sure the physics_objects array is OK
  if (po_delta) {
    if (po_delta > 0) {
      for (i = 0; i < po_delta; i++)
      {
        // int free_index = _physics_objects.size();

        BaseParticle *new_particle = _factory->alloc_particle();
        if (new_particle) {
          _factory->populate_particle(new_particle);

          _physics_objects.push_back(new_particle);
        } else {
          #ifdef PSDEBUG
          cout << "Error allocating new particle" << endl;
          _particle_pool_size--;
          #endif
        }
      }
    } else {
      #ifdef PSDEBUG
      cout << "physics_object array is too large??" << endl;
      _particle_pool_size--;
      #endif
      po_delta = -po_delta;
      for (i = 0; i < po_delta; i++) {
        int delete_index = _physics_objects.size()-1;
        BaseParticle *bp = (BaseParticle *) _physics_objects[delete_index].p();
        if (bp->get_alive()) {
          kill_particle(delete_index);
          _free_particle_fifo.pop_back();
        } else {
          pdeque<int>::iterator i;
          i = find(_free_particle_fifo.begin(), _free_particle_fifo.end(), delete_index);
          if (i != _free_particle_fifo.end()) {
            _free_particle_fifo.erase(i);
          }
        }
        _physics_objects.pop_back();
      }
    }
  }

  // disregard no change
  if (delta == 0)
    return;

  // update the pool
  if (delta > 0) {
    // add elements
    for (i = 0; i < delta; i++)
    {
      int free_index = _physics_objects.size();

      BaseParticle *new_particle = _factory->alloc_particle();
      if (new_particle) {
        _factory->populate_particle(new_particle);

        _physics_objects.push_back(new_particle);
        _free_particle_fifo.push_back(free_index);
      } else {
        #ifdef PSDEBUG
        cout << "Error allocating new particle" << endl;
        _particle_pool_size--;
        #endif
      }
    }
  } else {
    // subtract elements
    delta = -delta;
    for (i = 0; i < delta; i++) {
      int delete_index = _physics_objects.size()-1;
      BaseParticle *bp = (BaseParticle *) _physics_objects[delete_index].p();

      if (bp->get_alive()) {
        #ifdef PSDEBUG
        cout << "WAS ALIVE" << endl;
        #endif
        kill_particle(delete_index);
        _free_particle_fifo.pop_back();
      } else {
        #ifdef PSDEBUG
        cout << "WAS NOT ALIVE" << endl;
        #endif
        pdeque<int>::iterator i;
        i = find(_free_particle_fifo.begin(), _free_particle_fifo.end(), delete_index);
        if (i != _free_particle_fifo.end()) {
          _free_particle_fifo.erase(i);
        }
        #ifdef PSDEBUG
        else {
          cout << "particle not found in free FIFO!!!!!!!!" << endl;
        }
        #endif
      }

      _physics_objects.pop_back();
    }
  }

  _renderer->resize_pool(_particle_pool_size);

  #ifdef PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
  cout << "particle pool resized" << endl;
  #endif
}

/**
 * Updates the particle system.  Call once per frame.
 */
#ifdef PSDEBUG
// #define PARTICLE_SYSTEM_UPDATE_SENTRIES
#endif
void ParticleSystem::
update(PN_stdfloat dt) {
  PStatTimer t1(_update_collector);

  int ttl_updates_left = _living_particles;
  int current_index = 0, index_counter = 0;
  BaseParticle *bp;
  PN_stdfloat age;

  #ifdef PSSANITYCHECK
  // check up on things
  if (sanity_check()) return;
  #endif

  #ifdef PARTICLE_SYSTEM_UPDATE_SENTRIES
  cout << "UPDATE: pool size: " << _particle_pool_size
       << ", live particles: " << _living_particles << endl;
  #endif

  // run through the particle array
  while (ttl_updates_left) {
    current_index = index_counter;
    index_counter++;

    #ifdef PSDEBUG
    if (current_index >= _particle_pool_size) {
      cout << "ERROR: _living_particles is out of sync (too large)" << endl;
      cout << "pool size: " << _particle_pool_size
           << ", live particles: " << _living_particles
           << ", updates left: " << ttl_updates_left << endl;
      break;
    }
    #endif

    // get the current particle.
    bp = (BaseParticle *) _physics_objects[current_index].p();

    #ifdef PSDEBUG
    if (!bp) {
      cout << "NULL ptr at index " << current_index << endl;
      continue;
    }
    #endif

    if (bp->get_alive() == false)
      continue;

    age = bp->get_age() + dt;
    bp->set_age(age);

    // cerr<<"bp->get_position().get_z() returning
    // "<<bp->get_position().get_z()<<endl;
    if (age >= bp->get_lifespan()) {
      kill_particle(current_index);
    } else if (get_floor_z() != -HUGE_VAL
            && bp->get_position().get_z() <= get_floor_z()) {
      // ...the particle is going under the floor.  Maybe tell the particle to
      // bounce: bp->bounce()?
      kill_particle(current_index);
    } else {
      bp->update();
    }

    // break out early if we're lucky
    ttl_updates_left--;
  }


  // generate new particles if necessary.
  _tics_since_birth += dt;

  while (_tics_since_birth >= _cur_birth_rate) {
    birth_litter();
    _tics_since_birth -= _cur_birth_rate;
  }

  #ifdef PARTICLE_SYSTEM_UPDATE_SENTRIES
  cout << "particle update complete" << endl;
  #endif

}

#ifdef PSSANITYCHECK
/**
 * Checks consistency of live particle count, free particle list, etc.
 * returns 0 if everything is normal
 */
#ifndef NDEBUG
#define PSSCVERBOSE
#endif

class SC_valuenamepair : public ReferenceCount {
public:
  int value;
  char *name;
  SC_valuenamepair(int v, char *s) : value(v), name(s) {}
};

// returns 0 if OK, # of errors if not OK
static int check_free_live_total_particles(pvector< PT(SC_valuenamepair) > live_counts,
  pvector< PT(SC_valuenamepair) > dead_counts, pvector< PT(SC_valuenamepair) > total_counts,
  int print_all = 0) {

  int val = 0;
  int l, d, t;

  for(l = 0; l < live_counts.size(); l++) {
    for(d = 0; d < dead_counts.size(); d++) {
      for(t = 0; t < total_counts.size(); t++) {
        int live = live_counts[l]->value;
        int dead = dead_counts[d]->value;
        int total = total_counts[t]->value;
        if ((live + dead) != total) {
          #ifdef PSSCVERBOSE
          cout << "free/live/total count: "
               << live_counts[l]->name << " (" << live << ") + "
               << dead_counts[d]->name << " (" << dead << ") = "
               << live + dead << ", != "
               << total_counts[t]->name << " (" << total << ")"
               << endl;
          #endif
          val++;
        }
      }
    }
  }

  return val;
}

int ParticleSystem::
sanity_check() {
  int result = 0;
  int i;
  BaseParticle *bp;
  int pool_size;

  // check pool size
  if (_particle_pool_size != _physics_objects.size()) {
    #ifdef PSSCVERBOSE
    cout << "_particle_pool_size (" << _particle_pool_size
         << ") != particle array size (" << _physics_objects.size() << ")" << endl;
    #endif
    result++;
  }
  pool_size = std::min(_particle_pool_size, _physics_objects.size());

  // find out how many particles are REALLY alive and dead
  int real_live_particle_count = 0;
  int real_dead_particle_count = 0;

  for (i = 0; i < _physics_objects.size(); i++) {
    bp = (BaseParticle *) _physics_objects[i].p();
    if (true == bp->get_alive()) {
      real_live_particle_count++;
    } else {
      real_dead_particle_count++;
    }
  }

  if (real_live_particle_count != _living_particles) {
    #ifdef PSSCVERBOSE
    cout << "manually counted live particle count (" << real_live_particle_count
         << ") != _living_particles (" << _living_particles << ")" << endl;
    #endif
    result++;
  }

  if (real_dead_particle_count != _free_particle_fifo.size()) {
    #ifdef PSSCVERBOSE
    cout << "manually counted dead particle count (" << real_dead_particle_count
         << ") != free particle fifo size (" << _free_particle_fifo.size() << ")" << endl;
    #endif
    result++;
  }

  // check the free particle pool
  for (i = 0; i < _free_particle_fifo.size(); i++) {
    int index = _free_particle_fifo[i];

    // check that we're in bounds
    if (index >= pool_size) {
      #ifdef PSSCVERBOSE
      cout << "index from free particle fifo (" << index
           << ") is too large; pool size is " << pool_size << endl;
      #endif
      result++;
      continue;
    }

    // check that the particle is indeed dead
    bp = (BaseParticle *) _physics_objects[index].p();
    if (true == bp->get_alive()) {
      #ifdef PSSCVERBOSE
      cout << "particle " << index << " in free fifo is not dead" << endl;
      #endif
      result++;
    }
  }

  // check the numbers of free particles, live particles, and total particles
  pvector< PT(SC_valuenamepair) > live_counts;
  pvector< PT(SC_valuenamepair) > dead_counts;
  pvector< PT(SC_valuenamepair) > total_counts;

  live_counts.push_back(new SC_valuenamepair(real_live_particle_count, "real_live_particle_count"));

  dead_counts.push_back(new SC_valuenamepair(real_dead_particle_count, "real_dead_particle_count"));
  dead_counts.push_back(new SC_valuenamepair(_free_particle_fifo.size(), "free particle fifo size"));

  total_counts.push_back(new SC_valuenamepair(_particle_pool_size, "_particle_pool_size"));
  total_counts.push_back(new SC_valuenamepair(_physics_objects.size(), "actual particle pool size"));

  result += check_free_live_total_particles(live_counts, dead_counts, total_counts);

  return result;
}
#endif

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystem::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ParticleSystem";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystem::
write_free_particle_fifo(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_free_particle_fifo ("<<_free_particle_fifo.size()<<" forces)\n";
  for (pdeque< int >::const_iterator i=_free_particle_fifo.begin();
       i != _free_particle_fifo.end();
       ++i) {
    out.width(indent+2); out<<""; out<<(*i)<<"\n";
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystem::
write_spawn_templates(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_spawn_templates ("<<_spawn_templates.size()<<" templates)\n";
  for (pvector< PT(ParticleSystem) >::const_iterator i=_spawn_templates.begin();
       i != _spawn_templates.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystem::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ParticleSystem:\n";
  out.width(indent+2); out<<""; out<<"_particle_pool_size "<<_particle_pool_size<<"\n";
  out.width(indent+2); out<<""; out<<"_living_particles "<<_living_particles<<"\n";
  out.width(indent+2); out<<""; out<<"_tics_since_birth "<<_tics_since_birth<<"\n";
  out.width(indent+2); out<<""; out<<"_litter_size "<<_litter_size<<"\n";
  out.width(indent+2); out<<""; out<<"_litter_spread "<<_litter_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_system_age "<<_system_age<<"\n";
  out.width(indent+2); out<<""; out<<"_system_lifespan "<<_system_lifespan<<"\n";
  out.width(indent+2); out<<""; out<<"_factory "<<_factory<<"\n";
  out.width(indent+2); out<<""; out<<"_emitter "<<_emitter<<"\n";
  out.width(indent+2); out<<""; out<<"_renderer "<<_renderer<<"\n";
  out.width(indent+2); out<<""; out<<"_manager "<<_manager<<"\n";
  out.width(indent+2); out<<""; out<<"_template_system_flag "<<_template_system_flag<<"\n";
  out.width(indent+2); out<<""; out<<"_render_parent "<<_render_parent<<"\n";
  out.width(indent+2); out<<""; out<<"_render_node "<<_render_node_path<<"\n";
  out.width(indent+2); out<<""; out<<"_active_system_flag "<<_active_system_flag<<"\n";
  out.width(indent+2); out<<""; out<<"_local_velocity_flag "<<_local_velocity_flag<<"\n";
  out.width(indent+2); out<<""; out<<"_system_grows_older_flag "<<_system_grows_older_flag<<"\n";
  out.width(indent+2); out<<""; out<<"_spawn_on_death_flag "<<_spawn_on_death_flag<<"\n";
  out.width(indent+2); out<<""; out<<"_spawn_render_node "<<_spawn_render_node_path<<"\n";
  out.width(indent+2); out<<""; out<<"_i_was_spawned_flag "<<_i_was_spawned_flag<<"\n";
  write_free_particle_fifo(out, indent+2);
  write_spawn_templates(out, indent+2);
  Physical::write(out, indent+2);
  #endif //] NDEBUG
}
