// Filename: linearIntegrator.cxx
// Created by:  charles (02Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "linearIntegrator.h"
#include "config_physics.h"
#include "physicalNode.h"
#include "forceNode.h"

ConfigVariableDouble LinearIntegrator::_max_linear_dt
("default_max_linear_dt", 1.0f / 30.0f);


////////////////////////////////////////////////////////////////////
//    Function : BaseLinearIntegrator
//      Access : Protected
// Description : constructor
////////////////////////////////////////////////////////////////////
LinearIntegrator::
LinearIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearIntegrator
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearIntegrator::
~LinearIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : integrate
//      Access : public
// Description : parent integration routine, hands off to child
//               virtual.
////////////////////////////////////////////////////////////////////
void LinearIntegrator::
integrate(Physical *physical, LinearForceVector &forces,
          PN_stdfloat dt) {
/* <-- darren, 2000.10.06
  // cap dt so physics don't go flying off on lags
  if (dt > _max_linear_dt)
    dt = _max_linear_dt;
*/

  PhysicsObject::Vector::const_iterator current_object_iter;
  current_object_iter = physical->get_object_vector().begin();
  for (; current_object_iter != physical->get_object_vector().end();
       ++current_object_iter) {
    PhysicsObject *current_object = *current_object_iter;
    
    // bail out if this object doesn't exist or doesn't want to be
    // processed.
    if (current_object == (PhysicsObject *) NULL) {
      continue;
    }

    // set the object's last position to its current position before we move it
    current_object->set_last_position(current_object->get_position());
  }
  child_integrate(physical, forces, dt);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearIntegrator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearIntegrator";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearIntegrator::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearIntegrator:\n";
  out.width(indent+2); out<<""; out<<"_max_linear_dt "<<_max_linear_dt<<" (class static)\n";
  BaseIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
