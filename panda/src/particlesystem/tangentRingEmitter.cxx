// Filename: tangentRingEmitter.cxx
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
//     Function : assign_initial_values
//       Access : private, virtual
//  Description : fills in a newly-born particle
////////////////////////////////////////////////////////////////////
void TangentRingEmitter::
assign_initial_values(LPoint3f& pos, LVector3f& vel) {
  float theta = bounded_rand() * 2.0f * MathNumbers::pi;

  float x = _radius * cosf(theta);
  float y = _radius * sinf(theta);

  pos.set(x, y, 0);
  vel.set(-y, x, 0);
}
