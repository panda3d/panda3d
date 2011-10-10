// Filename: linearRandomForce.cxx
// Created by:  charles (19Jun00)
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

#include "linearRandomForce.h"

TypeHandle LinearRandomForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearRandomForce
//      Access : Protected
// Description : vector constructor
////////////////////////////////////////////////////////////////////
LinearRandomForce::
LinearRandomForce(PN_stdfloat a, bool mass) :
  LinearForce(a, mass) {
}

////////////////////////////////////////////////////////////////////
//    Function : LinearRandomForce
//      Access : Protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearRandomForce::
LinearRandomForce(const LinearRandomForce &copy) :
  LinearForce(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearRandomForce
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearRandomForce::
~LinearRandomForce() {
}

////////////////////////////////////////////////////////////////////
//     Function : bounded_rand
//       Access : Protected
//  Description : Returns a float in [0, 1]
////////////////////////////////////////////////////////////////////
PN_stdfloat LinearRandomForce::
bounded_rand() {
  return ((PN_stdfloat)rand() / (PN_stdfloat)RAND_MAX);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearRandomForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearRandomForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearRandomForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearRandomForce:\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
