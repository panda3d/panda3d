// Filename: physical.I
// Created by:  charles (16Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include <algorithm>

////////////////////////////////////////////////////////////////////
//    Function : clear_linear_forces
//      Access : Public
// Description : Erases the linear force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
clear_linear_forces() {
  _linear_forces.erase(_linear_forces.begin(),
                        _linear_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : clear_angular_forces
//      Access : Public
// Description : Erases the angular force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
clear_angular_forces() {
  _angular_forces.erase(_angular_forces.begin(),
                        _angular_forces.end());
}

////////////////////////////////////////////////////////////////////
//    Function : clear_physics_objects
//      Access : Public
// Description : Erases the object list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
clear_physics_objects() {
  _physics_objects.erase(_physics_objects.begin(),
                         _physics_objects.end());
}

////////////////////////////////////////////////////////////////////
//    Function : add_linear_force
//      Access : Public
// Description : Adds a linear force to the force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
add_linear_force(LinearForce *f) {
  _linear_forces.push_back(f);
}

////////////////////////////////////////////////////////////////////
//    Function : add_angular_force
//      Access : Public
// Description : Adds an angular force to the force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
add_angular_force(AngularForce *f) {
  _angular_forces.push_back(f);
}

////////////////////////////////////////////////////////////////////
//    Function : remove_linear_force
//      Access : Public
// Description : removes a linear force from the force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
remove_linear_force(LinearForce *f) {
  LinearForceVector::iterator found;

  // this is a PT because the templates don't like what should be
  // perfectly allowable, which is to search for bf directly.
  PT(LinearForce) pt_lf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), pt_lf);

  if (found == _linear_forces.end())
    return;

  _linear_forces.erase(found);
}

////////////////////////////////////////////////////////////////////
//    Function : remove_angular_force
//      Access : Public
// Description : removes an angular force from the force list
////////////////////////////////////////////////////////////////////
INLINE void Physical::
remove_angular_force(AngularForce *f) {
  AngularForceVector::iterator found;

  PT(AngularForce) pt_af = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), pt_af);

  if (found == _angular_forces.end())
    return;

  _angular_forces.erase(found);
}

////////////////////////////////////////////////////////////////////
//    Function : add_physics_object
//      Access : Public
// Description : Adds an object to the physics object vector
////////////////////////////////////////////////////////////////////
INLINE void Physical::
add_physics_object(PhysicsObject *po) {
  _physics_objects.push_back(po);
}

////////////////////////////////////////////////////////////////////
//    Function : get_physics_manager
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE PhysicsManager *Physical::
get_physics_manager() const {
  return _physics_manager;
}

////////////////////////////////////////////////////////////////////
//    Function : get_phys_body
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE PhysicsObject *Physical::
get_phys_body() const {
  return _phys_body;
}

////////////////////////////////////////////////////////////////////
//    Function : get_physical_node
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE PhysicalNode *Physical::
get_physical_node() const {
  return _physical_node;
}

////////////////////////////////////////////////////////////////////
//    Function : get_object_vector
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE const PhysicsObject::Vector &Physical::
get_object_vector() const {
  return _physics_objects;
}

////////////////////////////////////////////////////////////////////
//    Function : get_linear_forces
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE const Physical::LinearForceVector &Physical::
get_linear_forces() const {
  return _linear_forces;
}

////////////////////////////////////////////////////////////////////
//    Function : get_angular_forces
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE const Physical::AngularForceVector &Physical::
get_angular_forces() const {
  return _angular_forces;
}

////////////////////////////////////////////////////////////////////
//    Function : get_num_linear_forces
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE int Physical::
get_num_linear_forces() const {
  return _linear_forces.size();
}

////////////////////////////////////////////////////////////////////
//    Function : get_linear_force
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE PT(LinearForce) Physical::
get_linear_force(int index) const {
  nassertr(index >= 0 && index < (int)_linear_forces.size(), NULL);
  return _linear_forces[index];
}

////////////////////////////////////////////////////////////////////
//    Function : get_num_angular_forces
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE int Physical::
get_num_angular_forces() const {
  return _angular_forces.size();
}

////////////////////////////////////////////////////////////////////
//    Function : get_angular_force
//      Access : Public
////////////////////////////////////////////////////////////////////
INLINE PT(AngularForce) Physical::
get_angular_force(int index) const {
  nassertr(index >= 0 && index < (int)_angular_forces.size(), NULL);
  return _angular_forces[index];
}

////////////////////////////////////////////////////////////////////
//    Function : set_viscosity
//      Access : Public
// Description : Set the local viscosity.
////////////////////////////////////////////////////////////////////
INLINE void Physical::
set_viscosity(float viscosity) {
  _viscosity=viscosity;
}

////////////////////////////////////////////////////////////////////
//    Function : get_viscosity
//      Access : Public
// Description : Get the local viscosity.
////////////////////////////////////////////////////////////////////
INLINE float Physical::
get_viscosity() const {
  //zzzzzzzzzzzzzzzz return max(_viscosity, get_physics_manager()->get_viscosity());
  return _viscosity;
}
