/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionRay.h
 * @author drose
 * @date 2000-06-22
 */

#ifndef COLLISIONRAY_H
#define COLLISIONRAY_H

#include "pandabase.h"

#include "collisionSolid.h"
#include "lensNode.h"

/**
 * An infinite ray, with a specific origin and direction.  It begins at its
 * origin and continues in one direction to infinity, and it has no radius.
 * Useful for picking from a window, or for gravity effects.
 */
class EXPCL_PANDA_COLLIDE CollisionRay : public CollisionSolid {
PUBLISHED:
  INLINE CollisionRay();
  INLINE explicit CollisionRay(const LPoint3 &origin, const LVector3 &direction);
  INLINE explicit CollisionRay(PN_stdfloat ox, PN_stdfloat oy, PN_stdfloat oz,
                               PN_stdfloat dx, PN_stdfloat dy, PN_stdfloat dz);

  virtual LPoint3 get_collision_origin() const;

public:
  INLINE CollisionRay(const CollisionRay &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE void set_origin(const LPoint3 &origin);
  INLINE void set_origin(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LPoint3 &get_origin() const;
  MAKE_PROPERTY(origin, get_origin, set_origin);

  INLINE void set_direction(const LVector3 &direction);
  INLINE void set_direction(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE const LVector3 &get_direction() const;
  MAKE_PROPERTY(direction, get_direction, set_direction);

  bool set_from_lens(LensNode *camera, const LPoint2 &point);
  INLINE bool set_from_lens(LensNode *camera, PN_stdfloat px, PN_stdfloat py);

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
  virtual void fill_viz_geom();

private:
  LPoint3 _origin;
  LVector3 _direction;

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
