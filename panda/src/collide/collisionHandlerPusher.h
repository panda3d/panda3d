/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPusher.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONHANDLERPUSHER_H
#define COLLISIONHANDLERPUSHER_H

#include "pandabase.h"

#include "collisionHandlerPhysical.h"

/**
 * A specialized kind of CollisionHandler that simply pushes back on things
 * that attempt to move into solid walls.  This is the simplest kind of "real-
 * world" collisions you can have.
 */
class EXPCL_PANDA_COLLIDE CollisionHandlerPusher : public CollisionHandlerPhysical {
PUBLISHED:
  CollisionHandlerPusher();
  virtual ~CollisionHandlerPusher();

  INLINE void set_horizontal(bool flag);
  INLINE bool get_horizontal() const;

PUBLISHED:
  MAKE_PROPERTY(horizontal, get_horizontal, set_horizontal);

protected:
  virtual bool handle_entries();
  virtual void apply_net_shove(
      ColliderDef &def, const LVector3 &net_shove,
      const LVector3 &force_normal);
  virtual void apply_linear_force(ColliderDef &def, const LVector3 &force);

  bool _horizontal;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPhysical::init_type();
    register_type(_type_handle, "CollisionHandlerPusher",
                  CollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerPusher.I"

#endif
