/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCapsuleShape.h
 * @author enn0x
 * @date 2010-01-27
 */

#ifndef __BULLET_CAPSULE_SHAPE_H__
#define __BULLET_CAPSULE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "collisionCapsule.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletCapsuleShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletCapsuleShape();

PUBLISHED:
  explicit BulletCapsuleShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up=Z_up);
  BulletCapsuleShape(const BulletCapsuleShape &copy);
  INLINE ~BulletCapsuleShape();

  static BulletCapsuleShape *make_from_solid(const CollisionCapsule *solid);

  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_half_height() const;

public:
  INLINE PN_stdfloat get_height() const;

  virtual btCollisionShape *ptr() const;

PUBLISHED:
  MAKE_PROPERTY(radius, get_radius);
  MAKE_PROPERTY(height, get_height);

private:
  btCapsuleShape *_shape;
  PN_stdfloat _radius;
  PN_stdfloat _height;
  BulletUpAxis _up;

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
    BulletShape::init_type();
    register_type(_type_handle, "BulletCapsuleShape",
                  BulletShape::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletCapsuleShape.I"

#endif // __BULLET_CAPSULE_SHAPE_H__
