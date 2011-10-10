// Filename: sphereSurfaceEmitter.cxx
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

#include "sphereSurfaceEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
SphereSurfaceEmitter() {
  _radius = 1.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
SphereSurfaceEmitter(const SphereSurfaceEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
}

////////////////////////////////////////////////////////////////////
//    Function : ~SphereSurfaceEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
~SphereSurfaceEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *SphereSurfaceEmitter::
make_copy() {
  return new SphereSurfaceEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat z, theta, r;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  pos.set(r * cosf(theta), r * sinf(theta), z);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SphereSurfaceEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"SphereSurfaceEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
