// Filename: boxEmitter.cxx
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

#include "boxEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
BoxEmitter() :
  BaseParticleEmitter() {
  _vmin.set(-0.5f, -0.5f, -0.5f);
  _vmax.set( 0.5f,  0.5f,  0.5f);
}

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
BoxEmitter(const BoxEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BoxEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
~BoxEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *BoxEmitter::
make_copy() {
  return new BoxEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void BoxEmitter::
assign_initial_position(LPoint3f& pos) {
  float t_x = NORMALIZED_RAND();
  float t_y = NORMALIZED_RAND();
  float t_z = NORMALIZED_RAND();

  LVector3f v_diff = _vmax - _vmin;

  float lerp_x = _vmin[0] + t_x * v_diff[0];
  float lerp_y = _vmin[1] + t_y * v_diff[1];
  float lerp_z = _vmin[2] + t_z * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
}

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void BoxEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0,0,0);
}
