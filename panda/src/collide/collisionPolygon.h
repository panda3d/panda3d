// Filename: collisionPolygon.h
// Created by:  drose (25Apr00)
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

#ifndef COLLISIONPOLYGON_H
#define COLLISIONPOLYGON_H

#include <pandabase.h>

#include "collisionPlane.h"

#include <vector_LPoint2f.h>

///////////////////////////////////////////////////////////////////
//       Class : CollisionPolygon
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionPolygon : public CollisionPlane {
PUBLISHED:
  INLINE CollisionPolygon(const LPoint3f &a, const LPoint3f &b,
                          const LPoint3f &c);
  INLINE CollisionPolygon(const LPoint3f &a, const LPoint3f &b,
                          const LPoint3f &c, const LPoint3f &d);
  INLINE CollisionPolygon(const LPoint3f *begin, const LPoint3f *end);

public:
  CollisionPolygon(const CollisionPolygon &copy);

  virtual CollisionSolid *make_copy();

  INLINE static bool verify_points(const LPoint3f &a, const LPoint3f &b,
                                   const LPoint3f &c);
  INLINE static bool verify_points(const LPoint3f &a, const LPoint3f &b,
                                   const LPoint3f &c, const LPoint3f &d);
  static bool verify_points(const LPoint3f *begin, const LPoint3f *end);


  virtual void xform(const LMatrix4f &mat);
  virtual LPoint3f get_collision_origin() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  INLINE CollisionPolygon(void);
  virtual BoundingVolume *recompute_bound();

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  bool is_inside(const LPoint2f &p) const;
  bool is_concave() const;

  void setup_points(const LPoint3f *begin, const LPoint3f *end);
  LPoint2f to_2d(const LPoint3f &point3d) const;
  LPoint3f to_3d(const LPoint2f &point2d) const;

private:
  typedef vector_LPoint2f Points;
  Points _points;
  LPoint2f _median;

  enum AxisType {
    AT_x, AT_y, AT_z
  };
  AxisType _axis;
  bool _reversed;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_CollisionPolygon(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionPlane::init_type();
    register_type(_type_handle, "CollisionPolygon",
                  CollisionPlane::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionPolygon.I"

#endif


