// Filename: physicsCollisionHandler.h
// Created by:  drose (16Mar02)
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

#ifndef PHYSICSCOLLISIONHANDLER_H
#define PHYSICSCOLLISIONHANDLER_H

#include "pandabase.h"

#include "collisionHandlerPusher.h"

///////////////////////////////////////////////////////////////////
//       Class : PhysicsCollisionHandler
// Description : A specialized kind of CollisionHandler that simply
//               pushes back on things that attempt to move into solid
//               walls.  This also puts forces onto the physics objects
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PhysicsCollisionHandler : public CollisionHandlerPusher {
PUBLISHED:
  PhysicsCollisionHandler();
  virtual ~PhysicsCollisionHandler();
  
  // These setters and getter are a bit of a hack:
  INLINE void set_almost_stationary_speed(float speed);
  INLINE float get_almost_stationary_speed();
  
  INLINE void set_static_friction_coef(float coef);
  INLINE float get_static_friction_coef();
  
  INLINE void set_dynamic_friction_coef(float coef);
  INLINE float get_dynamic_friction_coef();

protected:
  float _almost_stationary_speed;
  float _static_friction_coef;
  float _dynamic_friction_coef;

  void apply_friction(ColliderDef &def, LVector3f &vel, const LVector3f& force, float angle);
  virtual void apply_net_shove(ColliderDef &def, const LVector3f &net_shove, const LVector3f &force_normal);
  virtual void apply_linear_force(ColliderDef &def, const LVector3f &force);

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



