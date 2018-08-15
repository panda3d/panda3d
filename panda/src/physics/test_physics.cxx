/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_physics.cxx
 * @author charles
 * @date 2000-06-13
 */

#include <iostream>
#include "physical.h"
#include "physicsManager.h"
#include "forces.h"

using std::cout;
using std::endl;

class Baseball : public Physical {
public:
  int ttl_balls;
  // int color;

  Baseball(int tb = 1) : ttl_balls(tb), Physical(tb, true) {}
};

int main(int argc, char **argv) {
  PhysicsManager physics_manager;
  Baseball b(8);

  // test the noise force
  Baseball nf_b;
  nf_b.get_phys_body()->set_position(0.0f, 0.0f, 0.0f);
  nf_b.get_phys_body()->set_velocity(1.0f / 16.0f, 0.0f, 0.0f);
  nf_b.get_phys_body()->set_active(true);
  nf_b.get_phys_body()->set_mass(1.0f);

  LinearNoiseForce nf(1.0, false);
  physics_manager.attach_physical(&nf_b);

  int steps=16;
  PN_stdfloat delta_time=1.0f/(PN_stdfloat)steps;
  while (steps--) {
    cout << "ball: " << nf_b.get_phys_body()->get_position() << endl;
    cout << "nf: " << nf.get_vector(nf_b.get_phys_body()) << endl;
    physics_manager.do_physics(delta_time);
  }

  physics_manager.remove_physical(&nf_b);

  // get on with life

  b.add_linear_force(new LinearJitterForce(0.1f));

  int i=0;
  for (PhysicsObject::Vector::const_iterator co=b.get_object_vector().begin();
       co != b.get_object_vector().end();
       ++i, ++co) {
    (*co)->set_position(i * 2.0f, PN_stdfloat(i), 0.0f);
    (*co)->set_velocity(5.0f, 0.0f, 30.0f);
    (*co)->set_active(true);
    (*co)->set_mass(1.0f);
  }

  physics_manager.attach_physical(&b);
  physics_manager.add_linear_force(new LinearVectorForce(0.0f, 0.0f, -9.8f, 1.0f, false));

  cout << "Object vector:" << endl;
  for (PhysicsObject::Vector::const_iterator co=b.get_object_vector().begin();
       co != b.get_object_vector().end();
       ++co) {
    cout << "vel: " << (*co)->get_velocity() << "  ";
    cout << "pos: " << (*co)->get_position() << endl;
  }

  physics_manager.do_physics(1.0f);
  cout << "Physics have been applied." << endl;

  for (PhysicsObject::Vector::const_iterator co=b.get_object_vector().begin();
       co != b.get_object_vector().end();
       ++co) {
    cout << "vel: " << (*co)->get_velocity() << "  ";
    cout << "pos: " << (*co)->get_position() << endl;
  }

  return 0;
}
