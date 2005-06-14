// Filename: physical.h
// Created by:  charles (14Jun00)
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

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "pandabase.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"

#include "pvector.h"
#include "plist.h"

#include "physicsObject.h"
#include "linearForce.h"
#include "angularForce.h"

class PhysicalNode;
class PhysicsManager;

////////////////////////////////////////////////////////////////////
//       Class : Physical
// Description : Defines a set of physically modeled attributes.
//               If you want physics applied to your class, derive
//               it from this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS Physical : public TypedReferenceCount {
public:
  //typedef pvector<PT(PhysicsObject)> PhysicsObjectVector;
  typedef pvector<PT(LinearForce)> LinearForceVector;
  typedef pvector<PT(AngularForce)> AngularForceVector;

PUBLISHED:
  Physical(int total_objects = 1, bool pre_alloc = false);
  Physical(const Physical& copy);

  virtual ~Physical();

  // helpers
  INLINE PhysicsManager *get_physics_manager() const;
  INLINE PhysicalNode *get_physical_node() const;
  INLINE PhysicsObject *get_phys_body() const;

  INLINE void clear_linear_forces();
  INLINE void clear_angular_forces();
  INLINE void clear_physics_objects();
  INLINE void add_linear_force(LinearForce *f);
  INLINE void add_angular_force(AngularForce *f);
  INLINE void add_physics_object(PhysicsObject *po);
  INLINE void remove_linear_force(LinearForce *f);
  INLINE void remove_angular_force(AngularForce *f);

  INLINE int get_num_linear_forces() const;
  INLINE PT(LinearForce) get_linear_force(int index) const;
  INLINE int get_num_angular_forces() const;
  INLINE PT(AngularForce) get_angular_force(int index) const;

  INLINE void set_viscosity(float viscosity);
  INLINE float get_viscosity() const;
  
  virtual void output(ostream &out = cout) const;
  virtual void write_physics_objects(ostream &out = cout, unsigned int indent=0) const;
  virtual void write_linear_forces(ostream &out = cout, unsigned int indent=0) const;
  virtual void write_angular_forces(ostream &out = cout, unsigned int indent=0) const;
  virtual void write(ostream &out = cout, unsigned int indent=0) const;

public:
  INLINE const PhysicsObject::Vector &get_object_vector() const;
  INLINE const LinearForceVector &get_linear_forces() const;
  INLINE const AngularForceVector &get_angular_forces() const;

  friend class PhysicsManager;
  friend class PhysicalNode;

protected:
  float _viscosity;
  // containers
  PhysicsObject::Vector _physics_objects;
  LinearForceVector _linear_forces;
  AngularForceVector _angular_forces;

  // this pointer exists to make life easy.  If a physical exists
  // with only one element (i.e. NOT a particle system or set-physical),
  // then this pointer points at the only physicsobject.  The object
  // is still of course contained in the _physics_objects vector, but
  // this is kind of a quicker way there.
  PhysicsObject *_phys_body;

private:
  PhysicsManager *_physics_manager;
  PhysicalNode *_physical_node;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Physical",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physical.I"

#endif // __PHYSICAL_H__
