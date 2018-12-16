/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingSphere.h
 * @author drose
 * @date 1999-10-01
 */

#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include "pandabase.h"

#include "finiteBoundingVolume.h"

/**
 * This defines a bounding sphere, consisting of a center and a radius.  It is
 * always a sphere, and never an ellipsoid or other quadric.
 */
class EXPCL_PANDA_MATHUTIL BoundingSphere : public FiniteBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingSphere();
  INLINE_MATHUTIL explicit BoundingSphere(const LPoint3 &center, PN_stdfloat radius);
  ALLOC_DELETED_CHAIN(BoundingSphere);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_min() const;
  virtual LPoint3 get_max() const;
  virtual PN_stdfloat get_volume() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL LPoint3 get_center() const;
  INLINE_MATHUTIL PN_stdfloat get_radius() const;

  INLINE_MATHUTIL void set_center(const LPoint3 &center);
  INLINE_MATHUTIL void set_radius(PN_stdfloat radius);

  MAKE_PROPERTY(center, get_center, set_center);
  MAKE_PROPERTY(radius, get_radius, set_radius);

public:
  virtual const BoundingSphere *as_bounding_sphere() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3 &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_box(const BoundingBox *box);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);
  virtual bool extend_by_finite(const FiniteBoundingVolume *volume);

  virtual bool around_points(const LPoint3 *first,
                             const LPoint3 *last);
  virtual bool around_finite(const BoundingVolume **first,
                             const BoundingVolume **last);

  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;

private:
  LPoint3 _center;
  PN_stdfloat _radius;


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
