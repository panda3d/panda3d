// Filename: linearCylinderVortexForce.h
// Created by:  charles (24Jul00)
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

#ifndef LINEARCYLINDERVORTEXFORCE_H
#define LINEARCYLINDERVORTEXFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearCylinderVortexForce
// Description : Defines a cylinder inside of which all forces are
//               tangential to the theta of the particle wrt the
//               z-axis in local coord. space.  This happens by
//               assigning the force a node by which the cylinder is
//               transformed.  Be warned- this will suck anything
//               that it can reach directly into orbit and will NOT
//               let go.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearCylinderVortexForce : public LinearForce {
PUBLISHED:
  LinearCylinderVortexForce(float radius = 1.0f,
                      float length = 0.0f,
                      float coef = 1.0f,
                      float a = 1.0f,
                      bool md = false);
  LinearCylinderVortexForce(const LinearCylinderVortexForce &copy);
  virtual ~LinearCylinderVortexForce();

  INLINE void set_coef(float coef);
  INLINE float get_coef() const;

  INLINE void set_radius(float radius);
  INLINE float get_radius() const;

  INLINE void set_length(float length);
  INLINE float get_length() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  float _radius;
  float _length;
  float _coef;

  virtual LinearForce *make_copy();
  virtual LVector3f get_child_vector(const PhysicsObject *po);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearCylinderVortexForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearCylinderVortexForce.I"

#endif // LINEARCYLINDERVORTEXFORCE_H
