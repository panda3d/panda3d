/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsManager.cxx
 * @author charles
 * @date 2000-06-14
 */

#include "physicsManager.h"
#include "actorNode.h"

#include <algorithm>
#include "pvector.h"

using std::ostream;

ConfigVariableInt PhysicsManager::_random_seed
("physics_manager_random_seed", 139);

/**
 * Default Constructor.  NOTE: EulerIntegrator is the standard default.
 */
PhysicsManager::
PhysicsManager() {
  _linear_integrator.clear();
  _angular_integrator.clear();
  _viscosity=0.0;
}

/**
 * Simple Destructor
 */
PhysicsManager::
~PhysicsManager() {
  PhysicalsVector::iterator pi;
  for (pi = _physicals.begin(); pi != _physicals.end(); ++pi) {
    nassertv((*pi)->_physics_manager == this);
    (*pi)->_physics_manager = nullptr;
  }
}

/**
 * One-time config function, sets up the random seed used by the physics and
 * particle systems.  For synchronizing across distributed computers
 */
void PhysicsManager::
init_random_seed() {
  // Use the random seed specified by the physics_manager_random_seed Config
  // Variable
  srand(_random_seed);
}

/**
 * takes a linear force out of the physics list
 */
void PhysicsManager::
remove_linear_force(LinearForce *f) {
  nassertv(f);
  LinearForceVector::iterator found;

  PT(LinearForce) ptbf = f;
  found = find(_linear_forces.begin(), _linear_forces.end(), ptbf);

  if (found == _linear_forces.end()) {
    return;
  }
  _linear_forces.erase(found);
}

/**
 * takes an angular force out of the physics list
 */
void PhysicsManager::
remove_angular_force(AngularForce *f) {
  nassertv(f);
  AngularForceVector::iterator found;

  PT(BaseForce) ptbf = f;
  found = find(_angular_forces.begin(), _angular_forces.end(), ptbf);

  if (found == _angular_forces.end()) {
    return;
  }
  _angular_forces.erase(found);
}

/**
 * takes a physical out of the object list
 */
void PhysicsManager::
remove_physical(Physical *p) {
  nassertv(p);
  pvector< Physical * >::iterator found;

  found = find(_physicals.begin(), _physicals.end(), p);
  if (found == _physicals.end()) {
    return;
  }
  nassertv(p->_physics_manager == this);
  p->_physics_manager = nullptr;
  _physicals.erase(found);
}

/**
 * Removes a physicalnode from the manager
 */
void PhysicsManager::
remove_physical_node(PhysicalNode *p) {
  nassertv(p);
  for (size_t i = 0; i < p->get_num_physicals(); ++i) {
    remove_physical(p->get_physical(i));
  }
}

/**
 * This is the main high-level API call.  Performs integration on every
 * attached Physical.
 */
void PhysicsManager::
do_physics(PN_stdfloat dt) {
  // now, run through each physics object in the set.
  PhysicalsVector::iterator p_cur = _physicals.begin();
  for (; p_cur != _physicals.end(); ++p_cur) {
    Physical *physical = *p_cur;
    nassertv(physical);

    // do linear if (_linear_integrator.is_null() == false) {
    if (_linear_integrator) {
      _linear_integrator->integrate(physical, _linear_forces, dt);
    }

    // do angular if (_angular_integrator.is_null() == false) {
    if (_angular_integrator) {
      _angular_integrator->integrate(physical, _angular_forces, dt);
    }

    // if it's an actor node, tell it to update itself.
    PhysicalNode *pn = physical->get_physical_node();
    if (pn && pn->is_of_type(ActorNode::get_class_type())) {
      ActorNode *an = (ActorNode *) pn;
      an->update_transform();
    }
  }
}

/**
 * This is the main high-level API call.  Performs integration on a single
 * physical.  Make sure its associated forces are active.
 */
void PhysicsManager::
do_physics(PN_stdfloat dt, Physical *physical) {
  nassertv(physical);

  // do linear if (_linear_integrator.is_null() == false) {
  if (_linear_integrator) {
    _linear_integrator->integrate(physical, _linear_forces, dt);
  }

  // do angular if (_angular_integrator.is_null() == false) {
  if (_angular_integrator) {
    _angular_integrator->integrate(physical, _angular_forces, dt);
  }

  // if it's an actor node, tell it to update itself.
  PhysicalNode *pn = physical->get_physical_node();
  if (pn && pn->is_of_type(ActorNode::get_class_type())) {
    ActorNode *an = (ActorNode *) pn;
    an->update_transform();
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"PhysicsManager";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
write_physicals(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  if (indent>10) {
    return;
  }
  out.width(indent);
  out<<""<<"_physicals ("<<_physicals.size()<<" physicals)\n";
  // out<<ios::width(indent)<<" "<<"[physicals \n";
  for (pvector< Physical * >::const_iterator i=_physicals.begin();
       i != _physicals.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
write_linear_forces(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_linear_forces ("<<_linear_forces.size()<<" forces)\n";
  for (LinearForceVector::const_iterator i=_linear_forces.begin();
       i != _linear_forces.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
write_angular_forces(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_angular_forces ("<<_angular_forces.size()<<" forces)\n";
  for (AngularForceVector::const_iterator i=_angular_forces.begin();
       i != _angular_forces.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"PhysicsManager:\n";
  if (indent>20) {
    // ...indent limit is arbitrary, it limits recursion.
    out.width(indent+2); out<<""<<"...\n";
    return;
  }
  write_physicals(out, indent+2);
  write_linear_forces(out, indent+2);
  write_angular_forces(out, indent+2);
  out.width(indent+2); out<<""<<"_linear_integrator:\n";
  if (_linear_integrator) {
    _linear_integrator->write(out, indent+4);
  } else {
    out.width(indent+4); out<<""<<"null\n";
  }
  out.width(indent+2); out<<""<<"_angular_integrator:\n";
  if (_angular_integrator) {
    _angular_integrator->write(out, indent+4);
  } else {
    out.width(indent+4); out<<""<<"null\n";
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsManager::
debug_output(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"PhysicsManager li"<<(_linear_integrator?1:0)<<" ai"<<(_angular_integrator?1:0)<<"\n";
  out<<"  _physicals "<<_physicals.size()<<"\n";
  // _physicals._phys_body.write(out, indent+2);


  out.width(indent+2);
  out<<""<<"_linear_forces ("<<_linear_forces.size()<<" forces)\n";
  LinearForceVector::const_iterator li;
  for (li=_linear_forces.begin();
       li != _linear_forces.end();
       ++li) {
    (*li)->write(out, indent+2);
  }

  out.width(indent+2);
  out<<""<<"  _angular_forces "<<_angular_forces.size()<<"\n";
  AngularForceVector::const_iterator ai;
  for (ai=_angular_forces.begin();
       ai != _angular_forces.end();
       ++ai) {
    (*ai)->write(out, indent+2);
  }
  #endif //] NDEBUG
}
