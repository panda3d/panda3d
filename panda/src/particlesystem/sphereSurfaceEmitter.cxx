// Filename: sphereSurfaceEmitter.cxx
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "sphereSurfaceEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
SphereSurfaceEmitter(void) {
  _radius = 0.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
SphereSurfaceEmitter(const SphereSurfaceEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
}

////////////////////////////////////////////////////////////////////
//    Function : ~SphereSurfaceEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
SphereSurfaceEmitter::
~SphereSurfaceEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *SphereSurfaceEmitter::
make_copy(void) {
  return new SphereSurfaceEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter::create_particle_location
//      Access : Public
// Description : Generates a location on the sphere
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float z, theta, r;
  float t;

  z = _radius - (2.0f * _radius * bounded_rand());
  r = sqrtf((_radius * _radius) - (z * z));
  theta = bounded_rand() * 2.0f * MathNumbers::pi;

  pos.set(r * cosf(theta), r * sinf(theta), z);
  vel = pos / _radius;
}
