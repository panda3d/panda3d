// Filename: LinearIntegrator.h
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARINTEGRATOR_H
#define LINEARINTEGRATOR_H

#include "physicsObject.h"
#include "baseIntegrator.h"
#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearIntegrator
// Description : Pure virtual base class for physical modeling.
//               Takes physically modelable objects and applies
//               forces to them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearIntegrator : public BaseIntegrator {
private:
  static const float _max_linear_dt;

  // this allows baseLinearIntegrator to censor/modify data that the
  // actual integration function receives.
  virtual void child_integrate(Physical *physical, vector< PT(LinearForce) > &forces,
                               float dt) = 0;

protected:
  LinearIntegrator(void);

public:
  virtual ~LinearIntegrator(void);

  void integrate(Physical *physical, vector< PT(LinearForce) > &forces,
                 float dt);
};

#endif // LINEARINTEGRATOR_H
