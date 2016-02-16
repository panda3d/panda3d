/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskBase.h
 * @author drose
 * @date 2010-02-09
 */

#ifndef ASYNCTASKBASE_H
#define ASYNCTASKBASE_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "namable.h"

class Thread;

/**
 * The abstract base class for AsyncTask.  This is defined here only so we can
 * store a pointer to the current task on the Thread.
 */
class EXPCL_PANDA_PIPELINE AsyncTaskBase : public TypedReferenceCount, public Namable {
protected:
  AsyncTaskBase();
public:
  ALLOC_DELETED_CHAIN(AsyncTaskBase);

PUBLISHED:
  virtual ~AsyncTaskBase();

protected:
  void record_task(Thread *current_thread);
  void clear_task(Thread *current_thread);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AsyncTaskBase",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "asyncTaskBase.I"

#endif
