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
  _radius = 0.0f;
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
//    Function : SphereVolumeEmitter::create_particle_location
//      Access : Public
// Description : Generates a location in the sphere
////////////////////////////////////////////////////////////////////
void SphereVolumeEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float z, theta, r;
  float t;

  z = _radius - (2.0f * _radius * bounded_rand());
  r = sqrtf((_radius * _radius) - (z * z));
  theta = bounded_rand() * 2.0f * MathNumbers::pi;

  t = bounded_rand();

  while (t == 0.0f)
    t = bounded_rand();

  float pos_x = r * cosf(theta) * t;
  float pos_y = r * sinf(theta) * t;
  float pos_z = z * t;

  pos.set(pos_x, pos_y, pos_z);
  vel = pos / _radius;
}
