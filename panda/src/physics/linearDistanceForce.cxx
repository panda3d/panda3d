// Filename: linearDistanceForce.cxx
// Created by:  charles (21Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
LinearDistanceForce(const LPoint3f& p, FalloffType ft, float r, float a, bool m) :
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
~LinearDistanceForce(void) {
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
