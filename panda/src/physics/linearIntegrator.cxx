// Filename: LinearIntegrator.C
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
/* <-- darren, 2000.10.06
  // cap dt so physics don't go flying off on lags
  if (dt > _max_linear_dt)
    dt = _max_linear_dt;
*/

  vector< PT(PhysicsObject) >::const_iterator current_object_iter;
  current_object_iter = physical->get_object_vector().begin();
  for (; current_object_iter != physical->get_object_vector().end();
       current_object_iter++) {
    PhysicsObject *current_object = *current_object_iter;

    // bail out if this object doesn't exist or doesn't want to be
    // processed.
    if (current_object == (PhysicsObject *) NULL)
      continue;

    // set the object's last position to its current position before we move it
    current_object->set_last_position(current_object->get_position());

  }

  child_integrate(physical, forces, dt);
}
