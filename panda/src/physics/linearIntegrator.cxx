// Filename: LinearIntegrator.cxx
// Created by:  charles (02Aug00)
// 
////////////////////////////////////////////////////////////////////

#include <get_rel_pos.h>

#include "linearIntegrator.h"
#include "config_physics.h"
#include "physicalNode.h"
#include "forceNode.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseLinearIntegrator
//      Access : Protected
// Description : constructor
////////////////////////////////////////////////////////////////////
LinearIntegrator::
LinearIntegrator(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~LinearIntegrator
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
LinearIntegrator::
~LinearIntegrator(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : integrate
//      Access : public
// Description : parent integration routine, hands off to child
//               virtual.
////////////////////////////////////////////////////////////////////
void LinearIntegrator::
integrate(Physical *physical, vector< PT(LinearForce) > &forces,
	  float dt) {
  // cap dt so physics don't go flying off on lags
  if (dt > _max_linear_dt)
    dt = _max_linear_dt;

  child_integrate(physical, forces, dt);
}
