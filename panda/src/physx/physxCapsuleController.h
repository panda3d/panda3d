// Filename: physxCapsuleController.h
// Created by:  enn0x (24Sep09)
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

#ifndef PHYSXCAPSULECONTROLLER_H
#define PHYSXCAPSULECONTROLLER_H

#include "pandabase.h"
#include "luse.h"

#include "physxController.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleController
// Description : A capsule character controller.
//
//               The capsule is defined as a position, a vertical
//               height, and a radius. The height is the same height
//               as for PhysxCapsuleShape objects, i.e. the distance
//               between the two sphere centers at the end of the
//               capsule. In other words:
//
//               p = pos (returned by controller)
//               h = height
//               r = radius
//
//               p = center of capsule
//               top sphere center = p.y + h*0.5
//               bottom sphere center = p.y - h*0.5
//               top capsule point = p.y + h*0.5 + r
//               bottom capsule point = p.y - h*0.5 - r
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleController : public PhysxController {

PUBLISHED:
  INLINE PhysxCapsuleController();
  INLINE ~PhysxCapsuleController();

  void set_radius(float radius);
  void set_height(float height);
  float get_radius() const;
  float get_height() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxController *ptr() const { return (NxController *)_ptr; };

  void link(NxController *controllerPtr);
  void unlink();

private:
  NxCapsuleController *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxController::init_type();
    register_type(_type_handle, "PhysxCapsuleController", 
                  PhysxController::get_class_type());
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

#include "physxCapsuleController.I"

#endif // PHYSXCAPSULECONTROLLER_H
