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
//    Function : RectangleEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
assign_initial_position(LPoint3f& pos) {
  float t_x = NORMALIZED_RAND();
  float t_y = NORMALIZED_RAND();

  LVector2f v_diff = _vmax - _vmin;

  float lerp_x = _vmin[0] + t_x * v_diff[0];
  float lerp_y = _vmin[1] + t_y * v_diff[1];

  pos.set(lerp_x, lerp_y, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : RectangleEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void RectangleEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0.0f,0.0f,0.0f);
}
