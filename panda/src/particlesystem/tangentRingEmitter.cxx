// Filename: tangentRingEmitter.cxx
// Created by:  charles (25Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
assign_initial_position(LPoint3f& pos) {
  float theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  _x = cosf(theta);
  _y = sinf(theta);

  pos.set(_radius * _x, _radius * _y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : TangentRingEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_velocity(LVector3f& vel) {
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
  out.width(indent+2); out<<""; out<<"_x "<<_x<<"\n";
  out.width(indent+2); out<<""; out<<"_y "<<_y<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
