/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physical.I
 * @author charles
 * @date 2000-06-16
 */

#include <algorithm>

/**
 * Erases the linear force list
 */
INLINE void Physical::
clear_linear_forces() {
  _linear_forces.erase(_linear_forces.begin(),
                        _linear_forces.end());
}

/**
 * Erases the angular force list
 */
INLINE void Physical::
clear_angular_forces() {
  _angular_forces.erase(_angular_forces.begin(),
                        _angular_forces.end());
}

/**
 * Erases the object list
 */
INLINE void Physical::
clear_physics_objects() {
  _physics_objects.erase(_physics_objects.begin(),
                         _physics_objects.end());
}

/**
 * Adds a linear force to the force list
 */
INLINE void Physical::
add_linear_force(LinearForce *f) {
  _linear_forces.push_back(f);
}

/**
 * Adds an angular force to the force list
 */
INLINE void Physical::
add_angular_force(AngularForce *f) {
  _angular_forces.push_back(f);
}

/**
 * removes a linear force from the force list
 */
INLINE void Physical::
remove_linear_force(LinearForce *f) {
  LinearForceVector::iterator found;

  // this is a PT because the templates don't like what should be perfectly
  // allowable, which is to search for bf directly.
  PT(LinearForce) pt_lf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), pt_lf);

  if (found == _linear_forces.end())
    return;

  _linear_forces.erase(found);
}

/**
 * removes an angular force from the force list
 */
INLINE void Physical::
remove_angular_force(AngularForce *f) {
  AngularForceVector::iterator found;

  PT(AngularForce) pt_af = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), pt_af);

  if (found == _angular_forces.end())
    return;

  _angular_forces.erase(found);
}

/**
 * Adds an object to the physics object vector
 */
INLINE void Physical::
add_physics_object(PhysicsObject *po) {
  _physics_objects.push_back(po);
}

/**

 */
INLINE PhysicsManager *Physical::
get_physics_manager() const {
  return _physics_manager;
}

/**

 */
INLINE PhysicsObject *Physical::
get_phys_body() const {
  return _phys_body;
}

/**

 */
INLINE PhysicalNode *Physical::
get_physical_node() const {
  return _physical_node;
}

/**

 */
INLINE NodePath Physical::
get_physical_node_path() const {
  return NodePath((PandaNode*) _physical_node);
}

/**

 */
INLINE const PhysicsObject::Vector &Physical::
get_object_vector() const {
  return _physics_objects;
}

/**

 */
INLINE const Physical::LinearForceVector &Physical::
get_linear_forces() const {
  return _linear_forces;
}

/**

 */
INLINE const Physical::AngularForceVector &Physical::
get_angular_forces() const {
  return _angular_forces;
}

/**

 */
INLINE int Physical::
get_num_linear_forces() const {
  return _linear_forces.size();
}

/**

 */
INLINE PT(LinearForce) Physical::
get_linear_force(int index) const {
  nassertr(index >= 0 && index < (int)_linear_forces.size(), nullptr);
  return _linear_forces[index];
}

/**

 */
INLINE int Physical::
get_num_angular_forces() const {
  return _angular_forces.size();
}

/**

 */
INLINE PT(AngularForce) Physical::
get_angular_force(int index) const {
  nassertr(index >= 0 && index < (int)_angular_forces.size(), nullptr);
  return _angular_forces[index];
}

/**
 * Set the local viscosity.
 */
INLINE void Physical::
set_viscosity(PN_stdfloat viscosity) {
  _viscosity=viscosity;
}

/**
 * Get the local viscosity.
 */
INLINE PN_stdfloat Physical::
get_viscosity() const {
  // zzzzzzzzzzzzzzzz return max(_viscosity,
  // get_physics_manager()->get_viscosity());
  return _viscosity;
}
