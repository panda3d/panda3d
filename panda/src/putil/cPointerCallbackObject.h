// Filename: cPointerCallbackObject.h
// Created by:  drose (13Mar09)
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

#ifndef CPOINTERCALLBACKOBJECT_H
#define CPOINTERCALLBACKOBJECT_H

#include "pandabase.h"
#include "callbackObject.h"

////////////////////////////////////////////////////////////////////
//       Class : CPointerCallbackObject
// Description : This is a specialization on CallbackObject to allow
//               association with a C-style function pointer and a
//               void * parameter.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL CPointerCallbackObject : public CallbackObject {
public:
  typedef void CallbackFunction(CallbackData *cbdata, void *data);
  INLINE CPointerCallbackObject(CallbackFunction *func, void *data);
  ALLOC_DELETED_CHAIN(CPointerCallbackObject);

public:
  virtual void do_callback(CallbackData *cbdata);

private:
  CallbackFunction *_func;
  void *_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackObject::init_type();
    register_type(_type_handle, "CPointerCallbackObject",
                  CallbackObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cPointerCallbackObject.I"

#endif
