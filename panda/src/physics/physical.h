// Filename: physical.h
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <pandabase.h>
#include <pointerTo.h>
#include <typedReferenceCount.h>

#include <vector>
#include <list>

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
  vector< PT(PhysicsObject) > _physics_objects;
  vector< PT(LinearForce) > _linear_forces;
  vector< PT(AngularForce) > _angular_forces;

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

  INLINE const vector< PT(PhysicsObject) > &get_object_vector(void) const;
  INLINE const vector< PT(LinearForce) > &get_linear_forces(void) const;
  INLINE const vector< PT(AngularForce) > &get_angular_forces(void) const;

  INLINE void clear_linear_forces(void);
  INLINE void clear_angular_forces(void);
  INLINE void clear_physics_objects(void);
  INLINE void add_linear_force(LinearForce *f);
  INLINE void add_angular_force(AngularForce *f);
  INLINE void add_physics_object(PhysicsObject *po);
  INLINE void remove_linear_force(LinearForce *f);
  INLINE void remove_angular_force(AngularForce *f);

public:
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
