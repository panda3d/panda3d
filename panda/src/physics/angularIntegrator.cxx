// Filename: angularIntegrator.cxx
// Created by:  charles (09Aug00)
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

#include "angularIntegrator.h"

ConfigVariableDouble AngularIntegrator::_max_angular_dt
("default_max_angular_dt", 1.0f / 30.0f);

////////////////////////////////////////////////////////////////////
//    Function : AngularIntegrator
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
AngularIntegrator::
AngularIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : ~AngularIntegrator
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
AngularIntegrator::
~AngularIntegrator() {
}

////////////////////////////////////////////////////////////////////
//    Function : Integrate
//      Access : public
// Description : high-level integration.  API.
////////////////////////////////////////////////////////////////////
void AngularIntegrator::
integrate(Physical *physical, AngularForceVector& forces,
          PN_stdfloat dt) {
  // intercept in case we want to censor/adjust values
  if (dt > _max_angular_dt) {
    dt = _max_angular_dt;
  }

  // this actually does the integration.
  child_integrate(physical, forces, dt);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularIntegrator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularIntegrator";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void AngularIntegrator::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularIntegrator:\n";
  out.width(indent+2); out<<""; out<<"_max_angular_dt "<<_max_angular_dt<<" (class const)\n";
  BaseIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
