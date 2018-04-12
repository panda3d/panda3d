/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCylinderShape.h
 * @author enn0x
 * @date 2010-02-17
 */

#ifndef __BULLET_CYLINDER_SHAPE_H__
#define __BULLET_CYLINDER_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletCylinderShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletCylinderShape();

PUBLISHED:
  explicit BulletCylinderShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up=Z_up);
  explicit BulletCylinderShape(const LVector3 &half_extents, BulletUpAxis up=Z_up);
  BulletCylinderShape(const BulletCylinderShape &copy);
  INLINE ~BulletCylinderShape();

  PN_stdfloat get_radius() const;
  LVecBase3 get_half_extents_without_margin() const;
  LVecBase3 get_half_extents_with_margin() const;

  MAKE_PROPERTY(radius, get_radius);
  MAKE_PROPERTY(half_extents_without_margin, get_half_extents_without_margin);
  MAKE_PROPERTY(half_extents_with_margin, get_half_extents_with_margin);

public:
  virtual btCollisionShape *ptr() const;

private:
  LVector3 _half_extents;
  btCylinderShape *_shape;
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
    register_type(_type_handle, "BulletCylinderShape",
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

#include "bulletCylinderShape.I"

#endif // __BULLET_CYLINDER_SHAPE_H__
