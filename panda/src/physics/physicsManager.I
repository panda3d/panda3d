// Filename: physicsManager.I
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : attach_physical
//      Access : Public
// Description : Registers a Physical class with the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_physical(Physical *p) {
  nassertv(p);
  p->_physics_manager = this;
  pvector< Physical * >::iterator found;
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
  nassertv(f);
  pvector< PT(LinearForce) >::iterator found;
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
  nassertv(p);
  for (int i = 0; i < p->get_num_physicals(); i++)
    attach_physical(p->get_physical(i));
}

////////////////////////////////////////////////////////////////////
//    Function : clear_linear_forces
//      Access : Public
// Description : Resets the physics manager force vector
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
clear_linear_forces() {
  _linear_forces.erase(_linear_forces.begin(), _linear_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : attach_angular_force
//      Access : Public
// Description : Adds a global angular force to the physics manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
add_angular_force(AngularForce *f) {
  nassertv(f);
  pvector< PT(AngularForce) >::iterator found;
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
clear_angular_forces() {
  _angular_forces.erase(_angular_forces.begin(), _angular_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : clear_physicals
//      Access : Public
// Description : Resets the physics manager objects vector
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
clear_physicals() {
  _physicals.erase(_physicals.begin(), _physicals.end());
}

////////////////////////////////////////////////////////////////////
//    Function : set_viscosity
//      Access : Public
// Description : Set the global viscosity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
set_viscosity(float viscosity) {
  _viscosity=viscosity;
}

////////////////////////////////////////////////////////////////////
//    Function : get_viscosity
//      Access : Public
// Description : Get the global viscosity.
////////////////////////////////////////////////////////////////////
INLINE float PhysicsManager::
get_viscosity() const {
  return _viscosity;
}

////////////////////////////////////////////////////////////////////
//    Function : attach_linear_integrator
//      Access : Public
// Description : Hooks a linear integrator into the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_linear_integrator(LinearIntegrator *i) {
  nassertv(i);
  _linear_integrator = i;
}

////////////////////////////////////////////////////////////////////
//    Function : attach_angular_integrator
//      Access : Public
// Description : Hooks an angular integrator into the manager
////////////////////////////////////////////////////////////////////
INLINE void PhysicsManager::
attach_angular_integrator(AngularIntegrator *i) {
  nassertv(i);
  _angular_integrator = i;
}
