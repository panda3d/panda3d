// Filename: baseParticleEmitter.cxx
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "baseParticleEmitter.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleEmitter
//      Access : Protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseParticleEmitter::
BaseParticleEmitter(void) {
  _amplitude = 1.0f;
  _offset_force.set(0,0,0);
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleEmitter
//      Access : Protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BaseParticleEmitter::
BaseParticleEmitter(const BaseParticleEmitter &copy) {
  _offset_force = copy._offset_force;
  _amplitude = copy._amplitude;
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleEmitter
//      Access : Protected
// Description : destructor
////////////////////////////////////////////////////////////////////
BaseParticleEmitter::
~BaseParticleEmitter(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleEmitter::bounded_rand
//      Access : Private
// Description : Random number in [0, 1]
////////////////////////////////////////////////////////////////////
float BaseParticleEmitter::bounded_rand(void)
{
  int value = rand() & 0x7fffffff;
  float f_value = (float) value / (float) 0x7fffffff;

  return f_value;
}

