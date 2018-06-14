/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularIntegrator.cxx
 * @author charles
 * @date 2000-08-09
 */

#include "angularIntegrator.h"

ConfigVariableDouble AngularIntegrator::_max_angular_dt
("default_max_angular_dt", 1.0f / 30.0f);

/**
 * constructor
 */
AngularIntegrator::
AngularIntegrator() {
}

/**
 * destructor
 */
AngularIntegrator::
~AngularIntegrator() {
}

/**
 * high-level integration.  API.
 */
void AngularIntegrator::
integrate(Physical *physical, AngularForceVector& forces,
          PN_stdfloat dt) {
  // intercept in case we want to censoradjust values
  if (dt > _max_angular_dt) {
    dt = _max_angular_dt;
  }

  // this actually does the integration.
  child_integrate(physical, forces, dt);
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularIntegrator::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularIntegrator";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularIntegrator::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularIntegrator:\n";
  out.width(indent+2); out<<""; out<<"_max_angular_dt "<<_max_angular_dt<<" (class const)\n";
  BaseIntegrator::write(out, indent+2);
  #endif //] NDEBUG
}
