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
  _radius(0.0f), _aoe(0.0f), _mag(0.0f) {
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
  _mag = copy._mag;
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
//    Function : RingEmitter::create_particle_location
//      Access : Public
// Description : Generates a location on the ring
////////////////////////////////////////////////////////////////////
void RingEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float theta = bounded_rand() * 2.0f * MathNumbers::pi;
  float cos_theta = cosf(theta);
  float sin_theta = sinf(theta);

  float new_x = cos_theta * _radius;
  float new_y = sin_theta * _radius;

  pos.set(new_x, new_y, 0.0f);

  float vel_z = _mag * sinf(_aoe * (MathNumbers::pi / 180.0f));
  float abs_diff = fabs((_mag *_mag) - (vel_z * vel_z));
  float root_mag_minus_z_squared = sqrtf(abs_diff);

  float vel_x = cos_theta * root_mag_minus_z_squared;
  float vel_y = sin_theta * root_mag_minus_z_squared;

  vel.set(vel_x, vel_y, vel_z);
}
