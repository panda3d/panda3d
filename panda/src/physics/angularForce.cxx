/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularForce.cxx
 * @author charles
 * @date 2000-08-08
 */

#include "angularForce.h"

TypeHandle AngularForce::_type_handle;

/**
 * constructor
 */
AngularForce::
AngularForce() :
  BaseForce() {
}

/**
 * copy constructor
 */
AngularForce::
AngularForce(const AngularForce &copy) :
  BaseForce(copy) {
}

/**
 * destructor
 */
AngularForce::
~AngularForce() {
}

/**
 * access query
 */
LRotation AngularForce::
get_quat(const PhysicsObject *po) {
  LRotation v = get_child_quat(po);
  return v;
}

/**
 * access query
 */
bool AngularForce::
is_linear() const {
  return false;
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularForce (id "<<this<<")";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularForce (id "<<this<<")\n";
  BaseForce::write(out, indent+2);
  #endif //] NDEBUG
}
