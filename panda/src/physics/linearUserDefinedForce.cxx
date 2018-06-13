/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearUserDefinedForce.cxx
 * @author charles
 * @date 2000-07-31
 */

#include "linearUserDefinedForce.h"

TypeHandle LinearUserDefinedForce::_type_handle;

/**
 * constructor
 */
LinearUserDefinedForce::
LinearUserDefinedForce(LVector3 (*proc)(const PhysicsObject *),
    PN_stdfloat a, bool md) :
  LinearForce(a, md),
  _proc(proc)
{
}

/**
 * copy constructor
 */
LinearUserDefinedForce::
LinearUserDefinedForce(const LinearUserDefinedForce &copy) :
  LinearForce(copy) {
  _proc = copy._proc;
}

/**
 * destructor
 */
LinearUserDefinedForce::
~LinearUserDefinedForce() {
}

/**
 * child copier
 */
LinearForce *LinearUserDefinedForce::
make_copy() {
  return new LinearUserDefinedForce(*this);
}

/**
 * force builder
 */
LVector3 LinearUserDefinedForce::
get_child_vector(const PhysicsObject *po) {
  return _proc(po);
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearUserDefinedForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearUserDefinedForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearUserDefinedForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearUserDefinedForce:\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
