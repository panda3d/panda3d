// Filename: lineareulerintegrator.h
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEAREULERINTEGRATOR_H
#define LINEAREULERINTEGRATOR_H

#include "linearIntegrator.h"

////////////////////////////////////////////////////////////////////
//       Class : LINEAREulerIntegrator
// Description : Performs Euler integration on a vector of
//               physically modelable objects given a quantum dt.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearEulerIntegrator : public LinearIntegrator {
private:
  virtual void child_integrate(Physical *physical,
			       vector< PT(LinearForce) >& forces,
			       float dt);

public:
  LinearEulerIntegrator(void);
  virtual ~LinearEulerIntegrator(void);
};

#endif // EULERINTEGRATOR_H
