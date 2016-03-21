/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerFloor.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONHANDLERFLOOR_H
#define COLLISIONHANDLERFLOOR_H

#include "pandabase.h"

#include "collisionHandlerPhysical.h"

/**
 * A specialized kind of CollisionHandler that sets the Z height of the
 * collider to a fixed linear offset from the highest detected collision point
 * each frame.  It's intended to implement walking around on a floor of
 * varying height by casting a ray down from the avatar's head.
 */
class EXPCL_PANDA_COLLIDE CollisionHandlerFloor : public CollisionHandlerPhysical {
PUBLISHED:
  CollisionHandlerFloor();
  virtual ~CollisionHandlerFloor();

  INLINE void set_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_offset() const;

  INLINE void set_reach(PN_stdfloat reach);
  INLINE PN_stdfloat get_reach() const;

  INLINE void set_max_velocity(PN_stdfloat max_vel);
  INLINE PN_stdfloat get_max_velocity() const;

PUBLISHED:
  MAKE_PROPERTY(offset, get_offset, set_offset);
  MAKE_PROPERTY(reach, get_reach, set_reach);
  MAKE_PROPERTY(max_velocity, get_max_velocity, set_max_velocity);

protected:
  PN_stdfloat set_highest_collision(const NodePath &target_node_path, const NodePath &from_node_path, const Entries &entries);
  virtual bool handle_entries();
  virtual void apply_linear_force(ColliderDef &def, const LVector3 &force);

private:
  PN_stdfloat _offset;
  PN_stdfloat _reach;
  PN_stdfloat _max_velocity;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPhysical::init_type();
    register_type(_type_handle, "CollisionHandlerFloor",
                  CollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerFloor.I"

#endif
