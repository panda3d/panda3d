// Filename: particleSystem.cxx
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include <luse.h>
#include <lmat_ops.h>
#include <get_rel_pos.h>
#include <clockObject.h>
#include <nodeRelation.h>
#include <transformTransition.h>
#include <physicsManager.h>
#include <physicalNode.h>
#include <nearly_zero.h>

#include "config_particlesystem.h"
#include "particleSystem.h"
#include "particleSystemManager.h"
#include "pointParticleRenderer.h"
#include "pointParticleFactory.h"
#include "sphereSurfaceEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : ParticleSystem
//      Access : Public
// Description : Default Constructor.
////////////////////////////////////////////////////////////////////
ParticleSystem::
ParticleSystem(int pool_size) :
  Physical(pool_size, false), _particle_pool_size(pool_size)
{
  _birth_rate = 0.5f;
  _tics_since_birth = _birth_rate;
  _litter_size = 1;
  _litter_spread = 0;
  _living_particles = 0;
  _active_system_flag = true;
  _local_velocity_flag = true;
  _spawn_on_death_flag = false;
  _system_grows_older_flag = false;
  _system_lifespan = 0.0f;
  _i_was_spawned_flag = false;

  // just in case someone tries to do something that requires the
  // use of an emitter, renderer, or factory before they've actually
  // assigned one.  This is ok, because assigning them (set_renderer(),
  // set_emitter(), etc...) forces them to set themselves up for the
  // system, keeping the pool sizes consistent.

  _render_arc.clear();
  _render_parent = new NamedNode("ParticleSystem default render parent");

  set_emitter(new SphereSurfaceEmitter);
  set_renderer(new PointParticleRenderer);
  set_factory(new PointParticleFactory);
}

////////////////////////////////////////////////////////////////////
//    Function : ParticleSystem
//      Access : Public
// Description : Copy Constructor.
////////////////////////////////////////////////////////////////////
ParticleSystem::
ParticleSystem(const ParticleSystem& copy) :
  Physical(copy),
  _system_age(0.0f),
  _template_system_flag(false)
{

  _particle_pool_size = copy._particle_pool_size;
  _birth_rate = copy._birth_rate;
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

  _render_arc = copy._render_arc;

  _render_parent = copy._render_parent;

  _tics_since_birth = _birth_rate;
  _system_lifespan = copy._system_lifespan;
  _living_particles = 0;

  resize_pool();
}

////////////////////////////////////////////////////////////////////
//    Function : ~ParticleSystem
//      Access : Public
// Description : You get the ankles and I'll get the wrists.
////////////////////////////////////////////////////////////////////
ParticleSystem::
~ParticleSystem(void) {
  set_pool_size(0);

  if (_template_system_flag == false) {
    _renderer.clear();

    if (_render_arc.is_null() == false)
      remove_arc(_render_arc);
  }

  if (_i_was_spawned_flag == true)
    remove_arc(_physical_node_arc);
}

////////////////////////////////////////////////////////////////////
//    Function : birth_particle
//      Access : Private
// Description : A new particle is born.  This doesn't allocate,
//               resets an element from the particle pool.
////////////////////////////////////////////////////////////////////
bool ParticleSystem::
birth_particle(void) {
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
  LPoint3f new_pos, world_pos;
  LVector3f new_vel;
  LMatrix4f birth_to_render_xform;
  GeomNode *render_node;

  _emitter->generate(new_pos, new_vel);
  render_node = _renderer->get_render_node();

  // go from birth space to render space
  get_rel_mat(get_physical_node(), render_node, birth_to_render_xform);
  world_pos = new_pos * birth_to_render_xform;

  //  cout << "New particle at " << world_pos << endl;

  // possibly transform the initial velocity as well.
  if (_local_velocity_flag == false)
    new_vel = new_vel * birth_to_render_xform;

  bp->set_position_HandOfGod(world_pos/* + (NORMALIZED_RAND() * new_vel)*/);
  bp->set_velocity(new_vel);

  _living_particles++;

  // propogate information down to renderer
  _renderer->birth_particle(pool_index);

  return true;
}

////////////////////////////////////////////////////////////////////
//    Function : birth_litter
//      Access : Private
// Description : spawns a new batch of particles
////////////////////////////////////////////////////////////////////
void ParticleSystem::
birth_litter(void) {
  int litter_size, i;

  litter_size = _litter_size;

  if (_litter_spread != 0)
    litter_size += I_SPREAD(_litter_spread);

  for (i = 0; i < litter_size; i++) {
    if (birth_particle() == false)
      return;
  }
}

////////////////////////////////////////////////////////////////////
//    Function : spawn_child_system
//      Access : private
// Description : Creates a new particle system based on local
//               template info and adds it to the ps and physics
//               managers
////////////////////////////////////////////////////////////////////
void ParticleSystem::
spawn_child_system(BaseParticle *bp) {
  // first, make sure that the system exists in the graph via a
  // physicalnode reference.
  PhysicalNode *this_pn = get_physical_node();
  if (!this_pn) {
    physics_cat.error() << "ParticleSystem::spawn_child_system: "
			<< "Spawning system is not in the scene graph,"
			<< " aborting." << endl;
    return;
  }

  if (this_pn->get_num_parents(RenderRelation::get_class_type()) == 0) {
    physics_cat.error() << "ParticleSystem::spawn_child_system: "
			<< "PhysicalNode this system is contained in "
			<< "has no parent, aborting." << endl;
    return;
  }

  NodeRelation *parent_relation =
    this_pn->get_parent(RenderRelation::get_class_type(), 0);

  Node *parent = parent_relation->get_parent();

  // handle the spawn templates
  int new_ps_index = rand() % _spawn_templates.size();
  ParticleSystem *ps_template = _spawn_templates[new_ps_index];

  // create a new particle system
  PT(ParticleSystem) new_ps = new ParticleSystem(*ps_template);
  new_ps->_i_was_spawned_flag = true;

  // first, set up the render node info.
  new_ps->_render_parent = _spawn_render_node;
  new_ps->_render_arc = new RenderRelation(new_ps->_render_parent,
					   new_ps->_renderer->get_render_node());

  // now set up the new system's PhysicalNode.
  PT(PhysicalNode) new_pn = new PhysicalNode;
  new_pn->add_physical(new_ps);

  // the arc from the parent to the new child has to represent the
  // transform from the current system up to its parent, and then
  // subsequently down to the new child.
  PT(RenderRelation) rr = new RenderRelation(parent, new_pn);

  LMatrix4f old_system_to_parent_xform;
  get_rel_mat(get_physical_node(), parent, old_system_to_parent_xform);

  LMatrix4f child_space_xform = old_system_to_parent_xform *
    bp->get_lcs();

  rr->set_transition(new TransformTransition(child_space_xform));

  // tack the new system onto the managers
  _manager->attach_particlesystem(new_ps);
  get_physics_manager()->attach_physical(new_ps);
}

////////////////////////////////////////////////////////////////////
//    Function : kill_particle
//      Access : Private
// Description : Kills a particle, returns its slot to the empty
//               stack.
////////////////////////////////////////////////////////////////////
void ParticleSystem::
kill_particle(int pool_index) {
  // get a handle on our particle
  BaseParticle *bp = (BaseParticle *) _physics_objects[pool_index].p();

  // create a new system where this one died, maybe.
  if (_spawn_on_death_flag == true)
    spawn_child_system(bp);

  // tell everyone that it's dead
  bp->set_alive(false);
  bp->set_active(false);
  bp->die();

  _free_particle_fifo.push_back(pool_index);

  // tell renderer
  _renderer->kill_particle(pool_index);

  _living_particles--;
}

////////////////////////////////////////////////////////////////////
//    Function : resize_pool
//      Access : Private
// Description : Resizes the particle pool according to _particle_pool_size
////////////////////////////////////////////////////////////////////
#ifdef PSDEBUG
#define PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
#endif
void ParticleSystem::
resize_pool(void) {
  int i;
  int delta = _particle_pool_size - _physics_objects.size();

#ifdef PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
  cout << "resizing particle pool from " << _physics_objects.size()
       << " to " << _particle_pool_size << endl;
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
    if (delta >= _physics_objects.size()) {
#ifdef PSDEBUG
      cout << "Weird... do we have a negative pool size??" << endl;
#endif
      _physics_objects.erase(_physics_objects.begin(), _physics_objects.end());
      _free_particle_fifo.clear();
    } else {
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
          deque<int>::iterator i;
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
  }

  _renderer->resize_pool(_particle_pool_size);

#ifdef PARTICLE_SYSTEM_RESIZE_POOL_SENTRIES
  cout << "particle pool resized" << endl;
#endif
}

//////////////////////////////////////////////////////////////////////
//    Function : update
//      Access : Public
// Description : Updates the particle system.  Call once per frame.
//////////////////////////////////////////////////////////////////////
#ifdef PSDEBUG
//#define PARTICLE_SYSTEM_UPDATE_SENTRIES
#endif
void ParticleSystem::
update(float dt) {
  int ttl_updates_left = _living_particles;
  int current_index = 0, index_counter = 0;
  BaseParticle *bp;
  float age;

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

    if (age >= bp->get_lifespan())
      kill_particle(current_index);
    else
      bp->update();

    // break out early if we're lucky
    ttl_updates_left--;
  }


  // generate new particles if necessary.
  _tics_since_birth += dt;

  while (_tics_since_birth >= _birth_rate) {
    birth_litter();
    _tics_since_birth -= _birth_rate;
  }

#ifdef PARTICLE_SYSTEM_UPDATE_SENTRIES
  cout << "particle update complete" << endl;
#endif
}
