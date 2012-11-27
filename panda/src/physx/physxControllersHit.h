// Filename: physxControllersHit.h
// Created by:  enn0x (28Nov12)
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

#ifndef PHYSXCONTROLLERSHIT
#define PHYSXCONTROLLERSHIT

#include "pandabase.h"
#include "callbackData.h"
#include "callbackObject.h"

#include "physx_includes.h"

class PhysxController;

////////////////////////////////////////////////////////////////////
//       Class : PhysxControllersHit
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxControllersHit : public CallbackData {

PUBLISHED:
  INLINE PhysxControllersHit(const NxControllersHit &hit);

  INLINE PhysxController *get_controller() const;
  INLINE PhysxController *get_other() const;

private:
  const NxControllersHit &_hit;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "PhysxControllersHit", 
                  CallbackData::get_class_type());
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

#include "physxControllersHit.I"

#endif // PHYSXCONTROLLERSHIT
