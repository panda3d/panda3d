// Filename: lineEmitter.cxx
// Created by:  charles (22Jun00)
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

#include "lineEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
LineEmitter() :
  BaseParticleEmitter() {
  _endpoint1.set(1.0f, 0.0f, 0.0f);
  _endpoint2.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
LineEmitter(const LineEmitter &copy) :
  BaseParticleEmitter(copy) {
  _endpoint1 = copy._endpoint1;
  _endpoint2 = copy._endpoint2;
}

////////////////////////////////////////////////////////////////////
//    Function : ~LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
~LineEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *LineEmitter::
make_copy() {
  return new LineEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void LineEmitter::
assign_initial_position(LPoint3f& pos) {
  float t = NORMALIZED_RAND();

  LVector3f v_diff = _endpoint2 - _endpoint1;

  float lerp_x = _endpoint1[0] + t * v_diff[0];
  float lerp_y = _endpoint1[1] + t * v_diff[1];
  float lerp_z = _endpoint1[2] + t * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void LineEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0,0,0);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LineEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LineEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void LineEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LineEmitter:\n";
  out.width(indent+2); out<<""; out<<"_endpoint1 "<<_endpoint1<<"\n";
  out.width(indent+2); out<<""; out<<"_endpoint2 "<<_endpoint2<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
