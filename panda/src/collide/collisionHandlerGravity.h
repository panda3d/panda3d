// Filename: CollisionHandlerGravity.h
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CollisionHandlerGravity_H
#define CollisionHandlerGravity_H

#include "pandabase.h"

#include "collisionHandlerPhysical.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionHandlerGravity
// Description : A specialized kind of CollisionHandler that sets the
//               Z height of the collider to a fixed linear offset
//               from the highest detected collision point each frame.
//               It's intended to implement walking around on a floor
//               of varying height by casting a ray down from the
//               avatar's head.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionHandlerGravity : public CollisionHandlerPhysical {
PUBLISHED:
  CollisionHandlerGravity();
  virtual ~CollisionHandlerGravity();

  INLINE void set_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_offset() const;

  INLINE void set_reach(PN_stdfloat reach);
  INLINE PN_stdfloat get_reach() const;

  INLINE PN_stdfloat get_airborne_height() const;
  INLINE bool is_on_ground() const;
  INLINE PN_stdfloat get_impact_velocity() const;
  INLINE const LVector3 &get_contact_normal() const;

  INLINE void add_velocity(PN_stdfloat velocity);
  INLINE void set_velocity(PN_stdfloat velocity);
  INLINE PN_stdfloat get_velocity() const;

  INLINE void set_gravity(PN_stdfloat gravity);
  INLINE PN_stdfloat get_gravity() const;

  INLINE void set_max_velocity(PN_stdfloat max_vel);
  INLINE PN_stdfloat get_max_velocity() const;

  INLINE void set_legacy_mode(bool legacy_mode);
  INLINE bool get_legacy_mode() const;

protected:
  PN_stdfloat set_highest_collision(const NodePath &target_node_path, const NodePath &from_node_path, const Entries &entries);
  virtual bool handle_entries();
  virtual void apply_linear_force(ColliderDef &def, const LVector3 &force);

private:
  PN_stdfloat _offset;
  PN_stdfloat _reach;
  PN_stdfloat _airborne_height;
  PN_stdfloat _impact_velocity;
  PN_stdfloat _gravity;
  PN_stdfloat _current_velocity;
  PN_stdfloat _max_velocity;
  LVector3 _contact_normal;
  bool _legacy_mode;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPhysical::init_type();
    register_type(_type_handle, "CollisionHandlerGravity",
                  CollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerGravity.I"

#endif



