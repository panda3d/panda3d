// Filename: linearDistanceForce.h
// Created by:  charles (21Jun00)
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

#ifndef LINEARDISTANCEFORCE_H
#define LINEARDISTANCEFORCE_H

#include "linearForce.h"

class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : LinearDistanceForce
// Description : Pure virtual class for sinks and sources
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearDistanceForce : public LinearForce {
PUBLISHED:
  enum FalloffType {
    FT_ONE_OVER_R,
    FT_ONE_OVER_R_SQUARED,
    FT_ONE_OVER_R_CUBED
  };

  INLINE void set_radius(float r);
  INLINE void set_falloff_type(FalloffType ft);
  INLINE void set_force_center(const LPoint3f& p);

  INLINE float get_radius() const;
  INLINE FalloffType get_falloff_type() const;
  INLINE LPoint3f get_force_center() const;

  INLINE float get_scalar_term() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  LPoint3f _force_center;

  FalloffType _falloff;
  float _radius;

  virtual LinearForce *make_copy() = 0;
  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

protected:
  LinearDistanceForce(const LPoint3f& p, FalloffType ft, float r, float a,
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
