// Filename: tangentRingEmitter.C
// Created by:  charles (25Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "tangentRingEmitter.h"

////////////////////////////////////////////////////////////////////
//     Function : tangentRingEmitter
//       Access : public
//  Description : constructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
TangentRingEmitter(void) {
  _radius = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function : tangentRingEmitter
//       Access : public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
TangentRingEmitter(const TangentRingEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
}

////////////////////////////////////////////////////////////////////
//     Function : ~tangentringemitter
//       Access : public, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
TangentRingEmitter::
~TangentRingEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : child copier
////////////////////////////////////////////////////////////////////
BaseParticleEmitter *TangentRingEmitter::
make_copy(void) {
  return new TangentRingEmitter(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : TangentRingEmitter::assign_initial_position
//      Access : Public
// Description : Generates a location for a new particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_position(LPoint3f& pos) {
  float theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi;

  _x = cosf(theta);
  _y = sinf(theta);

  pos.set(_radius * _x, _radius * _y, 0);
}

////////////////////////////////////////////////////////////////////
//    Function : TangentRingEmitter::assign_initial_velocity
//      Access : Public
// Description : Generates a velocity for a new particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_velocity(LVector3f& vel) {
  vel.set(-_y, _x, 0);
}
