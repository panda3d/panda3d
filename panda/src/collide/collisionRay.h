// Filename: collisionRay.h
// Created by:  drose (22Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONRAY_H
#define COLLISIONRAY_H

#include "pandabase.h"

#include "collisionSolid.h"

class LensNode;

///////////////////////////////////////////////////////////////////
//       Class : CollisionRay
// Description : An infinite ray, with a specific origin and
//               direction.  It begins at its origin and continues in
//               one direction to infinity, and it has no radius.
//               Useful for picking from a window, or for gravity
//               effects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionRay : public CollisionSolid {
PUBLISHED:
  INLINE CollisionRay();
  INLINE CollisionRay(const LPoint3f &origin, const LVector3f &direction);
  INLINE CollisionRay(float ox, float oy, float oz,
                      float dx, float dy, float dz);

public:
  INLINE CollisionRay(const CollisionRay &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4f &mat);
  virtual LPoint3f get_collision_origin() const;

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_origin(const LPoint3f &origin);
  INLINE void set_origin(float x, float y, float z);
  INLINE const LPoint3f &get_origin() const;

  INLINE void set_direction(const LVector3f &direction);
  INLINE void set_direction(float x, float y, float z);
  INLINE const LVector3f &get_direction() const;

  bool set_from_lens(LensNode *camera, const LPoint2f &point);
  INLINE bool set_from_lens(LensNode *camera, float px, float py);

protected:
  virtual BoundingVolume *recompute_bound();

protected:
  virtual void fill_viz_geom();

private:
  LPoint3f _origin;
  LVector3f _direction;

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
    register_type(_type_handle, "CollisionRay",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionRay.I"

#endif


