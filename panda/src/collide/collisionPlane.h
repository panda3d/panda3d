// Filename: collisionPlane.h
// Created by:  drose (25Apr00)
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

#ifndef COLLISIONPLANE_H
#define COLLISIONPLANE_H

#include "pandabase.h"

#include "collisionSolid.h"

#include "luse.h"
#include "plane.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionPlane
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionPlane : public CollisionSolid {
protected:
  INLINE CollisionPlane();

PUBLISHED:
  INLINE CollisionPlane(const LPlane &plane);
  INLINE CollisionPlane(const CollisionPlane &copy);

  virtual LPoint3 get_collision_origin() const;

public:
  virtual CollisionSolid *make_copy();

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(ostream &out) const;

  INLINE static void flush_level();

PUBLISHED:
  INLINE LVector3 get_normal() const;
  INLINE PN_stdfloat dist_to_plane(const LPoint3 &point) const;

  INLINE void set_plane(const LPlane &plane);
  INLINE const LPlane &get_plane() const;

  INLINE void flip();

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
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

private:
  LPlane _plane;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_CollisionPlane(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionPlane",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionPlane.I"

#endif


