// Filename: boundingBox.h
// Created by:  drose (31May07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "pandabase.h"

#include "finiteBoundingVolume.h"
#include "plane.h"

////////////////////////////////////////////////////////////////////
//       Class : BoundingBox
// Description : An axis-aligned bounding box; that is, a minimum and
//               maximum coordinate triple.
//
//               This box is always axis-aligned.  If you need a more
//               general bounding box, try BoundingHexahedron.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MATHUTIL BoundingBox : public FiniteBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingBox();
  INLINE_MATHUTIL BoundingBox(const LPoint3f &min, const LPoint3f &max);
  ALLOC_DELETED_CHAIN(BoundingBox);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_min() const;
  virtual LPoint3f get_max() const;
  virtual float get_volume() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL int get_num_points() const;
  INLINE_MATHUTIL LPoint3f get_point(int n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);
  INLINE_MATHUTIL int get_num_planes() const;
  INLINE_MATHUTIL Planef get_plane(int n) const;
  MAKE_SEQ(get_planes, get_num_planes, get_plane);

public:
  // Inline accessors for speed.
  INLINE_MATHUTIL const LPoint3f &get_minq() const;
  INLINE_MATHUTIL const LPoint3f &get_maxq() const;

public:
  virtual const BoundingBox *as_bounding_box() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3f &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_box(const BoundingBox *box);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);
  bool extend_by_finite(const FiniteBoundingVolume *volume);

  virtual bool around_points(const LPoint3f *first,
                             const LPoint3f *last);
  virtual bool around_spheres(const BoundingVolume **first,
                              const BoundingVolume **last);
  virtual bool around_boxes(const BoundingVolume **first,
                            const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
                                  const BoundingVolume **last);
  bool around_finite(const BoundingVolume **first,
                     const BoundingVolume **last);

  virtual int contains_point(const LPoint3f &point) const;
  virtual int contains_lineseg(const LPoint3f &a, const LPoint3f &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  int contains_finite(const FiniteBoundingVolume *volume) const;

private:
  LPoint3f _min;
  LPoint3f _max;

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
