// Filename: physical.cxx
// Created by:  charles (16Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pointerTo.h"

#include "physical.h"
#include "physicsManager.h"

TypeHandle Physical::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : Physical
//       Access : Public
//  Description : Default Constructor
//
//                The idea here is that most physicals will NOT 
//                be collections of sets (i.e. particle systems 
//                and whatever else).  Because of this, the default 
//                constructor, unless otherwise specified, will 
//                automatically allocate and initialize one 
//                PhysicalObject.  This makes it easier for 
//                high-level work.
//
//                pre-alloc is ONLY for multiple-object physicals, 
//                and if true, fills the physics_object vector 
//                with dead nodes, pre-allocating for the speed
//                end of the speed-vs-overhead deal.
////////////////////////////////////////////////////////////////////
Physical::
Physical(int total_objects, bool pre_alloc) {
  _viscosity=0.0;
  _physical_node = (PhysicalNode *) NULL;
  _physics_manager = (PhysicsManager *) NULL;

  if (total_objects == 1) {
    _phys_body = new PhysicsObject;
    add_physics_object(_phys_body);
  } else {
    _phys_body = (PhysicsObject *) NULL;
    // allocate each object.
    if (pre_alloc == true) {
      for (int i = 0; i < total_objects; ++i) {
        PhysicsObject *po = new PhysicsObject;
        add_physics_object(po);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : Physical
//       Access : Public
//  Description : copy constructor (note- does deep copy of pn's)
//                but does NOT attach itself to its template's
//                physicsmanager.
////////////////////////////////////////////////////////////////////
Physical::
Physical(const Physical& copy) {
  _physics_manager = (PhysicsManager *) NULL;

  // copy the forces.
  LinearForceVector::const_iterator lf_cur;
  LinearForceVector::const_iterator lf_end = copy._linear_forces.end();

  for (lf_cur = copy._linear_forces.begin(); lf_cur != lf_end; lf_cur++) {
    _linear_forces.push_back((*lf_cur)->make_copy());
  }

  AngularForceVector::const_iterator af_cur;
  AngularForceVector::const_iterator af_end = copy._angular_forces.end();

  for (af_cur = copy._angular_forces.begin(); af_cur != af_end; af_cur++) {
    _angular_forces.push_back((*af_cur)->make_copy());
  }

  // copy the physics objects
  PhysicsObject::Vector::const_iterator p_cur;
  PhysicsObject::Vector::const_iterator p_end = copy._physics_objects.end();

  for (p_cur = copy._physics_objects.begin(); p_cur != p_end; p_cur++) {
    // oooh so polymorphic.
    _physics_objects.push_back((*p_cur)->make_copy());
  }

  // now set the one-element-quick-access pointer
  if (_physics_objects.size() == 1)
    _phys_body = _physics_objects[0];
  else
    _phys_body = (PhysicsObject *) NULL;
}

////////////////////////////////////////////////////////////////////
//     Function : ~Physical
//       Access : Public
//  Description : destructor
////////////////////////////////////////////////////////////////////
Physical::
~Physical() {
  // note that this removes a physical from a physics manager.
  // this is safe because the physics manager doesn't keep PT's to
  // physicals, simply *'s, and also means that we don't have to tell
  // the physics manager ourselves when one of our physicals is dead.
  if (_physics_manager != (PhysicsManager *) NULL) {
    _physics_manager->remove_physical(this);
  }
}

////////////////////////////////////////////////////////////////////
//    Function : get_objects
//      Access : Public
////////////////////////////////////////////////////////////////////
const PhysicsObjectCollection Physical::
get_objects() const{
  PhysicsObjectCollection poc;

  for (PhysicsObject::Vector::const_iterator i=_physics_objects.begin();
       i != _physics_objects.end();
       ++i) {
    poc.add_physics_object((PhysicsObject*)(*i));
  }

  return poc;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void Physical::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"Physical";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write_physics_objects
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void Physical::
write_physics_objects(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"_physics_objects ("<<_physics_objects.size()<<" objects)\n";
  for (PhysicsObject::Vector::const_iterator i=_physics_objects.begin();
       i != _physics_objects.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write_linear_forces
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void Physical::
write_linear_forces(ostream &out, unsigned int indent) const {
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

////////////////////////////////////////////////////////////////////
//     Function : write_angular_forces
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void Physical::
write_angular_forces(ostream &out, unsigned int indent) const {
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

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void Physical::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"Physical\n";
  write_physics_objects(out, indent+2);
  write_linear_forces(out, indent+2);
  write_angular_forces(out, indent+2);
  if (_phys_body) {
    out.width(indent+2); out<<""<<"_phys_body\n";
    _phys_body->write(out, indent+4);
  } else {
    out.width(indent+2); out<<""<<"_phys_body is null\n";
  }
  #endif //] NDEBUG
}
