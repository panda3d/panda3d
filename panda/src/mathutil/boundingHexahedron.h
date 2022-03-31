/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingHexahedron.h
 * @author drose
 * @date 1999-10-03
 */

#ifndef BOUNDINGHEXAHEDRON_H
#define BOUNDINGHEXAHEDRON_H

#include "pandabase.h"

#include "finiteBoundingVolume.h"
#include "frustum.h"
#include "plane.h"

#include "coordinateSystem.h"


/**
 * This defines a bounding convex hexahedron.  It is typically used to
 * represent a frustum, but may represent any enclosing convex hexahedron,
 * including simple boxes.  However, if all you want is an axis-aligned
 * bounding box, you may be better off with the simpler BoundingBox class.
 */
class EXPCL_PANDA_MATHUTIL BoundingHexahedron : public FiniteBoundingVolume {
public:
  INLINE_MATHUTIL BoundingHexahedron();

PUBLISHED:
  BoundingHexahedron(const LFrustum &frustum, bool is_ortho,
                     CoordinateSystem cs = CS_default);
  BoundingHexahedron(const LPoint3 &fll, const LPoint3 &flr,
                     const LPoint3 &fur, const LPoint3 &ful,
                     const LPoint3 &nll, const LPoint3 &nlr,
                     const LPoint3 &nur, const LPoint3 &nul);

public:
  ALLOC_DELETED_CHAIN(BoundingHexahedron);
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_min() const;
  virtual LPoint3 get_max() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

PUBLISHED:
  INLINE_MATHUTIL int get_num_points() const;
  INLINE_MATHUTIL LPoint3 get_point(int n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);
  INLINE_MATHUTIL int get_num_planes() const;
  INLINE_MATHUTIL LPlane get_plane(int n) const;
  MAKE_SEQ(get_planes, get_num_planes, get_plane);

  MAKE_SEQ_PROPERTY(points, get_num_points, get_point);
  MAKE_SEQ_PROPERTY(planes, get_num_planes, get_plane);

public:
  virtual const BoundingHexahedron *as_bounding_hexahedron() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;

private:
  void set_planes();
  void set_centroid();

private:
  enum {
    num_points = 8,
    num_planes = 6
  };
  LPoint3 _points[num_points];
  LPlane _planes[num_planes];
  LPoint3 _centroid;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FiniteBoundingVolume::init_type();
    register_type(_type_handle, "BoundingHexahedron",
                  FiniteBoundingVolume::get_class_type());
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

#include "boundingHexahedron.I"

#endif
