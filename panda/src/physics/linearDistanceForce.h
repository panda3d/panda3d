/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearDistanceForce.h
 * @author charles
 * @date 2000-06-21
 */

#ifndef LINEARDISTANCEFORCE_H
#define LINEARDISTANCEFORCE_H

#include "linearForce.h"

class BamReader;

/**
 * Pure virtual class for sinks and sources
 */
class EXPCL_PANDA_PHYSICS LinearDistanceForce : public LinearForce {
PUBLISHED:
  enum FalloffType {
    FT_ONE_OVER_R,
    FT_ONE_OVER_R_SQUARED,
    FT_ONE_OVER_R_CUBED
  };

  INLINE void set_radius(PN_stdfloat r);
  INLINE void set_falloff_type(FalloffType ft);
  INLINE void set_force_center(const LPoint3& p);

  INLINE PN_stdfloat get_radius() const;
  INLINE FalloffType get_falloff_type() const;
  INLINE LPoint3 get_force_center() const;

  INLINE PN_stdfloat get_scalar_term() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  LPoint3 _force_center;

  FalloffType _falloff;
  PN_stdfloat _radius;

  virtual LinearForce *make_copy() = 0;
  virtual LVector3 get_child_vector(const PhysicsObject *po) = 0;

protected:
  LinearDistanceForce(const LPoint3& p, FalloffType ft, PN_stdfloat r, PN_stdfloat a,
                bool m);
  LinearDistanceForce(const LinearDistanceForce &copy);
  virtual ~LinearDistanceForce();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearDistanceForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearDistanceForce.I"

#endif // LINEARDISTANCEFORCE_H
