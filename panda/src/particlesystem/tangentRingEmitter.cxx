// Filename: tangentRingEmitter.cxx
// Created by:  charles (25Jul00)
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

#include "tangentRingEmitter.h"

////////////////////////////////////////////////////////////////////
//     Function : tangentRingEmitter
//       Access : public
//  Description : constructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
TangentRingEmitter() {
  _radius = 1.0f;
  _radius_spread = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function : tangentRingEmitter
//       Access : public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
TangentRingEmitter(const TangentRingEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _radius_spread = copy._radius_spread;
}

////////////////////////////////////////////////////////////////////
//     Function : ~tangentringemitter
//       Access : public, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
~TangentRingEmitter() {
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : child copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *TangentRingEmitter::
make_copy() {
  return new TangentRingEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : TangentRingEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  _x = cosf(theta);
  _y = sinf(theta);

  PN_stdfloat new_radius = _radius + SPREAD(_radius_spread);
  pos.set(new_radius * _x, new_radius * _y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : TangentRingEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(-_y, _x, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"TangentRingEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"TangentRingEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  out.width(indent+2); out<<""; out<<"_radius_spread "<<_radius_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_x "<<_x<<"\n";
  out.width(indent+2); out<<""; out<<"_y "<<_y<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
