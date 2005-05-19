// Filename: collisionInvSphere.h
// Created by:  drose (05Jan05)
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
class EXPCL_PANDA CollisionInvSphere : public CollisionSphere {
PUBLISHED:
  INLINE CollisionInvSphere(const LPoint3f &center, float radius);
  INLINE CollisionInvSphere(float cx, float cy, float cz, float radius);

protected:
  INLINE CollisionInvSphere();

public:
  INLINE CollisionInvSphere(const CollisionInvSphere &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void output(ostream &out) const;

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


