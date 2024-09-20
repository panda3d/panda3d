/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHeightfield.h
 * @author hecris
 * @date 2019-07-01
 */

#ifndef COLLISIONHEIGHTFIELD_H
#define COLLISIONHEIGHTFIELD_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "pnmImage.h"

/*
 * CollisionHeightfield efficiently deals with collisions on uneven
 * terrain given a heightfield image. A quad tree is implemented to
 * significantly reduce the amount of triangles tested. Each quad
 * tree node represents a sub-rectangle of the heightfield image
 * and thus a box in 3D space.
 * */
class EXPCL_PANDA_COLLIDE CollisionHeightfield : public CollisionSolid {
PUBLISHED:
  CollisionHeightfield(PNMImage heightfield,
                       PN_stdfloat max_height, int num_subdivisions);
  virtual LPoint3 get_collision_origin() const;

  INLINE PNMImage get_heightfield();
  INLINE void set_heightfield(PNMImage heightfield);

  INLINE PN_stdfloat get_max_height();
  INLINE void set_max_height(PN_stdfloat max_height);

  INLINE int get_num_subdivisions();
  void set_num_subdivisions(int num_subdivisions);

  INLINE PN_stdfloat get_height(int x, int y) const;

protected:
  struct Rect {
    LVector2 min;
    LVector2 max;
  };

  struct QuadTreeNode {
    int index;
    Rect area;
    PN_stdfloat height_min;
    PN_stdfloat height_max;
  };

  struct QuadTreeIntersection {
    int node_index;
    double tmin;
    double tmax;
    bool operator < (const QuadTreeIntersection& intersection) const {
      return tmin < intersection.tmin;
    }
  };

  struct IntersectionParams {
    // From Line
    double t1;
    double t2;
    LPoint3 from_origin;
    LVector3 from_direction;
    // From Sphere
    LPoint3 center;
    double radius;
    // From Box
    LPoint3 box_min;
    LPoint3 box_max;
  };

  struct Triangle {
    LPoint3 p1;
    LPoint3 p2;
    LPoint3 p3;
  };

private:
  PNMImage _heightfield;
  PN_stdfloat _max_height;
  int _num_subdivisions;
  QuadTreeNode *_nodes;
  int _nodes_count;
  int _leaf_first_index;
  void fill_quadtree_areas();
  void fill_quadtree_heights();
  std::vector<Triangle> get_triangles(int x, int y) const;

  // A pointer to a function that tests for intersection between a box and a
  // solid defined by the given IntersectionParams.
  typedef bool (*BoxIntersection)(const LPoint3 &box_min, const LPoint3 &box_max,
                                  IntersectionParams &params);

  std::vector<QuadTreeIntersection> find_intersections(BoxIntersection intersects_box,
                                                  IntersectionParams params) const;

protected:
  static LPoint3 closest_point_on_triangle(const LPoint3 &p, const Triangle &triangle);

  static bool line_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                                  IntersectionParams &params);

  static bool line_intersects_triangle(double &t, const LPoint3 &from,
                                const LPoint3 &delta,
                                const Triangle &triangle);

  static bool sphere_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                                    IntersectionParams &params);

  static bool box_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                                 IntersectionParams &params);

  static bool box_intersects_triangle(const LPoint3 &box_min, const LPoint3 &box_max,
                               const Triangle &triangle);

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

public:
  INLINE CollisionHeightfield(const CollisionHeightfield &copy);
  virtual CollisionSolid *make_copy();

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  INLINE static void flush_level();
  INLINE ~CollisionHeightfield();

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

private:
  INLINE CollisionHeightfield();
  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static void register_with_read_factory();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionHeightfield",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHeightfield.I"

#endif
