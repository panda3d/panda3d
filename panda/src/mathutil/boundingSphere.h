// Filename: boundingSphere.h
// Created by:  drose (01Oct99)
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

#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include "pandabase.h"

#include "finiteBoundingVolume.h"

///////////////////////////////////////////////////////////////////
//       Class : BoundingSphere
// Description : This defines a bounding sphere, consisting of a
//               center and a radius.  It is always a sphere, and
//               never an ellipsoid or other quadric.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundingSphere : public FiniteBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingSphere();
  INLINE_MATHUTIL BoundingSphere(const LPoint3f &center, float radius);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_min() const;
  virtual LPoint3f get_max() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL const LPoint3f &get_center() const;
  INLINE_MATHUTIL float get_radius() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3f &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);
  bool extend_by_finite(const FiniteBoundingVolume *volume);

  virtual bool around_points(const LPoint3f *first,
                             const LPoint3f *last);
  virtual bool around_spheres(const BoundingVolume **first,
                              const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
                                  const BoundingVolume **last);
  bool around_finite(const BoundingVolume **first,
                     const BoundingVolume **last);

  virtual int contains_point(const LPoint3f &point) const;
  virtual int contains_lineseg(const LPoint3f &a, const LPoint3f &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_line(const BoundingLine *line) const;

private:
  LPoint3f _center;
  float _radius;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FiniteBoundingVolume::init_type();
    register_type(_type_handle, "BoundingSphere",
                  FiniteBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingHexahedron;
};

#include "boundingSphere.I"

#endif
