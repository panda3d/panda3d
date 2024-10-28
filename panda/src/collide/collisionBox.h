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
  bool intersects_capsule(double &t,
                          const LPoint3 &from, const LVector3 &delta,
                          PN_stdfloat radius_sq) const;

private:
  LPoint3 _center;
  LPoint3 _min;
  LPoint3 _max;
  LPlane _planes[6]; //Points to each of the six sides of the Box

  static const int plane_def[6][4];

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
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
