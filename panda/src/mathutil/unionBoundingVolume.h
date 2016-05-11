/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file unionBoundingVolume.h
 * @author drose
 * @date 2012-02-08
 */

#ifndef UNIONBOUNDINGVOLUME_H
#define UNIONBOUNDINGVOLUME_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"
#include "pvector.h"

/**
 * This special bounding volume is the union of all of its constituent
 * bounding volumes.
 *
 * A point is defined to be within a UnionBoundingVolume if it is within any
 * one or more of its component bounding volumes.
 */
class EXPCL_PANDA_MATHUTIL UnionBoundingVolume : public GeometricBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL UnionBoundingVolume();
  ALLOC_DELETED_CHAIN(UnionBoundingVolume);

public:
  UnionBoundingVolume(const UnionBoundingVolume &copy);

  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

PUBLISHED:
  INLINE_MATHUTIL int get_num_components() const;
  INLINE_MATHUTIL const GeometricBoundingVolume *get_component(int n) const;
  MAKE_SEQ(get_components, get_num_components, get_component);
  MAKE_SEQ_PROPERTY(components, get_num_components, get_component);

  void clear_components();
  void add_component(const GeometricBoundingVolume *component);

  void filter_intersection(const BoundingVolume *volume);

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual bool extend_by_geometric(const GeometricBoundingVolume *volume);
  virtual bool around_geometric(const BoundingVolume **first,
                                const BoundingVolume **last);

  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  virtual int contains_union(const UnionBoundingVolume *unionv) const;
  virtual int contains_intersection(const IntersectionBoundingVolume *intersection) const;
  virtual int contains_finite(const FiniteBoundingVolume *volume) const;
  virtual int contains_geometric(const GeometricBoundingVolume *volume) const;
  int other_contains_union(const BoundingVolume *other) const;

private:
  typedef pvector<CPT(GeometricBoundingVolume) > Components;
  Components _components;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "UnionBoundingVolume",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingVolume;
};

#include "unionBoundingVolume.I"

#endif
