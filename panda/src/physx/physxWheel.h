/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheel.h
 * @author enn0x
 * @date 2010-03-23
 */

#ifndef PHYSXWHEEL_H
#define PHYSXWHEEL_H

#include "pandabase.h"
#include "nodePath.h"

#include "physxObject.h"
#include "physx_includes.h"

class PhysxWheelDesc;
class PhysxWheelShape;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxWheel : public PhysxObject {

PUBLISHED:
  INLINE PhysxWheel();
  INLINE ~PhysxWheel();

  //PhysxActor *get_touched_actor() const;
  //PhysxWheelShape *get_wheel_shape() const;

  //void attach_node_path(const NodePath &np);
  //void detach_node_path();
  //NodePath get_node_path() const;

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

private:
  PT(PhysxWheelShape) _wheelShape;
  NodePath _np;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxWheel",
                  PhysxObject::get_class_type());
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

#include "physxWheel.I"

#endif // PHYSXWHEEL_H
