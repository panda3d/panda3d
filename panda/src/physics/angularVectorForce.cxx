/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularVectorForce.cxx
 * @author charles
 * @date 2000-08-09
 */

#include "angularVectorForce.h"

TypeHandle AngularVectorForce::_type_handle;

/**
 * constructor
 */
AngularVectorForce::
AngularVectorForce(const LRotation &vec) :
  AngularForce(), _fvec(vec) {
}

/**
 * constructor
 */
AngularVectorForce::
AngularVectorForce(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) :
  AngularForce() {
  _fvec.set_hpr(LVecBase3(h, p, r));
}

/**
 * copy constructor
 */
AngularVectorForce::
AngularVectorForce(const AngularVectorForce &copy) :
  AngularForce(copy) {
  _fvec = copy._fvec;
}

/**
 * destructor
 */
AngularVectorForce::
~AngularVectorForce() {
}

/**
 * dynamic copier
 */
AngularForce *AngularVectorForce::
make_copy() const {
  return new AngularVectorForce(*this);
}

/**
 * query
 */
LRotation AngularVectorForce::
get_child_quat(const PhysicsObject *) {
  return _fvec;
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularVectorForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"AngularVectorForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void AngularVectorForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"AngularVectorForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  AngularForce::write(out, indent+2);
  #endif //] NDEBUG
}
