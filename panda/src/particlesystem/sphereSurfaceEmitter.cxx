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
//    Function : SphereSurfaceEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
assign_initial_position(LPoint3f& pos) {
  float z, theta, r;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi;

  pos.set(r * cosf(theta), r * sinf(theta), z);
}

////////////////////////////////////////////////////////////////////
//    Function : SphereSurfaceEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void SphereSurfaceEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0,0,0);
}
