// Filename: lineEmitter.C
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
  _endpoint1.set(0.0f, 0.0f, 0.0f);
  _endpoint2.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter
//      Access : Public
// Description : constructor
////////////////////////////////////////////////////////////////////
LineEmitter::
LineEmitter(const LineEmitter &copy) :
  BaseParticleEmitter(copy) {
  _endpoint1 = copy._endpoint1;
  _endpoint2 = copy._endpoint2;
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
//    Function : LineEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void LineEmitter::
assign_initial_position(LPoint3f& pos) {
  float t = NORMALIZED_RAND();

  LVector3f v_diff = _endpoint2 - _endpoint1;

  float lerp_x = _endpoint1[0] + t * v_diff[0];
  float lerp_y = _endpoint1[1] + t * v_diff[1];
  float lerp_z = _endpoint1[2] + t * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
}

////////////////////////////////////////////////////////////////////
//    Function : LineEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void LineEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(0,0,0);
}
