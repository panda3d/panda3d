// Filename: linearJitterForce.cxx
// Created by:  charles (16Jun00)
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

#include "linearJitterForce.h"

TypeHandle LinearJitterForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearJitterForce
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LinearJitterForce::
LinearJitterForce(PN_stdfloat a, bool mass) :
  LinearRandomForce(a, mass) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearJitterForce
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearJitterForce::
LinearJitterForce(const LinearJitterForce &copy) :
  LinearRandomForce(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearJitterForce
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LinearJitterForce::
~LinearJitterForce() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearJitterForce::
make_copy() {
  return new LinearJitterForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : Public
// Description : random value
////////////////////////////////////////////////////////////////////
LVector3 LinearJitterForce::
get_child_vector(const PhysicsObject *) {
  return random_unit_vector();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearJitterForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearJitterForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearJitterForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearJitterForce:\n";
  LinearRandomForce::write(out, indent+2);
  #endif //] NDEBUG
}
