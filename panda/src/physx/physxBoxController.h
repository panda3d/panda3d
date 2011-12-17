// Filename: physxBoxController.h
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

#ifndef PHYSXBOXCONTROLLER_H
#define PHYSXBOXCONTROLLER_H

#include "pandabase.h"
#include "luse.h"

#include "physxController.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxController
// Description : Box character controller.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxController : public PhysxController {

PUBLISHED:
  INLINE PhysxBoxController();
  INLINE ~PhysxBoxController();

  void set_extents(const LVector3f &extents);
  LVector3f get_extents() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxController *ptr() const { return (NxController *)_ptr; };

  void link(NxController *controllerPtr);
  void unlink();

private:
  NxBoxController *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxController::init_type();
    register_type(_type_handle, "PhysxBoxController", 
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

#include "physxBoxController.I"

#endif // PHYSXBOXCONTROLLER_H
