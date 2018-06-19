/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearControlForce.h
 * @author Dave Schuyler
 * @date 2006
 */

#ifndef LINEARCONTROLFORCE_H
#define LINEARCONTROLFORCE_H

#include "linearForce.h"

/**
 * Simple directed vector force.  This force is different from the others in
 * that it can be global and still only affect a single object.  That might
 * not make sense for a physics simulation, but it's very handy for a game.
 * I.e.  this is the force applied by user on the selected object.
 */
class EXPCL_PANDA_PHYSICS LinearControlForce : public LinearForce {
PUBLISHED:
  explicit LinearControlForce(const PhysicsObject *po = 0, PN_stdfloat a = 1.0f,
                              bool mass = false);
  LinearControlForce(const LinearControlForce &copy);
  virtual ~LinearControlForce();

  INLINE void clear_physics_object();
  INLINE void set_physics_object(const PhysicsObject *po);
  INLINE CPT(PhysicsObject) get_physics_object() const;

  INLINE void set_vector(const LVector3& v);
  INLINE void set_vector(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);

  INLINE LVector3 get_local_vector() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  CPT(PhysicsObject) _physics_object;
  LVector3 _fvec;

  virtual LinearForce *make_copy();
  virtual LVector3 get_child_vector(const PhysicsObject *po);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearControlForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearControlForce.I"

#endif // LINEARCONTROLFORCE_H
