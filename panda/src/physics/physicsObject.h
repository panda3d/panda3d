// Filename: physicsObject.h
// Created by:  charles (13Jun00)
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

#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <pandabase.h>
#include <typedReferenceCount.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : PhysicsObject
// Description : A body on which physics will be applied.  If you're
//               looking to add physical motion to your class, do
//               NOT derive from this.  Derive from Physical instead.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PhysicsObject : public TypedReferenceCount {
PUBLISHED:
  PhysicsObject();
  PhysicsObject(const PhysicsObject &copy);
  virtual ~PhysicsObject();
  const PhysicsObject &operator =(const PhysicsObject &other);

  static const float _default_terminal_velocity;

  INLINE void set_mass(float);
  INLINE float get_mass() const;

  INLINE void set_position(const LPoint3f &pos);
  INLINE void set_position(float x, float y, float z);
  INLINE LPoint3f get_position() const;

  INLINE void set_position_HandOfGod(const LPoint3f &pos);

  INLINE void set_last_position(const LPoint3f &pos);
  INLINE LPoint3f get_last_position() const;

  INLINE void set_velocity(const LVector3f &vel);
  INLINE void set_velocity(float x, float y, float z);
  INLINE LVector3f get_velocity() const;

  INLINE void set_active(bool flag);
  INLINE bool get_active() const;

  INLINE void set_oriented(bool flag);
  INLINE bool get_oriented() const;

  INLINE void set_terminal_velocity(float tv);
  INLINE float get_terminal_velocity() const;

  INLINE void set_orientation(const LOrientationf &orientation);
  INLINE LOrientationf get_orientation() const;

  INLINE void set_rotation(const LVector3f &rotation);
  INLINE LVector3f get_rotation() const;

  virtual LMatrix4f get_inertial_tensor(void) const;
  virtual LMatrix4f get_lcs() const;
  virtual PhysicsObject *make_copy() const;
  
  void output(ostream &out, unsigned int indent=0) const;

private:
  // physical
  LPoint3f _position;
  LPoint3f _last_position;
  LVector3f _velocity;

  // angular
  LOrientationf _orientation;
  LVector3f _rotation;

  float _terminal_velocity;
  float _mass;

  bool _process_me;
  bool _oriented;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PhysicsObject",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physicsObject.I"

#endif // __PHYSICS_OBJECT_H__
