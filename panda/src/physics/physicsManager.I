// Filename: physics_manager.I
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : attach_physical
//      Access : Public
// Description : Registers a Physical class with the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_physical(Physical *p) {
  p->_physics_manager = this;
  vector< Physical * >::iterator found;
  found = find(_physicals.begin(), _physicals.end(), p);
  if (found == _physicals.end())
    _physicals.push_back(p);
}

////////////////////////////////////////////////////////////////////
//    Function : attach_linear_force
//      Access : Public
// Description : Adds a global linear force to the physics manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
add_linear_force(LinearForce *f) {
  vector< PT(LinearForce) >::iterator found;
  PT(LinearForce) ptlf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), ptlf);
  if (found == _linear_forces.end())
    _linear_forces.push_back(f);
}

////////////////////////////////////////////////////////////////////
//    Function : attach_physicalnode
//      Access : Public
// Description : Registers an physicalnode with the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_physicalnode(PhysicalNode *p) {
  for (int i = 0; i < p->get_num_physicals(); i++)
    attach_physical(p->get_physical(i));
}

////////////////////////////////////////////////////////////////////
//    Function : clear_linear_forces
//      Access : Public
// Description : Resets the physics manager force vector
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
clear_linear_forces(void) {
  _linear_forces.erase(_linear_forces.begin(), _linear_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : attach_angular_force
//      Access : Public
// Description : Adds a global angular force to the physics manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
add_angular_force(AngularForce *f) {
  vector< PT(AngularForce) >::iterator found;
  PT(AngularForce) ptaf = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), ptaf);
  if (found == _angular_forces.end())
    _angular_forces.push_back(f);
}

////////////////////////////////////////////////////////////////////
//    Function : clear_angular_forces
//      Access : Public
// Description : Resets the physics manager force vector
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
clear_angular_forces(void) {
  _angular_forces.erase(_angular_forces.begin(), _angular_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : clear_physicals
//      Access : Public
// Description : Resets the physics manager objects vector
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
clear_physicals(void) {
  _physicals.erase(_physicals.begin(), _physicals.end());
}

////////////////////////////////////////////////////////////////////
//    Function : attach_linear_integrator
//      Access : Public
// Description : Hooks a linear integrator into the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_linear_integrator(LinearIntegrator *i) {
  _linear_integrator = i;
}

////////////////////////////////////////////////////////////////////
//    Function : attach_angular_integrator
//      Access : Public
// Description : Hooks an angular integrator into the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_angular_integrator(AngularIntegrator *i) {
  _angular_integrator = i;
}
