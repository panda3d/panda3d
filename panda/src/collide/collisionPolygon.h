/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon.h
 * @author drose
 * @date 2000-04-25
 */

#ifndef COLLISIONPOLYGON_H
#define COLLISIONPOLYGON_H

#include "pandabase.h"

#include "collisionPlane.h"
#include "clipPlaneAttrib.h"
#include "look_at.h"
#include "pvector.h"

class GeomNode;

/**
 *
 */
class EXPCL_PANDA_COLLIDE CollisionPolygon : public CollisionPlane {
PUBLISHED:
  INLINE CollisionPolygon(const LVecBase3 &a, const LVecBase3 &b,
                          const LVecBase3 &c);
  INLINE CollisionPolygon(const LVecBase3 &a, const LVecBase3 &b,
                          const LVecBase3 &c, const LVecBase3 &d);
  INLINE CollisionPolygon(const LPoint3 *begin, const LPoint3 *end);

private:
  INLINE CollisionPolygon();

public:
  CollisionPolygon(const CollisionPolygon &copy);

  virtual CollisionSolid *make_copy();

PUBLISHED:
  virtual LPoint3 get_collision_origin() const;

  INLINE size_t get_num_points() const;
  INLINE LPoint3 get_point(size_t n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);


  INLINE static bool verify_points(const LPoint3 &a, const LPoint3 &b,
                                   const LPoint3 &c);
  INLINE static bool verify_points(const LPoint3 &a, const LPoint3 &b,
                                   const LPoint3 &c, const LPoint3 &d);
  static bool verify_points(const LPoint3 *begin, const LPoint3 *end);

  bool is_valid() const;
  bool is_concave() const;

PUBLISHED:
  MAKE_SEQ_PROPERTY(points, get_num_points, get_point);
  MAKE_PROPERTY(valid, is_valid);
  MAKE_PROPERTY(concave, is_concave);

public:
  virtual void xform(const LMatrix4 &mat);

  virtual PT(PandaNode) get_viz(const CullTraverser *trav,
                                const CullTraverserData &data,
                                bool bounds_only) const;

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  INLINE static void flush_level();

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_parabola(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_capsule(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  INLINE static bool is_right(const LVector2 &v1, const LVector2 &v2);
  INLINE static PN_stdfloat dist_to_line(const LPoint2 &p,
                                   const LPoint2 &f, const LVector2 &v);
  static PN_stdfloat dist_to_line_segment(const LPoint2 &p,
                                    const LPoint2 &f, const LPoint2 &t,
                                    const LVector2 &v);

private:
  class PointDef {
  public:
    INLINE PointDef(const LPoint2 &p, const LVector2 &v);
    INLINE PointDef(PN_stdfloat x, PN_stdfloat y);
    INLINE PointDef(const PointDef &copy);
    INLINE void operator = (const PointDef &copy);

    LPoint2 _p;  // the point in 2-d space
    LVector2 _v; // the normalized vector to the next point
  };
  typedef pvector<PointDef> Points;

  static void compute_vectors(Points &points);
  void draw_polygon(GeomNode *viz_geom_node, GeomNode *bounds_viz_geom_node,
                    const Points &points) const;

  bool point_is_inside(const LPoint2 &p, const Points &points) const;
  PN_stdfloat dist_to_polygon(const LPoint2 &p, const Points &points) const;
  void project(const LVector3 &axis, PN_stdfloat &center, PN_stdfloat &extent) const;

  void setup_points(const LPoint3 *begin, const LPoint3 *end);
  INLINE LPoint2 to_2d(const LVecBase3 &point3d) const;
  INLINE void calc_to_3d_mat(LMatrix4 &to_3d_mat) const;
  INLINE void rederive_to_3d_mat(LMatrix4 &to_3d_mat) const;
  INLINE static LPoint3 to_3d(const LVecBase2 &point2d, const LMatrix4 &to_3d_mat);

  bool clip_polygon(Points &new_points, const Points &source_points,
                    const LPlane &plane) const;
  bool apply_clip_plane(Points &new_points, const ClipPlaneAttrib *cpa,
                        const TransformState *net_transform) const;

private:
  Points _points;
  LMatrix4 _to_2d_mat;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

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
