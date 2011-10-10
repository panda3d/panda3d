// Filename: collisionInvSphere.h
// Created by:  drose (05Jan05)
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

#ifndef COLLISIONINVSPHERE_H
#define COLLISIONINVSPHERE_H

#include "pandabase.h"

#include "collisionSphere.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionInvSphere
// Description : An inverted sphere: this is a sphere whose collision
//               surface is the inside surface of the sphere.
//               Everything outside the sphere is solid matter;
//               everything inside is empty space.  Useful for
//               constraining objects to remain within a spherical
//               perimeter.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionInvSphere : public CollisionSphere {
PUBLISHED:
  INLINE CollisionInvSphere(const LPoint3 &center, PN_stdfloat radius);
  INLINE CollisionInvSphere(PN_stdfloat cx, PN_stdfloat cy, PN_stdfloat cz, PN_stdfloat radius);

protected:
  INLINE CollisionInvSphere();

public:
  INLINE CollisionInvSphere(const CollisionInvSphere &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;

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

  virtual void fill_viz_geom();

private:
  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_CollisionInvSphere(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSphere::init_type();
    register_type(_type_handle, "CollisionInvSphere",
                  CollisionSphere::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionInvSphere.I"

#endif


