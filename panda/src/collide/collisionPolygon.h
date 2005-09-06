// Filename: collisionPolygon.h
// Created by:  drose (25Apr00)
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

#ifndef COLLISIONPOLYGON_H
#define COLLISIONPOLYGON_H

#include "pandabase.h"

#include "collisionPlane.h"
#include "clipPlaneAttrib.h"
#include "look_at.h"

#include "vector_LPoint2f.h"

class GeomNode;

////////////////////////////////////////////////////////////////////
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

private:
  INLINE CollisionPolygon();

public:
  CollisionPolygon(const CollisionPolygon &copy);

  virtual CollisionSolid *make_copy();

  INLINE static bool verify_points(const LPoint3f &a, const LPoint3f &b,
                                   const LPoint3f &c);
  INLINE static bool verify_points(const LPoint3f &a, const LPoint3f &b,
                                   const LPoint3f &c, const LPoint3f &d);
  static bool verify_points(const LPoint3f *begin, const LPoint3f *end);

  bool is_valid() const;
  bool is_concave() const;

  virtual void xform(const LMatrix4f &mat);
  virtual LPoint3f get_collision_origin() const;

  virtual PT(PandaNode) get_viz(const CullTraverser *trav,
                                const CullTraverserData &data,
                                bool bounds_only) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual BoundingVolume *recompute_bound();

  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  class PointDef {
  public:
    INLINE PointDef(const LPoint2f &p, const LVector2f &v);
    INLINE PointDef(float x, float y);
    INLINE PointDef(const PointDef &copy);
    INLINE void operator = (const PointDef &copy);

    LPoint2f _p;  // the point in 2-d space
    LVector2f _v; // the normalized vector to the next point
  };
  typedef pvector<PointDef> Points;

  static void compute_vectors(Points &points);
  void draw_polygon(GeomNode *viz_geom_node, GeomNode *bounds_viz_geom_node,
                    const Points &points) const;

  bool point_is_inside(const LPoint2f &p, const Points &points) const;
  float dist_to_polygon(const LPoint2f &p, const Points &points) const;

  void setup_points(const LPoint3f *begin, const LPoint3f *end);
  INLINE LPoint2f to_2d(const LVecBase3f &point3d) const;
  INLINE void calc_to_3d_mat(LMatrix4f &to_3d_mat) const;
  INLINE void rederive_to_3d_mat(LMatrix4f &to_3d_mat) const;
  INLINE static LPoint3f to_3d(const LVecBase2f &point2d, const LMatrix4f &to_3d_mat);
  LPoint3f legacy_to_3d(const LVecBase2f &point2d, int axis) const;

  bool clip_polygon(Points &new_points, const Points &source_points,
                    const Planef &plane) const;
  bool apply_clip_plane(Points &new_points, const ClipPlaneAttrib *cpa,
                        const TransformState *net_transform) const;

private:
  Points _points;
  LMatrix4f _to_2d_mat;

public:
  static void register_with_read_factory();
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


