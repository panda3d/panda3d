// Filename: angularIntegrator.cxx
// Created by:  charles (09Aug00)
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

#include "angularIntegrator.h"

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
          float dt) {
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
