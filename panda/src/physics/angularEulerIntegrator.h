// Filename: angularEulerIntergator.h
// Created by:  charles (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ANGULAREULERINTEGRATOR_H
#define ANGULAREULERINTEGRATOR_H

#include "angularIntegrator.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularEulerIntegrator
// Description : Performs Euler integration on a vector of
//               physically modelable objects given a quantum dt.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularEulerIntegrator : public AngularIntegrator {
private:
  virtual void child_integrate(Physical *physical,
                               vector< PT(AngularForce) >& forces,
                               float dt);

PUBLISHED:
  AngularEulerIntegrator(void);
  virtual ~AngularEulerIntegrator(void);
};

#endif // EULERINTEGRATOR_H
