// Filename: ringEmitter.cxx
// Created by:  charles (22Jun00)
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

#include "ringEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
RingEmitter::
RingEmitter() :
  _radius(1.0f), _aoe(0.0f), _radius_spread(0.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
RingEmitter::
RingEmitter(const RingEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _aoe = copy._aoe;
  _radius_spread = copy._radius_spread;

  _sin_theta = copy._sin_theta;
  _cos_theta = copy._cos_theta;
}

////////////////////////////////////////////////////////////////////
//    Function : ~RingEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
RingEmitter::
~RingEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *RingEmitter::
make_copy() {
  return new RingEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void RingEmitter::
assign_initial_position(LPoint3f& pos) {
  float theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;
  _cos_theta = cosf(theta);
  _sin_theta = sinf(theta);

  float new_radius_spread = SPREAD(_radius_spread);
  float new_x = _cos_theta * (_radius + new_radius_spread);
  float new_y = _sin_theta * (_radius + new_radius_spread);

  pos.set(new_x, new_y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void RingEmitter::
assign_initial_velocity(LVector3f& vel) {
  float vel_z = sinf(deg_2_rad(_aoe));
  float abs_diff = fabs(1.0f - (vel_z * vel_z));
  float root_mag_minus_z_squared = sqrtf(abs_diff);

  float vel_x = _cos_theta * root_mag_minus_z_squared;
  float vel_y = _sin_theta * root_mag_minus_z_squared;

  // quick and dirty
  if((_aoe > 90.0f) && (_aoe < 270.0f))
  {
    vel_x = -vel_x;
    vel_y = -vel_y;
  }

  vel.set(vel_x, vel_y, vel_z);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void RingEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"RingEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void RingEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"RingEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  out.width(indent+2); out<<""; out<<"_radius_spread "<<_radius_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_aoe "<<_aoe<<"\n";
  out.width(indent+2); out<<""; out<<"_sin_theta "<<_sin_theta<<"\n";
  out.width(indent+2); out<<""; out<<"_cos_theta "<<_cos_theta<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
