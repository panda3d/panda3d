// Filename: physics_manager.cxx
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>

#include "physicsManager.h"
#include "actorNode.h"

////////////////////////////////////////////////////////////////////
//     Function : PhysicsManager
//       Access : Public
//  Description : Default Constructor.  NOTE: EulerIntegrator is
//                the standard default.
////////////////////////////////////////////////////////////////////
PhysicsManager::
PhysicsManager(void) {
  _linear_integrator.clear();
  _angular_integrator.clear();
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysicsManager
//       Access : Public
//  Description : Simple Destructor
////////////////////////////////////////////////////////////////////
PhysicsManager::
~PhysicsManager(void) {
}

////////////////////////////////////////////////////////////////////
//     Function : remove_linear_force
//       Access : Public
//  Description : takes a linear force out of the physics list
////////////////////////////////////////////////////////////////////
void PhysicsManager::
remove_linear_force(LinearForce *f) {
  vector< PT(LinearForce) >::iterator found;

  PT(LinearForce) ptbf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), ptbf);

  if (found == _linear_forces.end())
    return;

  _linear_forces.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : remove_angular_force
//       Access : Public
//  Description : takes an angular force out of the physics list
////////////////////////////////////////////////////////////////////
void PhysicsManager::
remove_angular_force(AngularForce *f) {
  vector< PT(AngularForce) >::iterator found;

  PT(BaseForce) ptbf = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), ptbf);

  if (found == _angular_forces.end())
    return;

  _angular_forces.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : remove_physical
//       Access : Public
//  Description : takes a physical out of the object list
////////////////////////////////////////////////////////////////////
void PhysicsManager::
remove_physical(Physical *p) {
  vector< Physical * >::iterator found;

  found = find(_physicals.begin(), _physicals.end(), p);
  if (found == _physicals.end())
    return;

  p->_physics_manager = (PhysicsManager *) NULL;
  _physicals.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : DoPhysics
//       Access : Public
//  Description : This is the main high-level API call.  Performs
//                integration on every attached Physical.
////////////////////////////////////////////////////////////////////
void PhysicsManager::
do_physics(float dt) {
  vector< Physical * >::iterator p_cur;

  // now, run through each physics object in the set.
  p_cur = _physicals.begin();

  for (; p_cur != _physicals.end(); p_cur++) {
    Physical *physical = *p_cur;

    // do linear
    if (_linear_integrator.is_null() == false) {
      _linear_integrator->integrate(physical, _linear_forces, dt);
    }

    // do angular
    if (_angular_integrator.is_null() == false) {
      _angular_integrator->integrate(physical, _angular_forces, dt);
    }

    // if it's an actor node, tell it to update itself.
    PhysicalNode *pn = physical->get_physical_node();
    if (pn->is_of_type(ActorNode::get_class_type())) {
      ActorNode *an = (ActorNode *) pn;
      an->update_arc();
    }
  }
}
