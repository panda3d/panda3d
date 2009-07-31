// Filename: collisionBox.h
// Created by:  amith tudur( 31Jul09 )
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

#ifndef COLLISIONBOX_H
#define COLLISIONBOX_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"
#include "plane.h"
#include "look_at.h"
#include "clipPlaneAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionBox
// Description : A cuboid collision volume or object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionBox : public CollisionSolid {
PUBLISHED:
  INLINE CollisionBox(const LPoint3f &center, 
					   float x, float y, float z);
  INLINE CollisionBox(float cx, float cy,	float cz,
        float x,  float y,	float z);
  INLINE CollisionBox(const LPoint3f &min, const LPoint3f &max);

  virtual LPoint3f get_collision_origin() const;

protected:
  INLINE CollisionBox();

public:
  INLINE CollisionBox(const CollisionBox &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;
  virtual void xform(const LMatrix4f &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;
  
  virtual LPoint3f get_approx_center() const;
  virtual LPoint3f get_min() const;
  virtual LPoint3f get_max() const;

  INLINE static void flush_level();
  void setup_box();

PUBLISHED:
  INLINE_MATHUTIL int get_num_points() const;
  INLINE_MATHUTIL LPoint3f get_point_aabb(int n) const;
  INLINE_MATHUTIL LPoint3f get_point(int n) const;
  INLINE_MATHUTIL int get_num_planes() const;
  INLINE_MATHUTIL Planef set_plane(int n) const;
  INLINE_MATHUTIL Planef get_plane(int n) const;
  INLINE void set_center(const LPoint3f &center);
  INLINE void set_center(float x, float y, float z);
  INLINE const LPoint3f &get_center() const;
  INLINE float get_radius() const;

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;
  
  virtual void fill_viz_geom();

protected:
  Vertexf compute_point(float latitude, float longitude) const;

private:
  LPoint3f _center;
  LPoint3f _min;
  LPoint3f _max;
  float _x, _y, _z, _radius;
  LPoint3f _vertex[8]; // Each of the Eight Vertices of the Box
  Planef _planes[6]; //Points to each of the six sides of the Box
  
  static const int plane_def[6][4];
  
  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

private:
  INLINE static bool is_right(const LVector2f &v1, const LVector2f &v2);
  INLINE static float dist_to_line(const LPoint2f &p,
                                   const LPoint2f &f, const LVector2f &v);
  static float dist_to_line_segment(const LPoint2f &p,
                                    const LPoint2f &f, const LPoint2f &t,
                                    const LVector2f &v);
  
public:
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

  void setup_points(const LPoint3f *begin, const LPoint3f *end, int plane);
  INLINE LPoint2f to_2d(const LVecBase3f &point3d, int plane) const;
  INLINE void calc_to_3d_mat(LMatrix4f &to_3d_mat, int plane) const;
  INLINE void rederive_to_3d_mat(LMatrix4f &to_3d_mat, int plane) const;
  INLINE static LPoint3f to_3d(const LVecBase2f &point2d, const LMatrix4f &to_3d_mat);
  LPoint3f legacy_to_3d(const LVecBase2f &point2d, int axis) const;
  bool clip_polygon(Points &new_points, const Points &source_points,
                    const Planef &plane,int plane_no) const;
  bool apply_clip_plane(Points &new_points, const ClipPlaneAttrib *cpa,
                        const TransformState *net_transform, int plane_no) const;

private:
  Points _points[6]; // one set of points for each of the six planes that make up the box
  LMatrix4f _to_2d_mat[6]; 

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
