/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSphere.h
 * @author drose
 * @date 2000-04-24
 */

#ifndef COLLISIONSPHERE_H
#define COLLISIONSPHERE_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"
#include "collisionBox.h"

/**
 * A spherical collision volume or object.
 */
class EXPCL_PANDA_COLLIDE CollisionSphere : public CollisionSolid {
PUBLISHED:
  INLINE explicit CollisionSphere(const LPoint3 &center, PN_stdfloat radius);
  INLINE explicit CollisionSphere(PN_stdfloat cx, PN_stdfloat cy, PN_stdfloat cz, PN_stdfloat radius);

  virtual LPoint3 get_collision_origin() const;

protected:
  INLINE CollisionSphere();

public:
  INLINE CollisionSphere(const CollisionSphere &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;

  INLINE static void flush_level();

PUBLISHED:
  INLINE void set_center(const LPoint3 &center);
  INLINE void set_center(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_center() const;

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

PUBLISHED:
  MAKE_PROPERTY(center, get_center, set_center);
  MAKE_PROPERTY(radius, get_radius, set_radius);

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
  test_intersection_from_capsule(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_parabola(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;

  virtual void fill_viz_geom();

protected:
  bool intersects_line(double &t1, double &t2,
                       const LPoint3 &from, const LVector3 &delta,
                       PN_stdfloat inflate_radius) const;
  bool intersects_parabola(double &t, const LParabola &parabola,
                           double t1, double t2,
                           const LPoint3 &p1, const LPoint3 &p2) const;
  LVertex compute_point(PN_stdfloat latitude, PN_stdfloat longitude) const;

private:
  LPoint3 _center;
  PN_stdfloat _radius;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_CollisionSphere(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionSphere",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionSphere.I"

#endif
