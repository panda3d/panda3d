// Filename: test_physics.cxx
// Created by:  charles (13Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include "physical.h"
#include "physicsManager.h"
#include "forces.h"

class Baseball : public Physical
{
  public:

    int ttl_balls;
    int color;

    Baseball(int tb = 1) : ttl_balls(tb), Physical(tb, true) {}
};

int main(int, char **)
{
  PhysicsManager physics_manager;
  Baseball b(8);

  int i = 0;

  // test the noise force

  Baseball nf_b;
  nf_b._phys_body->set_position(0.0f, 0.0f, 0.0f);
  nf_b._phys_body->set_velocity(1.0f / 16.0f, 0.0f, 0.0f);
  nf_b._phys_body->set_processflag(true);
  nf_b._phys_body->set_mass(1.0f);

  NoiseForce nf(1.0, false);

  physics_manager.attach_physical(&nf_b);

  for (int monkey = 0; monkey < 16; monkey++)
  {
    cout << "ball: " << nf_b._phys_body->get_position() << endl;
    cout << "nf: " << nf.get_vector(nf_b._phys_body) << endl;

    physics_manager.do_physics(1.0f / 16.0f);
  }

  physics_manager.remove_physical(&nf_b);

  // get on with life

  b.add_force(new JitterForce(0.1f));

  for (i = 0; i < b.ttl_balls; i++)
  {
    b._physics_objects[i]->set_position(i * 2.0f, float(i), 0.0f);
    b._physics_objects[i]->set_velocity(5.0f, 0.0f, 30.0f);
    b._physics_objects[i]->set_processflag(true);
    b._physics_objects[i]->set_mass(1.0f);
  }

  physics_manager.attach_physical(&b);
  physics_manager.add_force(new VectorForce(0.0f, 0.0f, -9.8f, 1.0f, false));

  cout << "Object vector:" << endl;

  for (i = 0; i < b.ttl_balls; i++)
  {
    cout << "vel: " << b._physics_objects[i]->get_velocity() << "  ";
    cout << "pos: " << b._physics_objects[i]->get_position() << endl;
  }

  physics_manager.do_physics(1.0f);

  cout << "Physics have been applied." << endl;

  for (i = 0; i < b.ttl_balls; i++)
  {
    cout << "vel: " << b._physics_objects[i]->get_velocity() << "  ";
    cout << "pos: " << b._physics_objects[i]->get_position() << endl;
  }
}

