// Filename: discEmitter.cxx
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "discEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : DiscEmitter::DiscEmitter
//      Access : Public 
// Description : constructor
////////////////////////////////////////////////////////////////////
DiscEmitter::
DiscEmitter(void) {
  _radius = 0.0f;
  _inner_aoe = _outer_aoe = 0.0f;
  _inner_magnitude = _outer_magnitude = 0.0f;
  _cubic_lerping = false;
}

////////////////////////////////////////////////////////////////////
//    Function : DiscEmitter::DiscEmitter
//      Access : Public 
// Description : copy constructor
////////////////////////////////////////////////////////////////////
DiscEmitter::
DiscEmitter(const DiscEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _inner_aoe = copy._inner_aoe;
  _outer_aoe = copy._outer_aoe;
  _inner_magnitude = copy._inner_magnitude;
  _outer_magnitude = copy._outer_magnitude;
  _cubic_lerping = copy._cubic_lerping;
}

////////////////////////////////////////////////////////////////////
//    Function : DiscEmitter::~DiscEmitter
//      Access : Public 
// Description : destructor
////////////////////////////////////////////////////////////////////
DiscEmitter::
~DiscEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public 
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *DiscEmitter::
make_copy(void) {
  return new DiscEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : DiscEmitter::create_particle_location 
//      Access : Public 
// Description : Generates a location on the disc
////////////////////////////////////////////////////////////////////
void DiscEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel) {
  // position
  float theta = bounded_rand() * 2.0f * MathNumbers::pi;

  float t = bounded_rand();
  float r_scalar = t * _radius;

  float sinf_theta = sinf(theta);
  float cosf_theta = cosf(theta);

  float new_x = cosf_theta * r_scalar;
  float new_y = sinf_theta * r_scalar;

  pos.set(new_x, new_y, 0.0f);

  // lerp type
  float aoe, mag;

  if (_cubic_lerping == true) {
    aoe = cubic_lerp(t, _inner_aoe, _outer_aoe);
    mag = cubic_lerp(t, _inner_magnitude, _outer_magnitude);
  }
  else {
    aoe = lerp(t, _inner_aoe, _outer_aoe);
    mag = lerp(t, _inner_magnitude, _outer_magnitude);
  }

  // velocity
  float vel_z = mag * sinf(aoe * (MathNumbers::pi / 180.0f));
  float abs_diff = fabs((mag * mag) - (vel_z * vel_z));
  float root_mag_minus_z_squared = sqrtf(abs_diff);
  float vel_x = cosf_theta * root_mag_minus_z_squared;
  float vel_y = sinf_theta * root_mag_minus_z_squared;

  vel.set(vel_x, vel_y, vel_z);
}
