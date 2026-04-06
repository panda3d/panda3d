/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingLine.h
 * @author drose
 * @date 2000-07-04
 */

#ifndef BOUNDINGLINE_H
#define BOUNDINGLINE_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"

/**
 * This funny bounding volume is an infinite line with no thickness and
 * extending to infinity in both directions.
 *
 * Note that it *always* extends in both directions, despite the fact that you
 * specify two points to the constructor.  These are not endpoints, they are
 * two arbitrary points on the line.
 */
class EXPCL_PANDA_MATHUTIL BoundingLine : public GeometricBoundingVolume {
public:
  INLINE_MATHUTIL BoundingLine();

PUBLISHED:
  INLINE_MATHUTIL explicit BoundingLine(const LPoint3 &a, const LPoint3 &b);
  ALLOC_DELETED_CHAIN(BoundingLine);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL const LPoint3 &get_point_a() const;
  INLINE_MATHUTIL LPoint3 get_point_b() const;

public:
  virtual const BoundingLine *as_bounding_line() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual bool extend_by_line(const BoundingLine *line);

  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;

  PN_stdfloat sqr_dist_to_line(const LPoint3 &point) const;

private:
  LPoint3 _origin;
  LVector3 _vector;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "BoundingLine",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingSphere;
  friend class BoundingBox;
};

#include "boundingLine.I"

#endif
