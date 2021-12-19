/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionBox.h
 * @author amith tudur
 * @date 2009-07-31
 */

#ifndef COLLISIONBOX_H
#define COLLISIONBOX_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"
#include "plane.h"
#include "look_at.h"
#include "clipPlaneAttrib.h"

/**
 * A cuboid collision volume or object.
 */
class EXPCL_PANDA_COLLIDE CollisionBox : public CollisionSolid {
PUBLISHED:
  INLINE explicit CollisionBox(const LPoint3 &center,
                               PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE explicit CollisionBox(const LPoint3 &min, const LPoint3 &max);

  virtual LPoint3 get_collision_origin() const;

protected:
  INLINE CollisionBox();

public:
  INLINE CollisionBox(const CollisionBox &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
    test_intersection(const CollisionEntry &entry) const;
  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;

  INLINE static void flush_level();
  void setup_box();

PUBLISHED:
  INLINE int get_num_points() const;
  INLINE LPoint3 get_point_aabb(int n) const;
  INLINE LPoint3 get_point(int n) const;
  INLINE int get_num_planes() const;
  INLINE LPlane set_plane(int n) const;
  INLINE LPlane get_plane(int n) const;
  INLINE void set_center(const LPoint3 &center);
  INLINE void set_center(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_center() const;
  INLINE const LPoint3 &get_min() const;
  INLINE const LPoint3 &get_max() const;
  INLINE LVector3 get_dimensions() const;

PUBLISHED:
  MAKE_PROPERTY(center, get_center);
  MAKE_PROPERTY(min, get_min);
  MAKE_PROPERTY(max, get_max);
  MAKE_PROPERTY(dimensions, get_dimensions);

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

protected:
  bool intersects_line(double &t1, double &t2,
                       const LPoint3 &from, const LVector3 &delta,
                       PN_stdfloat inflate_size=0) const;

private:
  LPoint3 _center;
  LPoint3 _min;
  LPoint3 _max;
  PN_stdfloat _x, _y, _z, _radius;
  LPoint3 _vertex[8]; // Each of the Eight Vertices of the Box
  LPlane _planes[6]; //Points to each of the six sides of the Box

  static const int plane_def[6][4];

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

private:
  INLINE static bool is_right(const LVector2 &v1, const LVector2 &v2);
  INLINE static PN_stdfloat dist_to_line(const LPoint2 &p,
                                   const LPoint2 &f, const LVector2 &v);
  static PN_stdfloat dist_to_line_segment(const LPoint2 &p,
                                    const LPoint2 &f, const LPoint2 &t,
                                    const LVector2 &v);

public:
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

  void setup_points(const LPoint3 *begin, const LPoint3 *end, int plane);
  INLINE LPoint2 to_2d(const LVecBase3 &point3d, int plane) const;
  INLINE void calc_to_3d_mat(LMatrix4 &to_3d_mat, int plane) const;
  INLINE void rederive_to_3d_mat(LMatrix4 &to_3d_mat, int plane) const;
  INLINE static LPoint3 to_3d(const LVecBase2 &point2d, const LMatrix4 &to_3d_mat);
  bool clip_polygon(Points &new_points, const Points &source_points,
                    const LPlane &plane,int plane_no) const;
  bool apply_clip_plane(Points &new_points, const ClipPlaneAttrib *cpa,
                        const TransformState *net_transform, int plane_no) const;

private:
  Points _points[6]; // one set of points for each of the six planes that make up the box
  LMatrix4 _to_2d_mat[6];

public:
  INLINE Points get_plane_points( int n );

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_CollisionBox(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionBox",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionBox.I"

#endif /* COLLISIONBOX_H */
