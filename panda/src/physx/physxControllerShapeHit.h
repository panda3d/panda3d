// Filename: physxControllerShapeHit.h
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

#ifndef PHYSXCONTROLLERSHAPEHIT
#define PHYSXCONTROLLERSHAPEHIT

#include "pandabase.h"
#include "callbackData.h"
#include "callbackObject.h"

#include "physx_includes.h"
#include "physxManager.h"

class PhysxController;
class PhysxShape;

////////////////////////////////////////////////////////////////////
//       Class : PhysxControllerShapeHit
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxControllerShapeHit : public CallbackData {

PUBLISHED:
  INLINE PhysxControllerShapeHit(const NxControllerShapeHit &hit);

  INLINE PhysxController *get_controller() const;
  INLINE PhysxShape *get_shape() const;
  INLINE LPoint3 get_world_pos() const;
  INLINE LVector3 get_world_normal() const;
  INLINE LVector3 get_dir() const;
  INLINE PN_stdfloat get_length() const;

private:
  const NxControllerShapeHit &_hit;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "PhysxControllerShapeHit", 
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

#include "physxControllerShapeHit.I"

#endif // PHYSXCONTROLLERSHAPEHIT
