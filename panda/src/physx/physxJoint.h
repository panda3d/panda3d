/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJoint.h
 * @author enn0x
 * @date 2009-10-02
 */

#ifndef PHYSXJOINT_H
#define PHYSXJOINT_H

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "physxObject.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxActor;
class PhysxScene;

/**
 * Abstract base class for the different types of joints.  All joints are used
 * to connect two dynamic actors, or an actor and the environment.
 */
class EXPCL_PANDAPHYSX PhysxJoint : public PhysxObject, public PhysxEnums {

PUBLISHED:
  void release();

  PhysxActor *get_actor(unsigned int idx) const;
  PhysxScene *get_scene() const;

  void purge_limit_planes();

  void set_name(const char *name);
  void set_global_anchor(const LPoint3f &anchor);
  void set_global_axis(const LVector3f &axis);
  void set_breakable(float maxForce, float maxTorque);
  void set_solver_extrapolation_factor(float factor);
  void set_use_acceleration_spring(bool value);
  void set_limit_point(const LPoint3f &pos, bool isOnActor2=true);
  void add_limit_plane(const LVector3f &normal, const LPoint3f &pointInPlane, float restitution=0.0f);

  const char *get_name() const;
  LPoint3f get_global_anchor() const;
  LVector3f get_global_axis() const;
  float get_solver_extrapolation_factor() const;
  bool get_use_acceleration_spring() const;

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  static PhysxJoint *factory(NxJointType shapeType);

  virtual NxJoint *ptr() const = 0;

  virtual void link(NxJoint *shapePtr) = 0;
  virtual void unlink() = 0;

protected:
  INLINE PhysxJoint();

private:
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxJoint",
                  PhysxObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxJoint.I"

#endif // PHYSXJOINT_H
