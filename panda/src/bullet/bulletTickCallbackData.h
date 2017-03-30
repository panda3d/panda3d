/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTickCallbackData.h
 * @author enn0x
 * @date 2012-11-26
 */

#ifndef __BULLET_TICK_CALLBACK_DATA_H__
#define __BULLET_TICK_CALLBACK_DATA_H__

#include "pandabase.h"
#include "callbackData.h"
#include "callbackObject.h"

#include "bullet_includes.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletTickCallbackData : public CallbackData {

PUBLISHED:
  INLINE BulletTickCallbackData(btScalar timestep);

  INLINE PN_stdfloat get_timestep() const;

  MAKE_PROPERTY(timestep, get_timestep);

private:
  btScalar _timestep;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "BulletTickCallbackData",
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

#include "bulletTickCallbackData.I"

#endif // __BULLET_TICK_CALLBACK_DATA_H__
