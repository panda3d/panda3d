/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystemManager.cxx
 * @author charles
 * @date 2000-06-28
 */

#include "particleSystemManager.h"
#include "particleSystem.h"

#include "pandabase.h"
#include "physicsManager.h"
#include "clockObject.h"
#include "pStatTimer.h"

#include <algorithm>

PStatCollector ParticleSystemManager::_do_particles_collector("App:Particles:Do Particles");

/**
 * default constructor
 */
ParticleSystemManager::
ParticleSystemManager(int every_nth_frame) :
  _nth_frame(every_nth_frame), _cur_frame(0) {
}

/**
 * Destructor
 */
ParticleSystemManager::
~ParticleSystemManager() {
}

/**
 * removes a ps from the maintenance list
 */
void ParticleSystemManager::
remove_particlesystem(ParticleSystem *ps) {
  plist< PT(ParticleSystem) >::iterator found;

  PT(ParticleSystem) ptps = ps;
  found = find(_ps_list.begin(), _ps_list.end(), ptps);

  if (found == _ps_list.end())
    return;

  _ps_list.erase(found);
}

/**
 * does an update and render for each ps in the list.  this is probably the
 * one you want to use.  Rendering is the expensive operation, and particles
 * REALLY should at least be updated every frame, so nth_frame stepping
 * applies only to rendering.
 */
void ParticleSystemManager::
do_particles(PN_stdfloat dt) {
  // cout << "ParticlesystemManager::doparticles entering."  << endl;
  PStatTimer t1(_do_particles_collector);

  plist< PT(ParticleSystem) >::iterator cur;

  bool render_due = false;

  _cur_frame++;

  if (_cur_frame >= _nth_frame) {
    _cur_frame = 0;
    render_due = true;
  }

  cur = _ps_list.begin();

  // cout << "PSM::do_particles on a vector of size " << _ps_list.size() <<
  // endl; int cs = 0;

  while (cur != _ps_list.end()) {
    ParticleSystem *cur_ps = *cur;

    // update this system
    if (cur_ps->get_active_system_flag() == true) {
      // cout << "  system " << cs++ << endl; cout << "  count is: " <<
      // cur_ps->get_render_parent()->get_ref_count() << endl;
      cur_ps->update(dt);

      // Handle age:
      if (cur_ps->get_system_grows_older_flag() == true) {
        PN_stdfloat age = cur_ps->get_system_age() + dt;
        cur_ps->set_system_age(age);

        // handle death
        if (age >= cur_ps->get_system_lifespan()) {
          plist< PT(ParticleSystem) >::iterator kill = cur;
          ++cur;

          _ps_list.erase(kill);
          render_due = false;
        }
      }

      // handle render
      if (render_due) {
        cur_ps->render();
      }
    }

    if (cur != _ps_list.end()) {
      ++cur;
    }
  }
  // cout << "PSM::do_particles finished."  << endl; cout <<
  // "ParticleSystemManager::doparticles exiting."  << endl;
}

/**
 * does an update and an optional render for a specific ps.  Since rendering
 * is the expensive operation, multiple updates could be applied before
 * calling the final render.
 */
void ParticleSystemManager::
do_particles(PN_stdfloat dt, ParticleSystem *ps, bool do_render) {
  if (ps->get_active_system_flag() == true) {
    ps->update(dt);
    // Handle age:
    if (ps->get_system_grows_older_flag() == true) {
      PN_stdfloat age = ps->get_system_age() + dt;
      ps->set_system_age(age);
    }

    // handle render
    if (do_render) {
      ps->render();
    }
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystemManager::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ParticleSystemManager";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystemManager::
write_ps_list(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_ps_list ("<<_ps_list.size()<<" systems)\n";
  for (plist< PT(ParticleSystem) >::const_iterator i=_ps_list.begin();
       i != _ps_list.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ParticleSystemManager::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ParticleSystemManager:\n";
  out.width(indent+2); out<<""; out<<"_nth_frame "<<_nth_frame<<"\n";
  out.width(indent+2); out<<""; out<<"_cur_frame "<<_cur_frame<<"\n";
  write_ps_list(out, indent+2);
  #endif //] NDEBUG
}
