// Filename: cTask.h
// Created by:  Shochet (03Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#ifndef CTASK_H
#define CTASK_H

#include "directbase.h"
#include "typedReferenceCount.h"
#include "config_task.h"


class EXPCL_DIRECT CTask : public TypedReferenceCount {
PUBLISHED:
  CTask();
  ~CTask();

  INLINE void set_wake_time(float wake_time);
  INLINE float get_wake_time() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CTask",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  float _wake_time;

  static TypeHandle _type_handle;
};

#include "cTask.I"

#endif
