// Filename: rectangleEmitter.cxx
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

#include "rectangleEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
RectangleEmitter() :
  BaseParticleEmitter() {
  _vmin.set(-0.5f, -0.5f);
  _vmax.set( 0.5f,  0.5f);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
RectangleEmitter(const RectangleEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
~RectangleEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *RectangleEmitter::
make_copy() {
  return new RectangleEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat t_x = NORMALIZED_RAND();
  PN_stdfloat t_y = NORMALIZED_RAND();

  LVector2 v_diff = _vmax - _vmin;

  PN_stdfloat lerp_x = _vmin[0] + t_x * v_diff[0];
  PN_stdfloat lerp_y = _vmin[1] + t_y * v_diff[1];

  pos.set(lerp_x, lerp_y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0.0f,0.0f,0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"RectangleEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"RectangleEmitter:\n";
  out.width(indent+2); out<<""; out<<"_vmin "<<_vmin<<"\n";
  out.width(indent+2); out<<""; out<<"_vmax "<<_vmax<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
