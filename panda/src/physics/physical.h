// Filename: physical.h
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

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <pandabase.h>
#include <pointerTo.h>
#include <typedReferenceCount.h>

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
private:
  PhysicsManager *_physics_manager;
  PhysicalNode *_physical_node;

protected:
  // containers
  pvector< PT(PhysicsObject) > _physics_objects;
  pvector< PT(LinearForce) > _linear_forces;
  pvector< PT(AngularForce) > _angular_forces;

  // this pointer exists to make life easy.  If a physical exists
  // with only one element (i.e. NOT a particle system or set-physical),
  // then this pointer points at the only physicsobject.  The object
  // is still of course contained in the _physics_objects vector, but
  // this is kind of a quicker way there.
  PhysicsObject *_phys_body;

PUBLISHED:
  Physical(int ttl_objects = 1, bool pre_alloc = false);
  Physical(const Physical& copy);

  virtual ~Physical(void);

  // helpers
  INLINE PhysicsManager *get_physics_manager(void) const;
  INLINE PhysicalNode *get_physical_node(void) const;
  INLINE PhysicsObject *get_phys_body(void) const;

  INLINE void clear_linear_forces(void);
  INLINE void clear_angular_forces(void);
  INLINE void clear_physics_objects(void);
  INLINE void add_linear_force(LinearForce *f);
  INLINE void add_angular_force(AngularForce *f);
  INLINE void add_physics_object(PhysicsObject *po);
  INLINE void remove_linear_force(LinearForce *f);
  INLINE void remove_angular_force(AngularForce *f);

  INLINE int get_num_linear_forces(void) const;
  INLINE PT(LinearForce) get_linear_force(int index) const;
  INLINE int get_num_angular_forces(void) const;
  INLINE PT(AngularForce) get_angular_force(int index) const;

public:
  INLINE const pvector< PT(PhysicsObject) > &get_object_vector(void) const;
  INLINE const pvector< PT(LinearForce) > &get_linear_forces(void) const;
  INLINE const pvector< PT(AngularForce) > &get_angular_forces(void) const;

  friend class PhysicsManager;
  friend class PhysicalNode;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Physical",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physical.I"

#endif // __PHYSICAL_H__
