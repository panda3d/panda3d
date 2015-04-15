// Filename: bulletBoxShape.h
// Created by:  enn0x (24Jan10)
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

#ifndef __BULLET_BOX_SHAPE_H__
#define __BULLET_BOX_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

#include "luse.h"

#include "collisionBox.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletBoxShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletBoxShape : public BulletShape {
private:
  // Only used by make_from_bam
  INLINE BulletBoxShape() : _shape(NULL) {};

PUBLISHED:
  BulletBoxShape(const LVecBase3 &halfExtents);
  INLINE BulletBoxShape(const BulletBoxShape &copy);
  INLINE void operator = (const BulletBoxShape &copy);
  INLINE ~BulletBoxShape();

  LVecBase3 get_half_extents_without_margin() const;
  LVecBase3 get_half_extents_with_margin() const;

  static BulletBoxShape *make_from_solid(const CollisionBox *solid);

public:
  virtual btCollisionShape *ptr() const;

private:
  btBoxShape *_shape;

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
    register_type(_type_handle, "BulletBoxShape", 
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

#include "bulletBoxShape.I"

#endif // __BULLET_BOX_SHAPE_H__
