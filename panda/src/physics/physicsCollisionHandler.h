// Filename: physicsCollisionHandler.h
// Created by:  drose (16Mar02)
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

protected:
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



