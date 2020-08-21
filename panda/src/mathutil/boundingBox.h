/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingBox.h
 * @author drose
 * @date 2007-05-31
 */

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "pandabase.h"

#include "finiteBoundingVolume.h"
#include "plane.h"

/**
 * An axis-aligned bounding box; that is, a minimum and maximum coordinate
 * triple.
 *
 * This box is always axis-aligned.  If you need a more general bounding box,
 * try BoundingHexahedron.
 */
class EXPCL_PANDA_MATHUTIL BoundingBox : public FiniteBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingBox();
  INLINE_MATHUTIL explicit BoundingBox(const LPoint3 &min, const LPoint3 &max);
  ALLOC_DELETED_CHAIN(BoundingBox);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_min() const;
  virtual LPoint3 get_max() const;
  virtual PN_stdfloat get_volume() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;


PUBLISHED:
  INLINE_MATHUTIL int get_num_points() const;
  INLINE_MATHUTIL LPoint3 get_point(int n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);
  INLINE_MATHUTIL int get_num_planes() const;
  INLINE_MATHUTIL LPlane get_plane(int n) const;
  MAKE_SEQ(get_planes, get_num_planes, get_plane);

  MAKE_SEQ_PROPERTY(points, get_num_points, get_point);
  MAKE_SEQ_PROPERTY(planes, get_num_planes, get_plane);

  INLINE_MATHUTIL void set_min_max(const LPoint3 &min, const LPoint3 &max);

public:
  // Inline accessors for speed.
  INLINE_MATHUTIL const LPoint3 &get_minq() const;
  INLINE_MATHUTIL const LPoint3 &get_maxq() const;

public:
  virtual const BoundingBox *as_bounding_box() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3 &point);
  virtual bool extend_by_box(const BoundingBox *box);
  virtual bool extend_by_finite(const FiniteBoundingVolume *volume);

  virtual bool around_points(const LPoint3 *first,
                             const LPoint3 *last);
  virtual bool around_finite(const BoundingVolume **first,
                             const BoundingVolume **last);

  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  virtual int contains_finite(const FiniteBoundingVolume *volume) const;

private:
  LPoint3 _min;
  LPoint3 _max;

  static const int plane_def[6][3];

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FiniteBoundingVolume::init_type();
    register_type(_type_handle, "BoundingBox",
                  FiniteBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingSphere;
};

#include "boundingBox.I"

#endif
