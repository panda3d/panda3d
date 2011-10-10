// Filename: sphereVolumeEmitter.cxx
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

#include "sphereVolumeEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
SphereVolumeEmitter::
SphereVolumeEmitter() {
  _radius = 1.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
SphereVolumeEmitter::
SphereVolumeEmitter(const SphereVolumeEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _particle_pos = copy._particle_pos;
}

////////////////////////////////////////////////////////////////////
//    Function : ~SphereVolumeEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
SphereVolumeEmitter::
~SphereVolumeEmitter() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *SphereVolumeEmitter::
make_copy() {
  return new SphereVolumeEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat z, theta, r;
  PN_stdfloat t;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  t = NORMALIZED_RAND();

  while (t == 0.0f)
    t = NORMALIZED_RAND();

  PN_stdfloat pos_x = r * cosf(theta) * t;
  PN_stdfloat pos_y = r * sinf(theta) * t;
  PN_stdfloat pos_z = z * t;

  _particle_pos.set(pos_x, pos_y, pos_z);
  pos = _particle_pos;
}

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
assign_initial_velocity(LVector3& vel) {
  // set velocity to [0..1] according to distance from center,
  // along vector from center to position
  vel = _particle_pos / _radius;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SphereVolumeEmitter";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"SphereVolumeEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}
