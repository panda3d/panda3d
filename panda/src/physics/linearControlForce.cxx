/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearControlForce.cxx
 * @author Dave Schuyler
 * @date 2006
 */

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include "linearControlForce.h"

TypeHandle LinearControlForce::_type_handle;

/**
 * Vector Constructor
 */
LinearControlForce::
LinearControlForce(const PhysicsObject *po, PN_stdfloat a, bool mass) :
  LinearForce(a, mass),
  _physics_object(po),
  _fvec(0.0f, 0.0f, 0.0f) {
}

/**
 * Copy Constructor
 */
LinearControlForce::
LinearControlForce(const LinearControlForce &copy) :
  LinearForce(copy) {
  _physics_object = copy._physics_object;
  _fvec = copy._fvec;
}

/**
 * Destructor
 */
LinearControlForce::
~LinearControlForce() {
}

/**
 * copier
 */
LinearForce *LinearControlForce::
make_copy() {
  return new LinearControlForce(*this);
}

/**
 * vector access
 */
LVector3 LinearControlForce::
get_child_vector(const PhysicsObject *po) {
  if (_physics_object != nullptr && po == _physics_object) {
    return _fvec;
  } else {
    return LVector3::zero();
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearControlForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearControlForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearControlForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearControlForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  out.width(indent+2); out<<""; out<<"_physics_object "<<_physics_object<<"\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
