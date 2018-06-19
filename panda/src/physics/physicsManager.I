/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsManager.I
 * @author charles
 * @date 2000-06-14
 */

/**
 * Registers a Physical class with the manager
 */
INLINE void PhysicsManager::
attach_physical(Physical *p) {
  nassertv(p && p->_physics_manager == nullptr);
  p->_physics_manager = this;
  PhysicalsVector::iterator found;
  found = find(_physicals.begin(), _physicals.end(), p);
  if (found == _physicals.end()) {
    _physicals.push_back(p);
  }
}

/**
 * Adds a global linear force to the physics manager
 */
INLINE void PhysicsManager::
add_linear_force(LinearForce *f) {
  nassertv(f);
  LinearForceVector::iterator found;
  PT(LinearForce) ptlf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), ptlf);
  if (found == _linear_forces.end()) {
    _linear_forces.push_back(f);
  }
}

/**
 * Please call attach_physical_node instead.
 */
INLINE void PhysicsManager::
attach_physicalnode(PhysicalNode *p) {
  std::cerr<<"attach_physicalnode (aka attachPhysicalnode) has been"
    <<"replaced with attach_physical_node (aka attachPhysicalNode)."
    <<"  Please change the spelling of the function in your code."
    <<std::endl;
  attach_physical_node(p);
}

/**
 * Registers a physicalnode with the manager
 */
INLINE void PhysicsManager::
attach_physical_node(PhysicalNode *p) {
  nassertv(p);
  for (size_t i = 0; i < p->get_num_physicals(); ++i) {
    attach_physical(p->get_physical(i));
  }
}

/**
 * Resets the physics manager force vector
 */
INLINE void PhysicsManager::
clear_linear_forces() {
  _linear_forces.erase(_linear_forces.begin(), _linear_forces.end());
}

/**
 * Adds a global angular force to the physics manager
 */
INLINE void PhysicsManager::
add_angular_force(AngularForce *f) {
  nassertv(f);
  AngularForceVector::iterator found;
  PT(AngularForce) ptaf = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), ptaf);
  if (found == _angular_forces.end())
    _angular_forces.push_back(f);
}

/**
 * Resets the physics manager force vector
 */
INLINE void PhysicsManager::
clear_angular_forces() {
  _angular_forces.erase(_angular_forces.begin(), _angular_forces.end());
}

/**
 * Resets the physics manager objects vector
 */
INLINE void PhysicsManager::
clear_physicals() {
  _physicals.erase(_physicals.begin(), _physicals.end());
}

/**
 * Set the global viscosity.
 */
INLINE void PhysicsManager::
set_viscosity(PN_stdfloat viscosity) {
  _viscosity=viscosity;
}

/**
 * Get the global viscosity.
 */
INLINE PN_stdfloat PhysicsManager::
get_viscosity() const {
  return _viscosity;
}

/**
 * Hooks a linear integrator into the manager
 */
INLINE void PhysicsManager::
attach_linear_integrator(LinearIntegrator *i) {
  nassertv(i);
  _linear_integrator = i;
}

/**
 * Hooks an angular integrator into the manager
 */
INLINE void PhysicsManager::
attach_angular_integrator(AngularIntegrator *i) {
  nassertv(i);
  _angular_integrator = i;
}
