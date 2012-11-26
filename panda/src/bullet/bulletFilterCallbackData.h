// Filename: bulletFilterCallbackData.h
// Created by:  enn0x (26Nov12)
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

#ifndef __BULLET_FILTER_CALLBACK_DATA_H__
#define __BULLET_FILTER_CALLBACK_DATA_H__

#include "pandabase.h"
#include "callbackData.h"
#include "callbackObject.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletFilterCallbackData
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletFilterCallbackData : public CallbackData {

PUBLISHED:
  INLINE BulletFilterCallbackData(PandaNode *node0, 
                                  PandaNode *node1);

  INLINE PandaNode *get_node_0() const;
  INLINE PandaNode *get_node_1() const;

  INLINE void set_collide(bool collide);
  INLINE bool get_collide() const;

private:
  PandaNode *_node0;
  PandaNode *_node1;
  bool _collide;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "BulletFilterCallbackData", 
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

#include "bulletFilterCallbackData.I"

#endif // __BULLET_FILTER_CALLBACK_DATA_H__
