// Filename: AngularIntegrator.cxx
// Created by:  charles (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "angularIntegrator.h"

////////////////////////////////////////////////////////////////////
//    Function : AngularIntegrator
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
AngularIntegrator::
AngularIntegrator(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~AngularIntegrator
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
AngularIntegrator::
~AngularIntegrator(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : Integrate
//      Access : public
// Description : high-level integration.  API.
////////////////////////////////////////////////////////////////////
void AngularIntegrator::
integrate(Physical *physical, vector< PT(AngularForce) >& forces,
          float dt) {
  // intercept in case we want to censor/adjust values
  if (dt > _max_angular_dt)
    dt = _max_angular_dt;

  // this actually does the integration.
  child_integrate(physical, forces, dt);
}
