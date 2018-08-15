/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearVectorForce.cxx
 * @author charles
 * @date 2000-06-14
 */

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include "linearVectorForce.h"

TypeHandle LinearVectorForce::_type_handle;

/**
 * Vector Constructor
 */
LinearVectorForce::
LinearVectorForce(const LVector3& vec, PN_stdfloat a, bool mass) :
  LinearForce(a, mass),
  _fvec(vec) {
}

/**
 * Default/Piecewise constructor
 */
LinearVectorForce::
LinearVectorForce(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat a, bool mass) :
  LinearForce(a, mass) {
  _fvec.set(x, y, z);
}

/**
 * Copy Constructor
 */
LinearVectorForce::
LinearVectorForce(const LinearVectorForce &copy) :
  LinearForce(copy) {
  _fvec = copy._fvec;
}

/**
 * Destructor
 */
LinearVectorForce::
~LinearVectorForce() {
}

/**
 * copier
 */
LinearForce *LinearVectorForce::
make_copy() {
  return new LinearVectorForce(*this);
}

/**
 * vector access
 */
LVector3 LinearVectorForce::
get_child_vector(const PhysicsObject *) {
  return _fvec;
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearVectorForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearVectorForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearVectorForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearVectorForce:\n";
  out.width(indent+2); out<<""; out<<"_fvec "<<_fvec<<"\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
