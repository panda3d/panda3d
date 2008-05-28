// Filename: collisionSphere.h
// Created by:  drose (24Apr00)
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

#ifndef COLLISIONSPHERE_H
#define COLLISIONSPHERE_H

#include "pandabase.h"
#include "collisionSolid.h"
#include "parabola.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionSphere
// Description : A spherical collision volume or object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionSphere : public CollisionSolid {
PUBLISHED:
  INLINE CollisionSphere(const LPoint3f &center, float radius);
  INLINE CollisionSphere(float cx, float cy, float cz, float radius);

  virtual LPoint3f get_collision_origin() const;

protected:
  INLINE CollisionSphere();

public:
  INLINE CollisionSphere(const CollisionSphere &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4f &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;

  INLINE static void flush_level();

PUBLISHED:
  INLINE void set_center(const LPoint3f &center);
  INLINE void set_center(float x, float y, float z);
  INLINE const LPoint3f &get_center() const;

  INLINE void set_radius(float radius);
  INLINE float get_radius() const;

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

  virtual PT(CollisionEntry)
  test_intersection_from_ds_solid(const CollisionEntry &entry) const;
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

  virtual void fill_viz_geom();

protected:
  bool intersects_line(double &t1, double &t2,
                       const LPoint3f &from, const LVector3f &delta,
                       float inflate_radius) const;
  bool intersects_parabola(double &t, const Parabolaf &parabola,
                           double t1, double t2,
                           const LPoint3f &p1, const LPoint3f &p2) const;
  Vertexf compute_point(float latitude, float longitude) const;

private:
  LPoint3f _center;
  float _radius;

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


