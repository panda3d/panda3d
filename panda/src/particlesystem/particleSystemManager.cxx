// Filename: particleSystemManager.cxx
// Created by:  charles (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "particleSystemManager.h"
#include "particleSystem.h"

#include <pandabase.h>
#include <physicsManager.h>
#include <clockObject.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////
//    Function : ParticleSystemManager
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////
ParticleSystemManager::
ParticleSystemManager(int every_nth_frame) :
  _nth_frame(every_nth_frame), _cur_frame(0) {
}

////////////////////////////////////////////////////////////////////
//    Function : remove_particlesystem
//      Access : public
// Description : removes a ps from the maintenance list
////////////////////////////////////////////////////////////////////
void ParticleSystemManager::
remove_particlesystem(ParticleSystem *ps) {
  list< PT(ParticleSystem) >::iterator found;

  PT(ParticleSystem) ptps = ps;
  found = find(_ps_list.begin(), _ps_list.end(), ptps);

  if (found == _ps_list.end())
    return;

  _ps_list.erase(found);
}

////////////////////////////////////////////////////////////////////
//    Function : do_particles
//      Access : public
// Description : does an update and render for each ps in the list.
//               this is probably the one you want to use.  Rendering
//               is the expensive operation, and particles REALLY
//               should at least be updated every frame, so nth_frame
//               stepping applies only to rendering.
////////////////////////////////////////////////////////////////////
void ParticleSystemManager::
do_particles(float dt) {
  //  cout << "ParticlesystemManager::doparticles entering." << endl;

  list< PT(ParticleSystem) >::iterator cur;

  bool render_due = false;

  _cur_frame++;

  if (_cur_frame >= _nth_frame) {
    _cur_frame = 0;
    render_due = true;
  }

  cur = _ps_list.begin();

  //  cout << "PSM::do_particles on a vector of size " << _ps_list.size() << endl;
  int cs = 0;

  while (cur != _ps_list.end()) {
    ParticleSystem *cur_ps = *cur;

    // update this system
    if (cur_ps->get_active_system_flag() == true) {
      //      cout << "  system " << cs++ << endl;
      //      cout << "  count is: " << cur_ps->get_render_parent()->get_count() << endl;
      cur_ps->update(dt);

      // handle age
      if (cur_ps->get_system_grows_older_flag() == true) {
	float age = cur_ps->get_system_age() + dt;
	cur_ps->set_system_age(age);

	// handle death
	if (age >= cur_ps->get_system_lifespan()) {
	  list< PT(ParticleSystem) >::iterator kill = cur;
	  cur++;

	  _ps_list.erase(kill);
	  render_due = false;
	}
      }

      // handle render
      if (render_due) {
	cur_ps->render();
      }
    }

    cur++;
  }
  //  cout << "PSM::do_particles finished." << endl;
  //  cout << "ParticleSystemManager::doparticles exiting." << endl;
}
