// Filename: linearDistanceForce.cxx
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

#include "linearDistanceForce.h"

TypeHandle LinearDistanceForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : LinearDistanceForce
//      Access : Protected
// Description : Simple constructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
LinearDistanceForce(const LPoint3& p, FalloffType ft, PN_stdfloat r, PN_stdfloat a, bool m) :
  LinearForce(a, m),
  _force_center(p), _falloff(ft), _radius(r)
{
}

////////////////////////////////////////////////////////////////////
//    Function : LinearDistanceForce
//      Access : Protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
LinearDistanceForce(const LinearDistanceForce &copy) :
  LinearForce(copy) {
  _falloff = copy._falloff;
  _radius = copy._radius;
  _force_center = copy._force_center;
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearDistanceForce
//      Access : Protected
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearDistanceForce::
~LinearDistanceForce() {
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearDistanceForce::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearDistanceForce";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LinearDistanceForce::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearDistanceForce:\n";
  out.width(indent+2); out<<""; out<<"_force_center "<<_force_center<<"\n";
  out.width(indent+2); out<<""; out<<"_falloff "<<_falloff<<"\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}
