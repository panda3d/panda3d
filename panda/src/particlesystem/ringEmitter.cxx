// Filename: ringEmitter.cxx
// Created by:  charles (22Jun00)
//
////////////////////////////////////////////////////////////////////

#include "ringEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
RingEmitter::
RingEmitter(void) :
  _radius(1.0f), _aoe(0.0f) {
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

  _sin_theta = copy._sin_theta;
  _cos_theta = copy._cos_theta;
}

////////////////////////////////////////////////////////////////////
//    Function : ~RingEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
RingEmitter::
~RingEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *RingEmitter::
make_copy(void) {
  return new RingEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void RingEmitter::
assign_initial_position(LPoint3f& pos) {
  float theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi;
  _cos_theta = cosf(theta);
  _sin_theta = sinf(theta);

  float new_x = _cos_theta * _radius;
  float new_y = _sin_theta * _radius;

  pos.set(new_x, new_y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : RingEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void RingEmitter::
assign_initial_velocity(LVector3f& vel) {
  float vel_z = sinf(_aoe * (MathNumbers::pi / 180.0f));
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
