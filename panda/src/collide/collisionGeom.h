/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionGeom.h
 * @author drose
 * @date 2006-03-01
 */

#ifndef COLLISIONGEOM_H
#define COLLISIONGEOM_H

#include "pandabase.h"

#include "collisionPolygon.h"

/**
 * A special CollisionPolygon created just for the purpose of detecting
 * collision against geometry.  This kind of object does not have any
 * persistance in the scene graph; it is created on-the-fly.
 *
 * You should not attempt to create one of these directly; it is created only
 * by the CollisionTraverser, as needed.
 */
class EXPCL_PANDA_COLLIDE CollisionGeom : public CollisionPolygon {
private:
  INLINE CollisionGeom(const LVecBase3 &a, const LVecBase3 &b,
                       const LVecBase3 &c);
  INLINE CollisionGeom(const CollisionGeom &copy);

public:
  virtual CollisionSolid *make_copy();

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;

private:
  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionPolygon::init_type();
    register_type(_type_handle, "CollisionGeom",
                  CollisionPolygon::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
};

#include "collisionGeom.I"

#endif
