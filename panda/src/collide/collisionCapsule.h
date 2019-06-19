/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionCapsule.h
 * @author drose
 * @date 2003-09-25
 */

#ifndef COLLISIONCAPSULE_H
#define COLLISIONCAPSULE_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"

/**
 * This implements a solid consisting of a cylinder with hemispherical endcaps,
 * also known as a capsule or a spherocylinder.
 *
 * This shape was previously erroneously called CollisionTube.
 */
class EXPCL_PANDA_COLLIDE CollisionCapsule : public CollisionSolid {
PUBLISHED:
  INLINE explicit CollisionCapsule(const LPoint3 &a, const LPoint3 &db,
                                   PN_stdfloat radius);
  INLINE explicit CollisionCapsule(PN_stdfloat ax, PN_stdfloat ay, PN_stdfloat az,
                                   PN_stdfloat bx, PN_stdfloat by, PN_stdfloat bz,
                                   PN_stdfloat radius);

  virtual LPoint3 get_collision_origin() const;

private:
  INLINE CollisionCapsule();

public:
  INLINE CollisionCapsule(const CollisionCapsule &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;

  INLINE static void flush_level();

PUBLISHED:
  INLINE void set_point_a(const LPoint3 &a);
  INLINE void set_point_a(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_point_a() const;

  INLINE void set_point_b(const LPoint3 &b);
  INLINE void set_point_b(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_point_b() const;

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

PUBLISHED:
  MAKE_PROPERTY(point_a, get_point_a, set_point_a);
  MAKE_PROPERTY(point_b, get_point_b, set_point_b);
  MAKE_PROPERTY(radius, get_radius, set_radius);

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_capsule(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_parabola(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

private:
  void recalc_internals();

  LVertex calc_sphere1_vertex(int ri, int si, int num_rings, int num_slices);
  LVertex calc_sphere2_vertex(int ri, int si, int num_rings, int num_slices,
                              PN_stdfloat length);

  static void calc_closest_segment_points(double &t1, double &t2,
                                          const LPoint3 &from1, const LVector3 &delta1,
                                          const LPoint3 &from2, const LVector3 &delta2);
  bool intersects_line(double &t1, double &t2,
                       const LPoint3 &from, const LVector3 &delta,
                       PN_stdfloat inflate_radius) const;
  static bool sphere_intersects_line(double &t1, double &t2, PN_stdfloat center_y,
                                     const LPoint3 &from, const LVector3 &delta,
                                     PN_stdfloat radius);
  bool intersects_parabola(double &t, const LParabola &parabola,
                           double t1, double t2,
                           const LPoint3 &p1, const LPoint3 &p2) const;
  void calculate_surface_point_and_normal(const LPoint3 &surface_point,
                                          double extra_radius,
                                          LPoint3 &result_point,
                                          LVector3 &result_normal) const;
  void set_intersection_point(CollisionEntry *new_entry,
                              const LPoint3 &into_intersection_point,
                              double extra_radius) const;

private:
  LPoint3 _a, _b;
  PN_stdfloat _radius;

  // These are derived from the above.
  LMatrix4 _mat;
  LMatrix4 _inv_mat;
  PN_stdfloat _length;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionCapsule",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionBox;
  friend class CollisionPolygon;
};

#include "collisionCapsule.I"

#endif
