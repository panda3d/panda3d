// Filename: AngularIntegrator.h
// Created by:  charles (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ANGULARINTEGRATOR_H
#define ANGULARINTEGRATOR_H

#include "baseIntegrator.h"
#include "angularForce.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseAngularIntegrator
// Description : Pure virtual base class for physical modeling.
//               Takes physically modelable objects and applies
//               forces to them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularIntegrator : public BaseIntegrator {
private:
  static const float _max_angular_dt;

  // this allows baseAngularIntegrator to censor/modify data that the
  // actual integration function receives.
  virtual void child_integrate(Physical *physical, vector< PT(AngularForce) > &forces,
                               float dt) = 0;

protected:
  AngularIntegrator(void);

public:
  virtual ~AngularIntegrator(void);

  void integrate(Physical *physical, vector< PT(AngularForce) > &forces,
                 float dt);
};

#endif // ANGULARINTEGRATOR_H
