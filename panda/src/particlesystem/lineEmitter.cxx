// Filename: lineEmitter.cxx
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "lineEmitter.h"

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
LineEmitter(void) :
  BaseParticleEmitter() {
  _vmin.set(0.0f, 0.0f, 0.0f);
  _vmax.set(0.0f, 0.0f, 0.0f);
  _launch_vec.set(0,0,0);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
LineEmitter(const LineEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
  _launch_vec = copy._launch_vec;
}

////////////////////////////////////////////////////////////////////
//    Function : ~LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
~LineEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_copy
//      Access : Public
// Description : copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *LineEmitter::
make_copy(void) {
  return new LineEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter::create_particle_location
//      Access : Public
// Description : Generates a location on the line
////////////////////////////////////////////////////////////////////
void LineEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel)
{
  float t = bounded_rand();

  LVector3f v_diff = _vmax - _vmin;

  float lerp_x = _vmin[0] + t * v_diff[0];
  float lerp_y = _vmin[1] + t * v_diff[1];
  float lerp_z = _vmin[2] + t * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
  vel = _launch_vec;
}
