// Filename: pointEmitter.cxx
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
assign_initial_position(LPoint3f& pos) {
  pos = _location;
}

////////////////////////////////////////////////////////////////////
//    Function : PointEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void PointEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0,0,0);
}
