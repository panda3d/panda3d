/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsCollisionHandler.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef PHYSICSCOLLISIONHANDLER_H
#define PHYSICSCOLLISIONHANDLER_H

#include "pandabase.h"

#include "collisionHandlerPusher.h"

/**
 * A specialized kind of CollisionHandler that simply pushes back on things
 * that attempt to move into solid walls.  This also puts forces onto the
 * physics objects
 */
class EXPCL_PANDA_PHYSICS PhysicsCollisionHandler :
    public CollisionHandlerPusher {
PUBLISHED:
  PhysicsCollisionHandler();
  virtual ~PhysicsCollisionHandler();

  // These setters and getter are a bit of a hack:
  INLINE void set_almost_stationary_speed(PN_stdfloat speed);
  INLINE PN_stdfloat get_almost_stationary_speed();

  INLINE void set_static_friction_coef(PN_stdfloat coef);
  INLINE PN_stdfloat get_static_friction_coef();

  INLINE void set_dynamic_friction_coef(PN_stdfloat coef);
  INLINE PN_stdfloat get_dynamic_friction_coef();

protected:
  PN_stdfloat _almost_stationary_speed;
  PN_stdfloat _static_friction_coef;
  PN_stdfloat _dynamic_friction_coef;

  void apply_friction(
      ColliderDef &def, LVector3 &vel, const LVector3& force, PN_stdfloat angle);
  virtual void apply_net_shove(
      ColliderDef &def, const LVector3 &net_shove,
      const LVector3 &force_normal);
  virtual void apply_linear_force(ColliderDef &def, const LVector3 &force);

  virtual bool validate_target(const NodePath &target);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPusher::init_type();
    register_type(_type_handle, "PhysicsCollisionHandler",
                  CollisionHandlerPusher::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physicsCollisionHandler.I"

#endif
