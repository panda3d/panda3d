// Filename: boxEmitter.cxx
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "boxEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
BoxEmitter(void) :
  BaseParticleEmitter() {
  _vmin.set(0.0f, 0.0f, 0.0f);
  _vmax.set(0.0f, 0.0f, 0.0f);
  _launch_vec.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
BoxEmitter(const BoxEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
  _launch_vec = copy._launch_vec;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BoxEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
BoxEmitter::
~BoxEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *BoxEmitter::
make_copy(void) {
  return new BoxEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : BoxEmitter::create_particle_location
//      Access : Public
// Description : Generates a location in the box
////////////////////////////////////////////////////////////////////
void BoxEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float t_x = bounded_rand();
  float t_y = bounded_rand();
  float t_z = bounded_rand();

  LVector3f v_diff = _vmax - _vmin;

  float lerp_x = _vmin[0] + t_x * v_diff[0];
  float lerp_y = _vmin[1] + t_y * v_diff[1];
  float lerp_z = _vmin[2] + t_z * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
  vel = _launch_vec;
}
