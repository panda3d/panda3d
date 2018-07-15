/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physical.h
 * @author charles
 * @date 2000-06-14
 */

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "pandabase.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"

#include "pvector.h"
#include "plist.h"

#include "physicsObject.h"
#include "physicsObjectCollection.h"
#include "linearForce.h"
#include "angularForce.h"
#include "nodePath.h"

class PhysicalNode;
class PhysicsManager;

/**
 * Defines a set of physically modeled attributes.  If you want physics
 * applied to your class, derive it from this.
 */
class EXPCL_PANDA_PHYSICS Physical : public TypedReferenceCount {
public:
  // typedef pvector<PT(PhysicsObject)> PhysicsObjectVector;
  typedef pvector<PT(LinearForce)> LinearForceVector;
  typedef pvector<PT(AngularForce)> AngularForceVector;

PUBLISHED:
  explicit Physical(int total_objects = 1, bool pre_alloc = false);
  Physical(const Physical& copy);

  virtual ~Physical();

  // helpers
  INLINE PhysicsManager *get_physics_manager() const;
  INLINE PhysicalNode *get_physical_node() const;
  INLINE NodePath get_physical_node_path() const;
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
  MAKE_SEQ(get_linear_forces, get_num_linear_forces, get_linear_force);
  INLINE int get_num_angular_forces() const;
  INLINE PT(AngularForce) get_angular_force(int index) const;
  MAKE_SEQ(get_angular_forces, get_num_angular_forces, get_angular_force);

  INLINE void set_viscosity(PN_stdfloat viscosity);
  INLINE PN_stdfloat get_viscosity() const;

  const PhysicsObjectCollection get_objects() const;

  virtual void output(std::ostream &out = std::cout) const;
  virtual void write_physics_objects(
    std::ostream &out = std::cout, int indent=0) const;
  virtual void write_linear_forces(
    std::ostream &out = std::cout, int indent=0) const;
  virtual void write_angular_forces(
    std::ostream &out = std::cout, int indent=0) const;
  virtual void write(std::ostream &out = std::cout, int indent=0) const;

public:
  INLINE const PhysicsObject::Vector &get_object_vector() const;
  INLINE const LinearForceVector &get_linear_forces() const;
  INLINE const AngularForceVector &get_angular_forces() const;

  friend class PhysicsManager;
  friend class PhysicalNode;

protected:
  PN_stdfloat _viscosity;
  // containers
  PhysicsObject::Vector _physics_objects;
  LinearForceVector _linear_forces;
  AngularForceVector _angular_forces;

  // this pointer exists to make life easy.  If a physical exists with only
  // one element (i.e.  NOT a particle system or set-physical), then this
  // pointer points at the only PhysicsObject.  The object is still of course
  // contained in the _physics_objects vector, but this is kind of a quicker
  // way there.
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
