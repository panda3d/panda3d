// Filename: pointEmitter.cxx
// Created by:  charles (22Jun00)
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

#include "pointEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : PointEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
PointEmitter::
PointEmitter() :
  BaseParticleEmitter() {
  _location.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : PointEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
PointEmitter::
PointEmitter(const PointEmitter &copy) :
  BaseParticleEmitter(copy) {
  _location = copy._location;
}

////////////////////////////////////////////////////////////////////
//    Function : ~PointEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
PointEmitter::
~PointEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *PointEmitter::
make_copy() {
  return new PointEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : PointEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void PointEmitter::
assign_initial_position(LPoint3& pos) {
  pos = _location;
}

////////////////////////////////////////////////////////////////////
//    Function : PointEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void PointEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointEmitter:\n";
  out.width(indent+2); out<<""; out<<"_location "<<_location<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
