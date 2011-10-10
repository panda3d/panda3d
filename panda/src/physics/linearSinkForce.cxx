// Filename: linearSinkForce.cxx
// Created by:  charles (21Jun00)
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

#include "linearSinkForce.h"

TypeHandle LinearSinkForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearSinkForce
//      Access : Public
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearSinkForce::
LinearSinkForce(const LPoint3& p, FalloffType f, PN_stdfloat r, PN_stdfloat a,
                bool mass) :
  LinearDistanceForce(p, f, r, a, mass) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearSinkForce
//      Access : Public
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearSinkForce::
LinearSinkForce() :
  LinearDistanceForce(LPoint3(0.0f, 0.0f, 0.0f), FT_ONE_OVER_R_SQUARED,
                      1.0f, 1.0f, true) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearSinkForce
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearSinkForce::
LinearSinkForce(const LinearSinkForce &copy) :
  LinearDistanceForce(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearSinkForce
//      Access : Public
// Description : Simple destructor
////////////////////////////////////////////////////////////////////
LinearSinkForce::
~LinearSinkForce() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
LinearForce *LinearSinkForce::
make_copy() {
  return new LinearSinkForce(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : get_child_vector
//      Access : Public
// Description : virtual force query
////////////////////////////////////////////////////////////////////
LVector3 LinearSinkForce::
get_child_vector(const PhysicsObject *po) {
  return (get_force_center() - po->get_position()) * get_scalar_term();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearSinkForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearSinkForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearSinkForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearSinkForce:\n";
  LinearDistanceForce::write(out, indent+2);
  #endif //] NDEBUG
}
