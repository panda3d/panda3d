// Filename: rectangleEmitter.cxx
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "rectangleEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
RectangleEmitter(void) :
  BaseParticleEmitter() {
  _vmin.set(0.0f, 0.0f);
  _vmax.set(0.0f, 0.0f);
  _launch_vec.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
RectangleEmitter(const RectangleEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
  _launch_vec = copy._launch_vec;
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter
//      Access : Public
// Description : destructor
////////////////////////////////////////////////////////////////////
RectangleEmitter::
~RectangleEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *RectangleEmitter::
make_copy(void) {
  return new RectangleEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter::create_particle_location
//      Access : Public
// Description : Generates a location on the rectangle
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float t_x = bounded_rand();
  float t_y = bounded_rand();

  LVector2f v_diff = _vmax - _vmin;

  float lerp_x = _vmin[0] + t_x * v_diff[0];
  float lerp_y = _vmin[1] + t_y * v_diff[1];

  pos.set(lerp_x, lerp_y, 0.0f);
  vel = _launch_vec;
}
