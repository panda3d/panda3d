// Filename: collisionSegment.h
// Created by:  drose (30Jan01)
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

#ifndef COLLISIONSEGMENT_H
#define COLLISIONSEGMENT_H

#include "pandabase.h"

#include "collisionSolid.h"

class LensNode;

////////////////////////////////////////////////////////////////////
//       Class : CollisionSegment
// Description : A finite line segment, with two specific endpoints
//               but no thickness.  It's similar to a CollisionRay,
//               except it does not continue to infinity.
//
//               It does have an ordering, from point A to point B.
//               If more than a single point of the segment is
//               intersecting a solid, the reported intersection point
//               is generally the closest on the segment to point A.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionSegment : public CollisionSolid {
PUBLISHED:
  INLINE CollisionSegment();
  INLINE CollisionSegment(const LPoint3f &a, const LPoint3f &db);
  INLINE CollisionSegment(float ax, float ay, float az,
                          float bx, float by, float bz);

  virtual LPoint3f get_collision_origin() const;

public:
  INLINE CollisionSegment(const CollisionSegment &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_point_a(const LPoint3f &a);
  INLINE void set_point_a(float x, float y, float z);
  INLINE const LPoint3f &get_point_a() const;

  INLINE void set_point_b(const LPoint3f &b);
  INLINE void set_point_b(float x, float y, float z);
  INLINE const LPoint3f &get_point_b() const;

  bool set_from_lens(LensNode *camera, const LPoint2f &point);
  INLINE bool set_from_lens(LensNode *camera, float px, float py);

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
  virtual void fill_viz_geom();

private:
  LPoint3f _a, _b;

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
    register_type(_type_handle, "CollisionSegment",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionSegment.I"

#endif


