// Filename: CollisionHandlerGravity.h
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

#ifndef CollisionHandlerGravity_H
#define CollisionHandlerGravity_H

#include "pandabase.h"

#include "collisionHandlerPhysical.h"

///////////////////////////////////////////////////////////////////
//       Class : CollisionHandlerGravity
// Description : A specialized kind of CollisionHandler that sets the
//               Z height of the collider to a fixed linear offset
//               from the highest detected collision point each frame.
//               It's intended to implement walking around on a floor
//               of varying height by casting a ray down from the
//               avatar's head.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandlerGravity : public CollisionHandlerPhysical {
PUBLISHED:
  CollisionHandlerGravity();
  virtual ~CollisionHandlerGravity();

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;

  INLINE void set_reach(float reach);
  INLINE float get_reach() const;

  INLINE float get_airborne_height() const;
  INLINE bool is_on_ground() const;
  INLINE float get_impact_velocity() const;

  INLINE void add_velocity(float velocity);
  INLINE void set_velocity(float velocity);
  INLINE float get_velocity() const;

  INLINE void set_gravity(float gravity);
  INLINE float get_gravity() const;

  INLINE void set_max_velocity(float max_vel);
  INLINE float get_max_velocity() const;

protected:
  float set_highest_collision(const NodePath &target_node_path, const NodePath &from_node_path, const Entries &entries);
  virtual bool handle_entries();
  virtual void apply_linear_force(ColliderDef &def, const LVector3f &force);

private:
  float _offset;
  float _reach;
  float _airborne_height;
  float _impact_velocity;
  float _gravity;
  float _current_velocity;
  float _max_velocity;


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



