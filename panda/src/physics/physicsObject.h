/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsObject.h
 * @author charles
 * @date 2000-06-13
 */

#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"
#include "configVariableDouble.h"

/**
 * A body on which physics will be applied.  If you're looking to add physical
 * motion to your class, do NOT derive from this.  Derive from Physical
 * instead.
 */
class EXPCL_PANDA_PHYSICS PhysicsObject : public TypedReferenceCount {
public:
  typedef pvector<PT(PhysicsObject)> Vector;

PUBLISHED:
  PhysicsObject();
  PhysicsObject(const PhysicsObject &copy);
  virtual ~PhysicsObject();
  const PhysicsObject &operator =(const PhysicsObject &other);

  static ConfigVariableDouble _default_terminal_velocity;

  INLINE void set_active(bool flag);
  INLINE bool get_active() const;

  INLINE void set_mass(PN_stdfloat);
  INLINE PN_stdfloat get_mass() const;

  // INLINE void set_center_of_mass(const LPoint3 &pos); use set_position.
  INLINE void set_position(const LPoint3 &pos);
  INLINE void set_position(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE LPoint3 get_position() const;

  INLINE void reset_position(const LPoint3 &pos);

  INLINE void set_last_position(const LPoint3 &pos);
  INLINE LPoint3 get_last_position() const;

  INLINE void set_velocity(const LVector3 &vel);
  INLINE void set_velocity(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE LVector3 get_velocity() const;
  INLINE LVector3 get_implicit_velocity() const;

  // Global instantanious forces
  INLINE void add_torque(const LRotation &torque);
  INLINE void add_impulse(const LVector3 &impulse);
  virtual void add_impact(
      const LPoint3 &offset_from_center_of_mass, const LVector3 &impulse);

  // Local instantanious forces
  INLINE void add_local_torque(const LRotation &torque);
  INLINE void add_local_impulse(const LVector3 &impulse);
  virtual void add_local_impact(
      const LPoint3 &offset_from_center_of_mass, const LVector3 &impulse);

  INLINE void set_terminal_velocity(PN_stdfloat tv);
  INLINE PN_stdfloat get_terminal_velocity() const;

  INLINE void set_oriented(bool flag);
  INLINE bool get_oriented() const;

  INLINE void set_orientation(const LOrientation &orientation);
  INLINE LOrientation get_orientation() const;

  INLINE void reset_orientation(const LOrientation &orientation);

  INLINE void set_rotation(const LRotation &rotation);
  INLINE LRotation get_rotation() const;

  virtual LMatrix4 get_inertial_tensor() const;
  virtual LMatrix4 get_lcs() const;
  virtual PhysicsObject *make_copy() const;

#if !defined(NDEBUG) || !defined(CPPPARSER)
  void set_name(const std::string &name) {
    _name = name;
  }
  const std::string &get_name() {
    return _name;
  }
#endif

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  // physical
  LPoint3 _position; // aka _center_of_mass
  LPoint3 _last_position;
  LVector3 _velocity; // aka _linear_velocity

  // angular
  LOrientation _orientation;
  LRotation _rotation; // aka _angular_velocity

  PN_stdfloat _terminal_velocity;
  PN_stdfloat _mass;

  bool _process_me;
  bool _oriented;

  std::string _name;

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
