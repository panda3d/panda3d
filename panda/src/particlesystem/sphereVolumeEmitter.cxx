// Filename: sphereVolumeEmitter.cxx
// Created by:  charles (22Jun00)
//
////////////////////////////////////////////////////////////////////

#include "sphereVolumeEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
SphereVolumeEmitter::
SphereVolumeEmitter(void) {
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
~SphereVolumeEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *SphereVolumeEmitter::
make_copy(void) {
  return new SphereVolumeEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
assign_initial_position(LPoint3f& pos) {
  float z, theta, r;
  float t;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi;

  t = NORMALIZED_RAND();

  while (t == 0.0f)
    t = NORMALIZED_RAND();

  float pos_x = r * cosf(theta) * t;
  float pos_y = r * sinf(theta) * t;
  float pos_z = z * t;

  _particle_pos.set(pos_x, pos_y, pos_z);
  pos = _particle_pos;
}

////////////////////////////////////////////////////////////////////
//    Function : SphereVolumeEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
assign_initial_velocity(LVector3f& vel) {
  // set velocity to [0..1] according to distance from center,
  // along vector from center to position
  vel = _particle_pos / _radius;
}
